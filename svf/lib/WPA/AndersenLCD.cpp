//
// Created by Jiahao Zhang on 18/6/2024.
//

#include "WPA/AndersenLCD.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;

AndersenLCD* AndersenLCD::lcdAndersen = nullptr;

void AndersenLCD::analyze()
{
    AndersenBase::initialize();
    initWorklist();
    do
    {
        reanalyze = false;
        solveWorklist();
        if (updateCallGraph(getIndirectCallsites()))
            reanalyze = true;
    } while (reanalyze);
    finalize();
}

void AndersenLCD::solveWorklist()
{
    // process all address edges
    for (ConstraintGraph::const_iterator nodeIt = consCG->begin(),
                                         nodeEit = consCG->end();
         nodeIt != nodeEit; nodeIt++)
    {
        ConstraintNode* cgNode = nodeIt->second;
        for (ConstraintNode::const_iterator it = cgNode->incomingAddrsBegin(),
                                            eit = cgNode->incomingAddrsEnd();
             it != eit; ++it)
        {
            numOfProcessedAddr++;

            const AddrCGEdge* addr = SVFUtil::cast<AddrCGEdge>(*it);
            NodeID dst = addr->getDstID();
            NodeID src = addr->getSrcID();
            if (addPts(dst, src))
                pushIntoWorklist(dst);
        }
    }

    // process copy, load, store and gep edges
    while (!worklist.empty())
    {
        ConstraintNode* node = consCG->getConstraintNode(popFromWorklist());

        NodeID nodeId = node->getId();

        if (sccRepNode(nodeId) != nodeId)
            return;

        for (PointsTo::iterator piter = getPts(nodeId).begin(),
                                epiter = getPts(nodeId).end();
             piter != epiter; ++piter)
        {
            NodeID ptd = *piter;
            // process copy
            for (ConstraintEdge* edge : node->getCopyOutEdges())
            {

                LCDprocessCopy(nodeId, edge);

                /*
                NodeID dst = edge->getDstID();
                if (unionPts(dst, nodeId))
                {
                    pushIntoWorklist(dst);
                }
                */
            }

            // process gep
            for (ConstraintEdge* edge : node->getGepOutEdges())
            {
                /*
                if (GepCGEdge* gepEdge = SVFUtil::dyn_cast<GepCGEdge>(edge))
                    processGep(nodeId, gepEdge);
                */

                numOfProcessedGep++;

                if (SVFUtil::dyn_cast<GepCGEdge>(edge))
                {

                    PointsTo tmpDstPts;
                    if (SVFUtil::isa<VariantGepCGEdge>(edge))
                    {
                        for (NodeID o : getPts(nodeId))
                        {
                            if (consCG->isBlkObjOrConstantObj(o))
                            {
                                tmpDstPts.set(o);
                            }
                            if (!isFieldInsensitive(o))
                            {
                                setObjFieldInsensitive(o);
                                consCG->addNodeToBeCollapsed(
                                    consCG->getBaseObjVar(o));
                            }
                            NodeID baseId = consCG->getFIObjVar(o);
                            tmpDstPts.set(baseId);
                        }
                    }
                    else if (const NormalGepCGEdge* normalGepEdge =
                                 SVFUtil::dyn_cast<NormalGepCGEdge>(edge))
                    {
                        for (NodeID o : getPts(nodeId))
                        {
                            if (consCG->isBlkObjOrConstantObj(o) ||
                                isFieldInsensitive(o))
                            {
                                tmpDstPts.set(o);
                                continue;
                            }

                            NodeID fieldSrcPtdNode = consCG->getGepObjVar(
                                o, normalGepEdge->getAccessPath()
                                       .getConstantStructFldIdx());
                            tmpDstPts.set(fieldSrcPtdNode);
                        }
                    }
                    else
                    {
                        assert(
                            false &&
                            "Andersen::processGepPts: New type GEP edge type?");
                    }
                    NodeID dstId = edge->getDstID();
                    if (unionPts(dstId, tmpDstPts))
                    {
                        pushIntoWorklist(dstId);
                    }
                }
            }

            // process load
            for (ConstraintNode::const_iterator it = node->outgoingLoadsBegin(),
                                                eit = node->outgoingLoadsEnd();
                 it != eit; ++it)
            {
                if (processLoad(ptd, *it))
                    pushIntoWorklist(ptd);
                /*
                numOfProcessedLoad++;

                if (!pag->isConstantObj(ptd) &&
                    pag->getGNode((*it)->getDstID())->isPointer())
                {
                    NodeID dst = (*it)->getDstID();
                    if (addCopyEdge(ptd, dst))
                    {
                        pushIntoWorklist(dst);
                    }
                }
                */
            }

            // process store
            for (ConstraintNode::const_iterator
                     it = node->incomingStoresBegin(),
                     eit = node->incomingStoresEnd();
                 it != eit; ++it)
            {
                if (processStore(ptd, *it))
                    pushIntoWorklist((*it)->getSrcID());
                /*
                numOfProcessedStore++;

                if (!pag->isConstantObj(ptd) &&
                    pag->getGNode((*it)->getSrcID())->isPointer())
                {
                    NodeID src = (*it)->getSrcID();
                    if (addCopyEdge(src, ptd))
                    {
                        pushIntoWorklist((*it)->getSrcID());
                    }
                }
                */
            }
        }
    }
}

bool AndersenLCD::LCDprocessCopy(NodeID node, const ConstraintEdge* edge)
{
    numOfProcessedCopy++;

    AndersenLCD::ConstraintEdgeSet R;
    NodeID before = edge->getSrcID();
    NodeID after = edge->getDstID();
    auto ptsSrc = Andersen::getDiffPts(before);
    auto ptsDst = Andersen::getDiffPts(after);
    if (ptsSrc == ptsDst && R.find(edge) == R.end())
    {
        numOfSCCDetection++;

        double sccStart = stat->getClk();
        getSCCDetector()->find();
        double sccEnd = stat->getClk();
        timeOfSCCDetection += (sccEnd - sccStart) / TIMEINTERVAL;

        double mergeStart = stat->getClk();
        mergeSccCycle();
        double mergeEnd = stat->getClk();
        timeOfSCCMerges += (mergeEnd - mergeStart) / TIMEINTERVAL;

        R.insert(edge);

        return true;
    }
    NodeID dst = edge->getDstID();
    if (unionPts(dst, node))
        pushIntoWorklist(dst);
    return true;
}
