//
// Created by Jiahao Zhang on 18/11/2024.
//

#include "Graphs/FSConsG.h"
#include "WPA/Andersen.h"
#include "WPA/FlowSensitive.h"
#include "WPA/FlowSensitiveCG.h"

using namespace SVF;

/*!
 * Initialize analysis
 */
void FlowSensitiveCG::initialize()
{
    PointerAnalysis::initialize();

    /// Create Andersen statistic class
    stat = new AndersenStat(this);

    ander = AndersenWaveDiff::createAndersenWaveDiff(getPAG());
    svfg = memSSA.buildPTROnlySVFG(ander);

    /// Build Flow-Sensitive Constraint Graph
    fsconsCG = new FSConsG(svfg);
    setGraph(fsconsCG);
    if (Options::SVFG2CG())
        fsconsCG->dump("fsconsg_initial");
}

/*!
 * Finalize analysis
 */
void FlowSensitiveCG::finalize()
{
    if (Options::SVFG2CG())
        fsconsCG->dump("fsconsg_final");

    BVDataPTAImpl::finalize();
}

/*!
 * solve worklist
 */
void FlowSensitiveCG::solveWorklist()
{
    // Initialize the nodeStack via a whole SCC detection
    // Nodes in nodeStack are in topological order by default.
    NodeStack& nodeStack = SCCDetect();

    // Process nodeStack and put the changed nodes into workList.
    while (!nodeStack.empty())
    {
        NodeID nodeId = nodeStack.top();
        nodeStack.pop();
        collapsePWCNode(nodeId);
        // process nodes in nodeStack
        processNode(nodeId);
        collapseFields();
    }

    // New nodes will be inserted into workList during processing.
    while (!isWorklistEmpty())
    {
        NodeID nodeId = popFromWorklist();
        // process nodes in worklist
        postProcessNode(nodeId);
    }
}

/*!
 * Post process node
 */
void FlowSensitiveCG::postProcessNode(NodeID nodeId)
{
    double insertStart = stat->getClk();

    ConstraintNode* node = consCG->getConstraintNode(nodeId);

    // handle load
    for (ConstraintNode::const_iterator it = node->outgoingLoadsBegin(), eit = node->outgoingLoadsEnd();
            it != eit; ++it)
    {
        if (handleLoad(nodeId, *it))
            reanalyze = true;
    }
    // handle store
    for (ConstraintNode::const_iterator it = node->incomingStoresBegin(), eit =  node->incomingStoresEnd();
            it != eit; ++it)
    {
        if (handleStore(nodeId, *it))
            reanalyze = true;
    }

    double insertEnd = stat->getClk();
    timeOfProcessLoadStore += (insertEnd - insertStart) / TIMEINTERVAL;
}

/*
void FlowsensitiveCG::handleLoadStore(ConstraintNode* node)
{
    NodeID nodeId = node->getId();
    for (PointsTo::iterator piter = getPts(nodeId).begin(), epiter =
                getPts(nodeId).end(); piter != epiter; ++piter)
    {
        NodeID ptd = *piter;
        // handle load
        for (ConstraintNode::const_iterator it = node->outgoingLoadsBegin(),
                eit = node->outgoingLoadsEnd(); it != eit; ++it)
        {
            if (const FSLoadCGEdge* fsLoadEdge = SVFUtil::dyn_cast<FSLoadCGEdge>(*it))
            {
                if (processLoad(ptd, fsLoadEdge))
                    pushIntoWorklist(ptd);
            }
        }

        // handle store
        for (ConstraintNode::const_iterator it = node->incomingStoresBegin(),
                eit = node->incomingStoresEnd(); it != eit; ++it)
        {
            if (const FSStoreCGEdge* fsStoreEdge = SVFUtil::dyn_cast<FSStoreCGEdge>(*it))
            {
                if (processStore(ptd, fsStoreEdge))
                    pushIntoWorklist((*it)->getSrcID());
            }
        }
    }
}
*/

bool FlowSensitiveCG::handleStore(NodeID node, const ConstraintEdge* store)
{
    if (pag->isConstantObj(node) || pag->getGNode(store->getSrcID())->isPointer() == false)
        return false;

    numOfProcessedStore++;

    const StoreCGEdge* fsStoreEdge = SVFUtil::dyn_cast<StoreCGEdge>(store);

    auto src = fsStoreEdge->getSrcID();
    auto svfgid = fsStoreEdge->getSVFGID();

    NodeID dst = getAddrDef(node, svfgid);
    // add ConstraintNode
    // addConstraintNode(new ConstraintNode(dst), dst);

    bool changed = false;
    NodeID singleton;
    bool isSU = isStrongUpdate(fsStoreEdge, singleton);
    if (isSU)
    {
        clearFullPts(dst);
        unionPts(dst, src);
        changed = addCopyEdge(src, dst);
    }
    else
    {
        unionPts(dst, src);
        changed = addCopyEdge(src, dst);
    }

    return changed;
}

bool FlowSensitiveCG::handleLoad(NodeID node, const ConstraintEdge* load)
{
    if (pag->isConstantObj(node) || pag->getGNode(load->getDstID())->isPointer() == false)
        return false;

    numOfProcessedLoad++;

    const LoadCGEdge* fsLoadEdge = SVFUtil::dyn_cast<LoadCGEdge>(load);

    auto svfgid = fsLoadEdge->getSVFGID();
    auto src = getAddrDef(node, svfgid);

    auto dst = fsLoadEdge->getDstID();

    unionPts(dst, src);

    return addCopyEdge(src, dst);
}

NodeID FlowSensitiveCG::getAddrDef(NodeID consgid, NodeID svfgid)
{
    return fsconsCG->pairToidMap[NodePair(consgid, svfgid)];
}

bool FlowSensitiveCG::isStrongUpdate(const StoreCGEdge* node, NodeID& singleton)
{
    bool isSU = false;
    if (const ConstraintEdge* store = SVFUtil::dyn_cast<ConstraintEdge>(node))
    {
        const PointsTo& dstCPSet = getPts(store->getDstID());
        if (dstCPSet.count() == 1)
        {
            /// Find the unique element in cpts
            PointsTo::iterator it = dstCPSet.begin();
            singleton = *it;

            // Strong update can be made if this points-to target is not heap, array or field-insensitive.
            if (!isHeapMemObj(singleton) && !isArrayMemObj(singleton)
                    && pag->getBaseObj(singleton)->isFieldInsensitive() == false
                    && !isLocalVarInRecursiveFun(singleton))
            {
                isSU = true;
            }
        }
    }
    return isSU;
}