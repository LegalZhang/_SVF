//
// Created by Jiahao Zhang on 18/11/2024.
//

#include "Graphs/FSConsG.h"
#include "WPA/Andersen.h"
#include "WPA/FlowSensitiveCG.h"

using namespace SVF;

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
            if (processLoad(ptd, *it))
                pushIntoWorklist(ptd);
        }

        // handle store
        for (ConstraintNode::const_iterator it = node->incomingStoresBegin(),
                eit = node->incomingStoresEnd(); it != eit; ++it)
        {
            if (processStore(ptd, *it))
                pushIntoWorklist((*it)->getSrcID());
        }
    }
}

bool FlowsensitiveCG::processStore(NodeID node, const ConstraintEdge* store)
{
    if (pag->isConstantObj(node) || pag->getGNode(store->getSrcID())->isPointer() == false)
        return false;

    numOfProcessedStore++;

    NodeID src = store->getSrcID();
    return addCopyEdge(src, node);

    /*
    const PointsTo & dstPts = getPts(store->getPAGDstNodeID());
    if (dstPts.empty())
        return false;

    bool changed = false;
    NodeID singleton;
    bool isSU = isStrongUpdate(store, singleton);
    if (isSU)
    {
        svfgHasSU.set(store->getId());
        if (strongUpdateOutFromIn(store, singleton))
            changed = true;
    }
    else
    {
        svfgHasSU.reset(store->getId());
        if (weakUpdateOutFromIn(store))
            changed = true;
    }

    // *p = q, the type of q must be a pointer
    if(getPts(store->getPAGSrcNodeID()).empty() == false && store->getPAGSrcNode()->isPointer())
    {
        for (PointsTo::iterator it = dstPts.begin(), eit = dstPts.end(); it != eit; ++it)
        {
            NodeID ptd = *it;

            if (pag->isConstantObj(ptd))
                continue;

            if (unionPtsFromTop(store, store->getPAGSrcNodeID(), ptd))
                changed = true;
        }
    }
    */
}

