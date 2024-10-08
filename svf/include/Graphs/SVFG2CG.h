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

/*!
 * Constraint graph built from SVFG
 */

class  SVFG2CG : public SVF::ConstraintGraph
{
public:


protected:
    SVF::SVFG* svfg;
    NodeToRepMap nodeToRepMap;
    NodeToSubsMap nodeToSubsMap;
    WorkList nodesToBeCollapsed;
    SVF::EdgeID edgeIndex;

    SVF::ConstraintEdge::ConstraintEdgeSetTy AddrCGEdgeSet;
    SVF::ConstraintEdge::ConstraintEdgeSetTy directEdgeSet;
    SVF::ConstraintEdge::ConstraintEdgeSetTy LoadCGEdgeSet;
    SVF::ConstraintEdge::ConstraintEdgeSetTy StoreCGEdgeSet;

    void buildSVFG2CG();
};

#endif //SVFG2CG_H
