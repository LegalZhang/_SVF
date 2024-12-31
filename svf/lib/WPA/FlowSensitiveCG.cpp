//
// Created by Jiahao Zhang on 18/11/2024.
//

#include "Graphs/FSConsG.h"
#include "WPA/Andersen.h"
#include "WPA/FlowSensitive.h"
#include "WPA/FlowSensitiveCG.h"

using namespace SVF;

constexpr NodeID INVALID_NODE_ID = -1;

/*!
 * Initialize analysis
 */
void FlowSensitiveCG::initialize()
{
    resetData();
    /// Build SVFIR
    PointerAnalysis::initialize();

    /// Create Andersen statistic class
    stat = new AndersenStat(this);

    ander = AndersenWaveDiff::createAndersenWaveDiff(getPAG());
    svfg = memSSA.buildPTROnlySVFG(ander);

    /// Build Flow-Sensitive Constraint Graph
    fsconsCG = new FSConsG(svfg);
    if (!fsconsCG) {
        std::cerr << "Error: Failed to initialize fsconsCG!" << std::endl;
        return;
    }
    setGraph(fsconsCG);
    if (Options::SVFG2CG())
        fsconsCG->dump("fsconsg_initial");

    /// Initialize worklist
    processAllAddr();

    setDetectPWC(true);   // Standard wave propagation always collapses PWCs
}

/*!
 * Finalize analysis
 */
void FlowSensitiveCG::finalize()
{
    if (Options::SVFG2CG())
        fsconsCG->dump("fsconsg_final");

    // Debug: Check fsconsCG and svfg before cleanup
    if (!fsconsCG) {
        std::cerr << "Error: fsconsCG is null in finalize!" << std::endl;
    }
    if (!svfg) {
        std::cerr << "Error: svfg is null in finalize!" << std::endl;
    }

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

        // Debug check for invalid node IDs
        if (nodeId == INVALID_NODE_ID) {
            std::cerr << "Error: Invalid NodeID found in nodeStack!\n";
            continue;
        }

        // Debug log for processing a node
        std::cerr << "Processing NodeID: " << nodeId << std::endl;

        collapsePWCNode(nodeId);

        // Debug: Ensure collapsePWCNode didn't corrupt state
        if (!fsconsCG->getConstraintNode(nodeId)) {
            std::cerr << "Error: NodeID " << nodeId << " is invalid after collapsePWCNode!\n";
            continue;
        }

        processNode(nodeId);
        collapseFields();

        // Debug: Verify graph consistency after node processing
        std::cerr << "Finished processing NodeID: " << nodeId << std::endl;
    }

    // New nodes will be inserted into workList during processing.
    while (!isWorklistEmpty())
    {
        NodeID nodeId = popFromWorklist();

        // Debug check for invalid worklist node IDs
        if (nodeId == INVALID_NODE_ID) {
            std::cerr << "Error: Invalid NodeID found in workList!\n";
            continue;
        }

        std::cerr << "Post-processing NodeID: " << nodeId << std::endl;

        // process nodes in worklist
        postProcessNode(nodeId);

        // Debug: Verify state after post-processing
        std::cerr << "Finished post-processing NodeID: " << nodeId << std::endl;
    }
}


/*!
 * Process edge PAGNode
 */
void FlowSensitiveCG::processNode(NodeID nodeId)
{
    // This node may be merged during collapseNodePts() which means it is no longer a rep node
    // in the graph. Only rep node needs to be handled.
    if (sccRepNode(nodeId) != nodeId)
        return;

    double propStart = stat->getClk();
    ConstraintNode* node = fsconsCG->getConstraintNode(nodeId);
    handleCopyGep(node);
    double propEnd = stat->getClk();
    timeOfProcessCopyGep += (propEnd - propStart) / TIMEINTERVAL;
}

/*!
 * Post process node
 */
void FlowSensitiveCG::postProcessNode(NodeID nodeId)
{
    double insertStart = stat->getClk();

    ConstraintNode* node = fsconsCG->getConstraintNode(nodeId);

    if (!node) {
        std::cerr << "Error: ConstraintNode is null for NodeID: " << nodeId << std::endl;
        return;
    }

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
    bool changed = false;
    for (PointsTo::iterator piter = getPts(node).begin(), epiter = getPts(node).end();
            piter != epiter; ++piter)
    {
        if (processStore(*piter, store))
        {
            changed = true;
        }
    }
    return changed;
}

bool FlowSensitiveCG::processStore(NodeID node, const ConstraintEdge* store)
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

    NodeID singleton;
    bool isSU = isStrongUpdate(fsStoreEdge, singleton);
    if (isSU)
    {
        clearFullPts(dst);
        // unionPts(dst, src);
        return addCopyEdge(src, dst);
    }
    else
    {
        // unionPts(dst, src);
        return addCopyEdge(src, dst);
    }
}

bool FlowSensitiveCG::handleLoad(NodeID node, const ConstraintEdge* load)
{
    bool changed = false;

    for (PointsTo::iterator piter = getPts(node).begin(), epiter = getPts(node).end();
            piter != epiter; ++piter)
    {
        if (processLoad(*piter, load))
        {
            changed = true;
        }
    }
    return changed;
}

bool FlowSensitiveCG::processLoad(NodeID node, const ConstraintEdge* load)
{
    if (pag->isConstantObj(node) || pag->getGNode(load->getDstID())->isPointer() == false)
        return false;

    numOfProcessedLoad++;

    const LoadCGEdge* fsLoadEdge = SVFUtil::dyn_cast<LoadCGEdge>(load);

    auto svfgid = fsLoadEdge->getSVFGID();
    auto src = getAddrDef(node, svfgid);

    auto dst = fsLoadEdge->getDstID();

    // unionPts(dst, src);

    return addCopyEdge(src, dst);
}

NodeID FlowSensitiveCG::getAddrDef(NodeID consgid, NodeID svfgid)
{
    auto it = fsconsCG->pairToidMap.find(NodePair(consgid, svfgid));
    if (it == fsconsCG->pairToidMap.end()) {
        std::cerr << "Error: NodePair(" << consgid << ", " << svfgid << ") not found in pairToidMap!" << std::endl;
        return INVALID_NODE_ID;
    }
    return it->second;
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