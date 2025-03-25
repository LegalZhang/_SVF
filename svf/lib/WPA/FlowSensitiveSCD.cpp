//
// Created by Jiahao Zhang on 23/3/2025.
//

#include "WPA/AndersenPWC.h"
#include "MemoryModel/PointsTo.h"
#include "Util/Options.h"
#include "Graphs/FSConsG.h"
#include "WPA/FlowSensitiveSCD.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;

void FlowSensitiveSCD::initialize()
{
    std::cout << "啦啦啦啦啦啦FS--ANDER啦啦啦啦啦啦" << std::endl;
    resetData();
    /// Build SVFIR
    PointerAnalysis::initialize();

    ander = AndersenWaveDiff::createAndersenWaveDiff(getPAG());
    svfg = memSSA.buildPTROnlySVFG(ander);

    // /// Create Andersen statistic class
    // stat = new AndersenStat(this);

    /// Build Flow-Sensitive Constraint Graph
    fsconsCG = new FSConsG(svfg);
    setGraph(fsconsCG);
    if (Options::SVFG2CG())
        fsconsCG->dump("fsconsg_initial");

    std::cout << "啦啦啦啦啦啦END--FS--ANDER啦啦啦啦啦啦" << std::endl;

    std::cout << "Number of Load = " << fsconsCG->NumberOfLoad << std::endl;
    std::cout << "Number of Store = " << fsconsCG->NumberOfStore << std::endl;
    std::cout << "Number of Initial Copy = " << fsconsCG->NumberOfInitialCopy << std::endl;
    exit(0);
    /// Create Andersen statistic class
    stat = new AndersenStat(this);

    /// Initialize worklist
    processAllAddr();

    setDetectPWC(true);   // Standard wave propagation always collapses PWCs
}

void FlowSensitiveSCD::processAllAddr()
{
    for (ConstraintGraph::const_iterator nodeIt = fsconsCG->begin(), nodeEit = fsconsCG->end(); nodeIt != nodeEit; nodeIt++)
    {
        ConstraintNode * cgNode = nodeIt->second;
        for (ConstraintNode::const_iterator it = cgNode->incomingAddrsBegin(), eit = cgNode->incomingAddrsEnd();
                it != eit; ++it)
            processAddr(SVFUtil::cast<AddrCGEdge>(*it));
    }
}

void FlowSensitiveSCD::finalize()
{
    if (Options::SVFG2CG())
        fsconsCG->dump("fsconsg_final");

    std::cout << "哈哈哈哈哈哈END--FS--ANDER哈哈哈哈哈哈" << std::endl;

    BVDataPTAImpl::finalize();
}

void FlowSensitiveSCD::solveConstraints()
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

/// AndersenSCD::solveWorklist()
void FlowSensitiveSCD::solveWorklist()
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
            ConstraintNode* node = fsconsCG->getConstraintNode(nodeId);
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
        ConstraintNode* node = fsconsCG->getConstraintNode(nodeId);
        handleLoadStore(node);
        double insertEnd = stat->getClk();
        timeOfProcessLoadStore += (insertEnd - insertStart) / TIMEINTERVAL;
    }
}

/// AndersenSCD::SCCDetect()
NodeStack& FlowSensitiveSCD::SCCDetect()
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

/// AndersenSCD::PWCDetect()
void FlowSensitiveSCD::PWCDetect()
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

/// AndersenSCD::handleCopyGep()
void FlowSensitiveSCD::handleCopyGep(ConstraintNode* node)
{
    NodeID nodeId = node->getId();

    if (!Options::DetectPWC() && getSCCDetector()->subNodes(nodeId).count() > 1)
        processPWC(node);
    else if(isInWorklist(nodeId))
        FlowSensitiveSCD::handleCopyGepOriginal(node);
}

/// AndersenSCD::processPWC()
void FlowSensitiveSCD::processPWC(ConstraintNode* rep)
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
            ConstraintNode *node = fsconsCG->getConstraintNode(nodeId);
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

void FlowSensitiveSCD::handleCopyGepOriginal(ConstraintNode* node)
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

/// AndersenSCD::handleLoadStore()
void FlowSensitiveSCD::handleLoadStore(ConstraintNode* node)
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

/// AndersenSCD::processAddr()
void FlowSensitiveSCD::processAddr(const AddrCGEdge *addr)
{
    numOfProcessedAddr++;

    NodeID dst = addr->getDstID();
    NodeID src = addr->getSrcID();
    addPts(dst,src);
    addSccCandidate(dst);
}

/// AndersenSCD::addCopyEdge()
bool FlowSensitiveSCD::addCopyEdge(NodeID src, NodeID dst)
{
    if (FlowSensitiveSCD::addCopyEdgeOriginal(src, dst))
    {
        addSccCandidate(src);
        return true;
    }
    return false;
}

/// Add copy edge on constraint graph
bool FlowSensitiveSCD::addCopyEdgeOriginal(NodeID src, NodeID dst)
{
    if (fsconsCG->addCopyCGEdge(src, dst))
    {
        updatePropaPts(src, dst);
        return true;
    }
    return false;
}


/// AndersenSCD::updateCallGraph()
bool FlowSensitiveSCD::updateCallGraph(const PointerAnalysis::CallSiteToFunPtrMap& callsites)
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






/*
 * Merge constraint graph nodes based on SCC cycle detected.
 */
void FlowSensitiveSCD::mergeSccCycle()
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
void FlowSensitiveSCD::mergeSccNodes(NodeID repNodeId, const NodeBS& subNodes)
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

void FlowSensitiveSCD::mergeNodeToRep(NodeID nodeId,NodeID newRepId)
{

    if (mergeSrcToTgt(nodeId,newRepId))
        fsconsCG->setPWCNode(newRepId);
}

/*!
 * merge nodeId to newRepId. Return true if the newRepId is a PWC node
 */
bool FlowSensitiveSCD::mergeSrcToTgt(NodeID nodeId, NodeID newRepId)
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
void FlowSensitiveSCD::updateNodeRepAndSubs(NodeID nodeId, NodeID newRepId)
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
inline void FlowSensitiveSCD::collapsePWCNode(NodeID nodeId)
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
bool FlowSensitiveSCD::collapseNodePts(NodeID nodeId)
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

inline void FlowSensitiveSCD::collapseFields()
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
bool FlowSensitiveSCD::collapseField(NodeID nodeId)
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

void FlowSensitiveSCD::processNode(NodeID nodeId)
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

bool FlowSensitiveSCD::processCopy(NodeID node, const ConstraintEdge* edge)
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

bool FlowSensitiveSCD::processGep(NodeID, const GepCGEdge* edge)
{
    const PointsTo& srcPts = getDiffPts(edge->getSrcID());
    return processGepPts(srcPts, edge);
}

bool FlowSensitiveSCD::processGepPts(const PointsTo& pts, const GepCGEdge* edge)
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
void FlowSensitiveSCD::postProcessNode(NodeID nodeId)
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

bool FlowSensitiveSCD::handleLoad(NodeID node, const ConstraintEdge* load)
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

bool FlowSensitiveSCD::processLoad(NodeID node, const ConstraintEdge* load)
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

bool FlowSensitiveSCD::handleStore(NodeID node, const ConstraintEdge* store)
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

bool FlowSensitiveSCD::processStore(NodeID node, const ConstraintEdge* store)
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

NodeID FlowSensitiveSCD::getAddrDef(NodeID consgid, NodeID svfgid)
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


bool FlowSensitiveSCD::isStrongUpdate(const StoreCGEdge* node, NodeID& singleton)
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
