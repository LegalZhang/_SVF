//
// Created by Jiahao Zhang on 19/8/2024.
//

#ifndef FSCONSG_H
#define FSCONSG_H

#include "ConsG.h"
#include "SVFG.h"

namespace SVF
{

/*
class FSLoadCGEdge : public LoadCGEdge
{
public:
    /// Constructor
    FSLoadCGEdge(ConstraintNode* s, ConstraintNode* d, EdgeID id, NodeID fsID) : LoadCGEdge(s, d, id), fsID_(fsID)
    {
    }

    /// Getter for fsID
    NodeID getSVFGID() const {
        return fsID_;
    }

private:
    NodeID fsID_;

};

class FSStoreCGEdge : public StoreCGEdge
{
public:
    /// Constructor
    FSStoreCGEdge(ConstraintNode* s, ConstraintNode* d, EdgeID id, NodeID fsID) : StoreCGEdge(s, d, id), fsID_(fsID)
    {
    }

    /// Getter for fsID
    NodeID getSVFGID() const {
        return fsID_;
    }

private:
    NodeID fsID_;

};
*/

/*!
 * Constraint graph built from SVFG
 */

class  FSConsG : public ConstraintGraph
{
    friend class FlowSensitiveCG;

public:
    /// Constructor
    FSConsG(SVFG* svfg): ConstraintGraph(svfg->getPAG())
    {
        edgeIndex = 0;
        buildSVFG2CG(svfg);
    }

    inline bool hasConstraintNodePair(const NodeID& pagid, const NodeID& svfgid) const
    {
        NodePair nodePair(pagid, svfgid);
        return pairToidMap.find(nodePair) != pairToidMap.end();
    }

    typedef Map<NodeID, NodePair> IDToPairMap;
    typedef Map<NodePair, NodeID> PairToIDMap;

protected:
    u32_t totalCGNode;
    // SVFG* svfg;
    IDToPairMap idTopairMap;
    PairToIDMap pairToidMap;

    void buildSVFG2CG(SVFG* svfg);
};

} // End namespace SVF

#endif // FSCONSG_H