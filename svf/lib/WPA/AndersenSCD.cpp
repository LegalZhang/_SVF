

#include "WPA/AndersenPWC.h"
#include "MemoryModel/PointsTo.h"
#include "Util/Options.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;

AndersenSCD* AndersenSCD::scdAndersen = nullptr;


/*!
 *
 */
void AndersenSCD::solveWorklist()
{
    // Initialize the nodeStack via a whole SCC detection
    // Nodes in nodeStack are in topological order by default.
    NodeStack& nodeStack = SCCDetect();

    for (NodeID nId : sccCandidates)
        pushIntoWorklist(nId);
    sccCandidates.clear();

    // propagate point-to sets
    while (!nodeStack.empty())
    {
        NodeID nodeId = nodeStack.top();
        nodeStack.pop();

        if (sccRepNode(nodeId) == nodeId)
        {
            collapsePWCNode(nodeId);

            if (isInWorklist(nodeId))
                // push the rep of node into worklist
                pushIntoWorklist(nodeId);

            double propStart = stat->getClk();
            // propagate pts through copy and gep edges
            ConstraintNode* node = consCG->getConstraintNode(nodeId);
            handleCopyGep(node);
            double propEnd = stat->getClk();
            timeOfProcessCopyGep += (propEnd - propStart) / TIMEINTERVAL;

            collapseFields();
        }
    }

    // New nodes will be inserted into workList during processing.
    while (!isWorklistEmpty())
    {
        NodeID nodeId = popFromWorklist();

        double insertStart = stat->getClk();
        // add copy edges via processing load or store edges
        ConstraintNode* node = consCG->getConstraintNode(nodeId);
        handleLoadStore(node);
        double insertEnd = stat->getClk();
        timeOfProcessLoadStore += (insertEnd - insertStart) / TIMEINTERVAL;
    }
}


/*!
 * SCC detection for SCD
 */
NodeStack& AndersenSCD::SCCDetect()
{
    numOfSCCDetection++;

    double sccStart = stat->getClk();
    getSCCDetector()->find(sccCandidates);
    double sccEnd = stat->getClk();
    timeOfSCCDetection +=  (sccEnd - sccStart)/TIMEINTERVAL;

    double mergeStart = stat->getClk();
    mergeSccCycle();
    double mergeEnd = stat->getClk();
    timeOfSCCMerges +=  (mergeEnd - mergeStart)/TIMEINTERVAL;

    if (!Options::DetectPWC())
    {
        sccStart = stat->getClk();
        PWCDetect();
        sccEnd = stat->getClk();
        timeOfSCCDetection += (sccEnd - sccStart) / TIMEINTERVAL;
    }

    return getSCCDetector()->topoNodeStack();
}


/*!
 *
 */
void AndersenSCD::PWCDetect()
{
    // replace scc candidates by their reps
    NodeSet tmpSccCandidates = sccCandidates;
    sccCandidates.clear();
    for (NodeID candidate : tmpSccCandidates)
        sccCandidates.insert(sccRepNode(candidate));
    tmpSccCandidates.clear();

    // set scc edge type as direct edge
    bool pwcFlag = Options::DetectPWC();
    setDetectPWC(true);

    getSCCDetector()->find(sccCandidates);

    // reset scc edge type
    setDetectPWC(pwcFlag);
}


/*!
 * Compute diff points-to set before propagation
 */
void AndersenSCD::handleCopyGep(ConstraintNode* node)
{
    NodeID nodeId = node->getId();

    if (!Options::DetectPWC() && getSCCDetector()->subNodes(nodeId).count() > 1)
        processPWC(node);
    else if(isInWorklist(nodeId))
        Andersen::handleCopyGep(node);
}


/*!
 *
 */
