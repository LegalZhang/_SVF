//
// Created by Jiahao Zhang on 19/8/2024.
//


#include "ConsG.h"
#include "SVFG.h"

namespace SVF
{

/*!
 * Constraint graph built from SVFG
 */

class  FSConsG : public ConstraintGraph
{
public:
    /// Constructor
    FSConsG(SVFG* svfg): ConstraintGraph(svfg->getPAG())
    {
        edgeIndex = 0;
        buildSVFG2CG(svfg);
    }

    typedef Map<NodeID, NodePair> IDToPairMap;
    typedef Map<NodePair, NodeID> PairToIDMap;

protected:
    NodeID totalCGNode;
    SVFG* svfg;

    void buildSVFG2CG(SVFG* svfg);
};

} // End namespace SVF
