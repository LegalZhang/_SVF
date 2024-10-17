//
// Created by Jiahao Zhang on 19/8/2024.
//

#ifndef SVFG2CG_H
#define SVFG2CG_H

#include "ConsG.h"
#include "SVFG.h"
#include "Graphs/ConsGEdge.h"
#include "Graphs/ConsGNode.h"
#include "MSSA/SVFGBuilder.h"

namespace SVF
{

/*!
 * Constraint graph built from SVFG
 */

class  SVFG2CG : public ConstraintGraph
{
public:
    typedef Map<NodeID, NodePair> IDToPairMap;
    typedef Map<NodePair, NodeID> PairToIDMap;

protected:
    SVFG* svfg;
};

} // End namespace SVF

#endif //SVFG2CG_H