void AndersenSCD::processPWC(ConstraintNode* rep)
{
    NodeID repId = rep->getId();

    NodeSet pwcNodes;
    for (NodeID nId : getSCCDetector()->subNodes(repId))
        pwcNodes.insert(nId);

    WorkList tmpWorkList;
    for (NodeID subId : pwcNodes)
        if (isInWorklist(subId))
            tmpWorkList.push(subId);

    while (!tmpWorkList.empty())
    {
        NodeID nodeId = tmpWorkList.pop();
        computeDiffPts(nodeId);

        if (!getDiffPts(nodeId).empty())
        {
            ConstraintNode *node = consCG->getConstraintNode(nodeId);
            for (ConstraintEdge* edge : node->getCopyOutEdges())
            {
                bool changed = processCopy(nodeId, edge);
                if (changed && pwcNodes.find(edge->getDstID()) != pwcNodes.end())
                    tmpWorkList.push(edge->getDstID());
            }
            for (ConstraintEdge* edge : node->getGepOutEdges())
            {
                if (GepCGEdge *gepEdge = SVFUtil::dyn_cast<GepCGEdge>(edge))
                {
                    bool changed = processGep(nodeId, gepEdge);
                    if (changed && pwcNodes.find(edge->getDstID()) != pwcNodes.end())
                        tmpWorkList.push(edge->getDstID());
                }
            }
        }
    }
}


/*!
 * Source nodes of new added edges are pushed into sccCandidates.
 * Source nodes of new added edges whose pts differ from those of dst nodes are pushed into worklist.
 */
void AndersenSCD::handleLoadStore(ConstraintNode* node)
{
    double insertStart = stat->getClk();

    NodeID nodeId = node->getId();
    // handle load
    for (ConstraintNode::const_iterator it = node->outgoingLoadsBegin(),
            eit = node->outgoingLoadsEnd(); it != eit; ++it)
        for (PointsTo::iterator piter = getPts(nodeId).begin(), epiter =
                    getPts(nodeId).end(); piter != epiter; ++piter)
        {
            NodeID ptd = *piter;
            if (processLoad(ptd, *it))
            {
                reanalyze = true;
            }
        }

    // handle store
    for (ConstraintNode::const_iterator it = node->incomingStoresBegin(),
            eit = node->incomingStoresEnd(); it != eit; ++it)
        for (PointsTo::iterator piter = getPts(nodeId).begin(), epiter =
                    getPts(nodeId).end(); piter != epiter; ++piter)
        {
            NodeID ptd = *piter;
            if (processStore(ptd, *it))
            {
                reanalyze = true;
            }
        }

    double insertEnd = stat->getClk();
    timeOfProcessLoadStore += (insertEnd - insertStart) / TIMEINTERVAL;
}


/*!
 * Initialize worklist via processing addrs
 */
void AndersenSCD::processAddr(const AddrCGEdge *addr)
{
    numOfProcessedAddr++;

    NodeID dst = addr->getDstID();
    NodeID src = addr->getSrcID();
    addPts(dst,src);
    addSccCandidate(dst);
}


/*!
 * If one copy edge is successful added, the src node should be added into SCC detection
 */
bool AndersenSCD::addCopyEdge(NodeID src, NodeID dst)
{
    if (Andersen::addCopyEdge(src, dst))
    {
        addSccCandidate(src);
        return true;
    }
    return false;
}


/*!
 * Update call graph for the input indirect callsites
 */
bool AndersenSCD::updateCallGraph(const PointerAnalysis::CallSiteToFunPtrMap& callsites)
{
    double cgUpdateStart = stat->getClk();

    CallEdgeMap newEdges;
    onTheFlyCallGraphSolve(callsites,newEdges);
    NodePairSet cpySrcNodes;	/// nodes as a src of a generated new copy edge
    for(CallEdgeMap::iterator it = newEdges.begin(), eit = newEdges.end(); it!=eit; ++it )
    {
        for(FunctionSet::iterator cit = it->second.begin(), ecit = it->second.end(); cit!=ecit; ++cit)
        {
            connectCaller2CalleeParams(it->first,*cit,cpySrcNodes);
        }
    }

    double cgUpdateEnd = stat->getClk();
    timeOfUpdateCallGraph += (cgUpdateEnd - cgUpdateStart) / TIMEINTERVAL;

    return (!newEdges.empty());
}
