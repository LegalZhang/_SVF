//
// Created by Jiahao Zhang on 18/11/2024.
//

#include "Graphs/FSConsG.h"
#include "WPA/Andersen.h"
#include "WPA/FlowSensitiveCG.h"

using namespace SVF;

void FlowsensiveCG::handleLoadStore(ConstraintNode* node)
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
