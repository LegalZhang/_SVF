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
    std::cout << "啦啦啦啦啦啦FS--ANDER啦啦啦啦啦啦" << std::endl;
    resetData();
    /// Build SVFIR
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

    std::cout << "啦啦啦啦啦啦END--FS--ANDER啦啦啦啦啦啦" << std::endl;
    /// Initialize worklist
    processAllAddr();

    setDetectPWC(true);   // Standard wave propagation always collapses PWCs
}

void FlowSensitiveCG::processAllAddr()
{
    for (ConstraintGraph::const_iterator nodeIt = fsconsCG->begin(), nodeEit = fsconsCG->end(); nodeIt != nodeEit; nodeIt++)
    {
        ConstraintNode * cgNode = nodeIt->second;
        for (ConstraintNode::const_iterator it = cgNode->incomingAddrsBegin(), eit = cgNode->incomingAddrsEnd();
                it != eit; ++it)
            processAddr(SVFUtil::cast<AddrCGEdge>(*it));
    }
}

void FlowSensitiveCG::processAddr(const AddrCGEdge* addr)
{
    numOfProcessedAddr++;

    NodeID dst = addr->getDstID();
    NodeID src = addr->getSrcID();
    if(addPts(dst,src))
        pushIntoWorklist(dst);
}

void FlowSensitiveCG::finalize()
{
    if (Options::SVFG2CG())
        fsconsCG->dump("fsconsg_final");

    BVDataPTAImpl::finalize();
}

void FlowSensitiveCG::solveConstraints()
{
    // Start solving constraints
    DBOUT(DGENERAL, outs() << SVFUtil::pasMsg("Start Solving Constraints\n"));

    bool limitTimerSet = SVFUtil::startAnalysisLimitTimer(Options::AnderTimeLimit());

    initWorklist();
    do
    {
        numOfIteration++;
        if (0 == numOfIteration % iterationForPrintStat)
            printStat();

        reanalyze = false;

        solveWorklist();

        if (updateCallGraph(getIndirectCallsites()))
            reanalyze = true;

    }
    while (reanalyze);

    // Analysis is finished, reset the alarm if we set it.
    SVFUtil::stopAnalysisLimitTimer(limitTimerSet);

    DBOUT(DGENERAL, outs() << SVFUtil::pasMsg("Finish Solving Constraints\n"));
}

