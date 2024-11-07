//
// Created by Jiahao Zhang on 19/8/2024.
//


#include "ConsG.h"
#include "SVFG.h"

namespace SVF
{

class FSLoadCGEdge : public LoadCGEdge
{
public:
    /// Constructor
    FSLoadCGEdge(ConstraintNode* s, ConstraintNode* d, EdgeID id, NodeID fsID) : LoadCGEdge(s, d, id)
    {
    }

};

class FSStoreCGEdge : public StoreCGEdge
{
public:
    /// Constructor
    FSStoreCGEdge(ConstraintNode* s, ConstraintNode* d, EdgeID id, NodeID fsID) : StoreCGEdge(s, d, id)
    {
    }

};

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

    /// Add Flow-sensitive Load edge
    FSLoadCGEdge* addFSLoadCGEdge(NodeID src, NodeID dst, NodeID fsID);
    /// Add Flow-sensitive Store edge
    FSStoreCGEdge* addFSStoreCGEdge(NodeID src, NodeID dst, NodeID fsID);

protected:
    NodeID totalCGNode;
    SVFG* svfg;
    IDToPairMap idTopairMap;
    PairToIDMap pairToidMap;

    void buildSVFG2CG(SVFG* svfg);
};

} // End namespace SVF