void FlowSensitiveCG::solveWorklist()
{
    // Initialize the nodeStack via a whole SCC detection
    // Nodes in nodeStack are in topological order by default.
    NodeStack& nodeStack = SCCDetect();


    /// TODO insert graph folding
    /// A Function : 遍历SCCDetect后的图，遍历所有Copy边，取出所有Copy边，判断Copy边的dst能不能被merge到src上面
    /// 被merge到src的dst不能是address-taken variables（isMemObject）
    /// top-level的dst：incomingCopyEdge是不是有且只有取出来的Copy边那一条（incomingCopyEdge和incomingGepEdge和incomingLoadEdge和incomingDirectEdge）
    /// 修改NodeToRepMap
    /// 先统计起来需要merge的dst node，然后遍历结束后统一修改

    // graphFolding();

    // Process nodeStack and put the changed nodes into workList.
    while (!nodeStack.empty())
    {

        NodeID nodeId = nodeStack.top();
        nodeStack.pop();

        // If a node is not in NodeToRepMap, skip it.
        // if (!isRepNode(nodeId))
        //     continue;

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

void FlowSensitiveCG::graphFolding()
{
    for (ConstraintGraph::const_iterator nodeIt = fsconsCG->begin(), nodeEit = fsconsCG->end(); nodeIt != nodeEit; nodeIt++)
    {
        NodeID node = nodeIt->first;
        ConstraintNode* dstNode = nodeIt->second;
        // node is not an address-taken variable and only has one incoming copy edge
        if (!SVFUtil::isa<ObjVar>(nodeIt->second) &&
            hasOnlyOneIncomingCopyEdge(node))
        {
            ConstraintEdge* singleEdge = *dstNode->getCopyInEdges().begin();

            NodeID srcID = singleEdge->getSrcID();
            ConstraintNode* srcNode = fsconsCG->getConstraintNode(srcID);

            for (auto &entry : fsconsCG->nodeToRepMap) {
                if (entry.second == node) {
                    entry.second = srcID;
                }
            }

            for (ConstraintEdge* outEdge : dstNode->getOutEdges()) {
                fsconsCG->reTargetSrcOfEdge(outEdge, srcNode);
            }

            // fsconsCG->removeConstraintNode(dstNode);
        }
    }
}

// 遍历所有Copy边，检查target
// 所有Copy边的dst能不能被merge到src上面
// 如果dst符合186行，那么就可以merge到src上面
bool FlowSensitiveCG::hasOnlyOneIncomingCopyEdge(NodeID nodeId)
{
    // Check if the node has only one incoming direct edge
    ConstraintNode *consGNode = fsconsCG->getConstraintNode(nodeId);
    if (consGNode->getCopyInEdges().size() == 1 && consGNode->getInEdges().size() == 1)
    {
        return true;
    }
    return false;
}

/*!
 * SCC detection on constraint graph
 */
NodeStack& FlowSensitiveCG::SCCDetect()
{
    numOfSCCDetection++;

    double sccStart = stat->getClk();
    WPAConstraintSolver::SCCDetect();
    double sccEnd = stat->getClk();

    timeOfSCCDetection +=  (sccEnd - sccStart)/TIMEINTERVAL;

    double mergeStart = stat->getClk();

    mergeSccCycle();

    double mergeEnd = stat->getClk();

    timeOfSCCMerges +=  (mergeEnd - mergeStart)/TIMEINTERVAL;

    return getSCCDetector()->topoNodeStack();
}

/*
 * Merge constraint graph nodes based on SCC cycle detected.
 */
void FlowSensitiveCG::mergeSccCycle()
{
    NodeStack revTopoOrder;
    NodeStack & topoOrder = getSCCDetector()->topoNodeStack();
    while (!topoOrder.empty())
    {
        NodeID repNodeId = topoOrder.top();
        topoOrder.pop();
        revTopoOrder.push(repNodeId);
        const NodeBS& subNodes = getSCCDetector()->subNodes(repNodeId);
        // merge sub nodes to rep node
        mergeSccNodes(repNodeId, subNodes);
    }

    // restore the topological order for later solving.
    while (!revTopoOrder.empty())
    {
        NodeID nodeId = revTopoOrder.top();
        revTopoOrder.pop();
        topoOrder.push(nodeId);
    }
}

/**
 * Union points-to of subscc nodes into its rep nodes
 * Move incoming/outgoing direct edges of sub node to rep node
 */
void FlowSensitiveCG::mergeSccNodes(NodeID repNodeId, const NodeBS& subNodes)
{
    for (NodeBS::iterator nodeIt = subNodes.begin(); nodeIt != subNodes.end(); nodeIt++)
    {
        NodeID subNodeId = *nodeIt;
        if (subNodeId != repNodeId)
        {
            mergeNodeToRep(subNodeId, repNodeId);
        }
    }
}

void FlowSensitiveCG::mergeNodeToRep(NodeID nodeId,NodeID newRepId)
{

    if (mergeSrcToTgt(nodeId,newRepId))
        fsconsCG->setPWCNode(newRepId);
}

/*!
 * merge nodeId to newRepId. Return true if the newRepId is a PWC node
 */
bool FlowSensitiveCG::mergeSrcToTgt(NodeID nodeId, NodeID newRepId)
{

    if(nodeId==newRepId)
        return false;

    /// union pts of node to rep
    updatePropaPts(newRepId, nodeId);
    unionPts(newRepId,nodeId);

    /// move the edges from node to rep, and remove the node
    ConstraintNode* node = fsconsCG->getConstraintNode(nodeId);
    bool pwc = fsconsCG->moveEdgesToRepNode(node, fsconsCG->getConstraintNode(newRepId));

    /// 1. if find gep edges inside SCC cycle, the rep node will become a PWC node and
    /// its pts should be collapsed later.
    /// 2. if the node to be merged is already a PWC node, the rep node will also become
    /// a PWC node as it will have a self-cycle gep edge.
    if(node->isPWCNode())
        pwc = true;

    /// set rep and sub relations
    updateNodeRepAndSubs(node->getId(),newRepId);

    fsconsCG->removeConstraintNode(node);

    return pwc;
}

/*
 * Updates subnodes of its rep, and rep node of its subs
 */
void FlowSensitiveCG::updateNodeRepAndSubs(NodeID nodeId, NodeID newRepId)
{
    fsconsCG->setRep(nodeId,newRepId);
    NodeBS repSubs;
    repSubs.set(nodeId);
    /// update nodeToRepMap, for each subs of current node updates its rep to newRepId
    //  update nodeToSubsMap, union its subs with its rep Subs
    NodeBS& nodeSubs = fsconsCG->sccSubNodes(nodeId);
    for(NodeBS::iterator sit = nodeSubs.begin(), esit = nodeSubs.end(); sit!=esit; ++sit)
    {
        NodeID subId = *sit;
        fsconsCG->setRep(subId,newRepId);
    }
    repSubs |= nodeSubs;
    fsconsCG->setSubs(newRepId,repSubs);
    fsconsCG->resetSubs(nodeId);
}

/**
 * Detect and collapse PWC nodes produced by processing gep edges, under the constraint of field limit.
 */
inline void FlowSensitiveCG::collapsePWCNode(NodeID nodeId)
{
    // If a node is a PWC node, collapse all its points-to target.
    // collapseNodePts() may change the points-to set of the nodes which have been processed
    // before, in this case, we may need to re-do the analysis.
    if (fsconsCG->isPWCNode(nodeId) && collapseNodePts(nodeId))
        reanalyze = true;
}

/**
 * Collapse node's points-to set. Change all points-to elements into field-insensitive.
 */
bool FlowSensitiveCG::collapseNodePts(NodeID nodeId)
{
    bool changed = false;
    const PointsTo& nodePts = getPts(nodeId);
    /// Points to set may be changed during collapse, so use a clone instead.
    PointsTo ptsClone = nodePts;
    for (PointsTo::iterator ptsIt = ptsClone.begin(), ptsEit = ptsClone.end(); ptsIt != ptsEit; ptsIt++)
    {
        NodeID pagId = fsconsCG->getPAGNodeID(*ptsIt);
        if (isFieldInsensitive(pagId))
            continue;

        if (collapseField(pagId))
            changed = true;
    }
    return changed;
}

inline void FlowSensitiveCG::collapseFields()
{
    while (fsconsCG->hasNodesToBeCollapsed())
    {
        NodeID node = fsconsCG->getNextCollapseNode();
        // collapseField() may change the points-to set of the nodes which have been processed
        // before, in this case, we may need to re-do the analysis.
        if (collapseField(node))
            reanalyze = true;
    }
}

/**
 * Collapse field. make struct with the same base as nodeId become field-insensitive.
 */
bool FlowSensitiveCG::collapseField(NodeID nodeId)
{
    /// Black hole doesn't have structures, no collapse is needed.
    /// In later versions, instead of using base node to represent the struct,
    /// we'll create new field-insensitive node. To avoid creating a new "black hole"
    /// node, do not collapse field for black hole node.

    NodeID pagId = fsconsCG->getPAGNodeID(nodeId);

    if (fsconsCG->isBlkObjOrConstantObj(pagId))
        return false;

    bool changed = false;

    double start = stat->getClk();

    // set base node field-insensitive.
    setObjFieldInsensitive(pagId);

    // replace all occurrences of each field with the field-insensitive node
    NodeID baseId = fsconsCG->getFIObjVar(pagId);
    NodeID baseRepNodeId = fsconsCG->sccRepNode(baseId);
    NodeBS & allFields = fsconsCG->getAllFieldsObjVars(baseId);
    for (NodeBS::iterator fieldIt = allFields.begin(), fieldEit = allFields.end(); fieldIt != fieldEit; fieldIt++)
    {
        NodeID fieldId = *fieldIt;
        if (fieldId != baseId)
        {
            // use the reverse pts of this field node to find all pointers point to it
            const NodeSet revPts = getRevPts(fieldId);
            for (const NodeID o : revPts)
            {
                // change the points-to target from field to base node
                clearPts(o, fieldId);
                addPts(o, baseId);
                pushIntoWorklist(o);

                changed = true;
            }
            // merge field node into base node, including edges and pts.
            NodeID fieldRepNodeId = fsconsCG->sccRepNode(fieldId);
            mergeNodeToRep(fieldRepNodeId, baseRepNodeId);
            if (fieldId != baseRepNodeId)
            {
                // gep node fieldId becomes redundant if it is merged to its base node who is set as field-insensitive
                // two node IDs should be different otherwise this field is actually the base and should not be removed.
                redundantGepNodes.set(fieldId);
            }
        }
    }

    if (fsconsCG->isPWCNode(baseRepNodeId))
        if (collapseNodePts(baseRepNodeId))
            changed = true;

    double end = stat->getClk();
    timeOfCollapse += (end - start) / TIMEINTERVAL;

    return changed;
}

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

void FlowSensitiveCG::handleCopyGep(ConstraintNode* node)
{
    NodeID nodeId = node->getId();
    computeDiffPts(nodeId);

    if (!getDiffPts(nodeId).empty())
    {
        for (ConstraintEdge* edge : node->getCopyOutEdges())
            // process copy
            processCopy(nodeId, edge);
        for (ConstraintEdge* edge : node->getGepOutEdges())
        {
            if (GepCGEdge* gepEdge = SVFUtil::dyn_cast<GepCGEdge>(edge))
                // process gep
                processGep(nodeId, gepEdge);
        }
    }
}

bool FlowSensitiveCG::processCopy(NodeID node, const ConstraintEdge* edge)
{

    numOfProcessedCopy++;

    assert((SVFUtil::isa<CopyCGEdge>(edge)) && "not copy/call/ret ??");
    NodeID dst = edge->getDstID();
    const PointsTo& srcPts = getDiffPts(node);

    bool changed = unionPts(dst, srcPts);
    if (changed)
        pushIntoWorklist(dst);
    return changed;

}

bool FlowSensitiveCG::processGep(NodeID, const GepCGEdge* edge)
{
    const PointsTo& srcPts = getDiffPts(edge->getSrcID());
    return processGepPts(srcPts, edge);
}

bool FlowSensitiveCG::processGepPts(const PointsTo& pts, const GepCGEdge* edge)
{
    numOfProcessedGep++;

    PointsTo tmpDstPts;
    if (SVFUtil::isa<VariantGepCGEdge>(edge))
    {
        // If a pointer is connected by a variant gep edge,
        // then set this memory object to be field insensitive,
        // unless the object is a black hole/constant.
        for (NodeID o : pts)
        {
            NodeID pagId = fsconsCG->getPAGNodeID(o);
            if (fsconsCG->isBlkObjOrConstantObj(pagId))
            {
                tmpDstPts.set(pagId);
                continue;
            }

            if (!isFieldInsensitive(pagId))
            {
                setObjFieldInsensitive(pagId);
                fsconsCG->addNodeToBeCollapsed(fsconsCG->getBaseObjVar(pagId));
            }

            // Add the field-insensitive node into pts.
            NodeID baseId = fsconsCG->getFIObjVar(pagId);
            tmpDstPts.set(baseId);
        }
    }
    else if (const NormalGepCGEdge* normalGepEdge = SVFUtil::dyn_cast<NormalGepCGEdge>(edge))
    {
        // TODO: after the node is set to field insensitive, handling invariant
        // gep edge may lose precision because offsets here are ignored, and the
        // base object is always returned.
        for (NodeID o : pts)
        {
            NodeID pagId = fsconsCG->getPAGNodeID(o);
            if (fsconsCG->isBlkObjOrConstantObj(pagId) || isFieldInsensitive(pagId))
            {
                tmpDstPts.set(pagId);
                continue;
            }

            NodeID fieldSrcPtdNode = fsconsCG->getGepObjVar(pagId, normalGepEdge->getAccessPath().getConstantStructFldIdx());
            tmpDstPts.set(fieldSrcPtdNode);
        }
    }
    else
    {
        assert(false && "Andersen::processGepPts: New type GEP edge type?");
    }

    NodeID dstId = edge->getDstID();
    if (unionPts(dstId, tmpDstPts))
    {
        pushIntoWorklist(dstId);
        return true;
    }

    return false;
}

/*!
 * Post process node
 */
void FlowSensitiveCG::postProcessNode(NodeID nodeId)
{
    double insertStart = stat->getClk();

    ConstraintNode* node = fsconsCG->getConstraintNode(nodeId);

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
    NodeID pagId = fsconsCG->getPAGNodeID(node);
    if (pag->isConstantObj(pagId) || pag->getGNode(fsconsCG->getPAGNodeID(load->getDstID()))->isPointer() == false)
        return false;

    numOfProcessedLoad++;

    const LoadCGEdge* fsLoadEdge = SVFUtil::dyn_cast<LoadCGEdge>(load);

    auto svfgid = fsLoadEdge->getSVFGID();
    auto src = getAddrDef(node, svfgid);

    auto dst = fsLoadEdge->getDstID();

    // unionPts(dst, src);

    return addCopyEdge(src, dst);
}

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
    NodeID pagId = fsconsCG->getPAGNodeID(node);
    if (pag->isConstantObj(pagId) || pag->getGNode(fsconsCG->getPAGNodeID(store->getSrcID()))->isPointer() == false)
        return false;

    numOfProcessedStore++;

    const StoreCGEdge* fsStoreEdge = SVFUtil::dyn_cast<StoreCGEdge>(store);

    auto src = fsStoreEdge->getSrcID();
    auto svfgid = fsStoreEdge->getSVFGID();

    NodeID dst = getAddrDef(pagId, svfgid);
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

NodeID FlowSensitiveCG::getAddrDef(NodeID consgid, NodeID svfgid)
{
    auto it = fsconsCG->pairToidMap.find(NodePair(consgid, svfgid));
    if (it == fsconsCG->pairToidMap.end())
    {
        return consgid;
        // debug
        std::cout<<"[FlowSensitiveCG::getAddrDef] NodePair not found: ("<<consgid<<", "<<svfgid<<")"<<std::endl;
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
            if (!isHeapMemObj(singleton) && !isArrayMemObj(singleton))
            {
                assert(pag->getBaseObject(singleton)->isFieldInsensitive() == pag->getBaseObject(singleton)->isFieldInsensitive());
                if (pag->getBaseObject(singleton)->isFieldInsensitive() == false
                        && !isLocalVarInRecursiveFun(singleton))
                {
                    isSU = true;
                }
            }
        }
    }
    return isSU;
}

CopyCGEdge* ConstraintGraph::addCopyCGEdge(NodeID src, NodeID dst)
{

    ConstraintNode* srcNode = getConstraintNode(src);
    ConstraintNode* dstNode = getConstraintNode(dst);
    if (hasEdge(srcNode, dstNode, ConstraintEdge::Copy) || srcNode == dstNode)
        return nullptr;

    CopyCGEdge* edge = new CopyCGEdge(srcNode, dstNode, edgeIndex++);

    bool inserted = directEdgeSet.insert(edge).second;
    (void)inserted; // Suppress warning of unused variable under release build
    assert(inserted && "new CopyCGEdge not added??");

    srcNode->addOutgoingCopyEdge(edge);
    dstNode->addIncomingCopyEdge(edge);
    return edge;
}
