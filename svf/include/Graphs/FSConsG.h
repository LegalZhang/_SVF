//
// Created by Jiahao Zhang on 19/8/2024.
//

#ifndef FSCONSG_H
#define FSCONSG_H

#include "ConsG.h"
#include "SVFG.h"

namespace SVF
{

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

    NodeID getPAGNodeID (NodeID fsconsgid) const
    {
        auto it = idTopairMap.find(fsconsgid);
        if (it != idTopairMap.end()) {
            return it->second.first;
        }
        return fsconsgid;
    }

    inline bool insertMappingIntoPairToIDMap(const NodePair &key, NodeID value) {
        auto ret = pairToidMap.insert({ key, value });
        if (!ret.second) {
            assert(ret.first->second == value && "Duplicate key with inconsistent value in pairToidMap");
            return false;
        }
        return true;
    }

    inline bool insertMappingIntoIDToPairMap(NodeID key, const NodePair &value) {
        auto ret = idTopairMap.insert({ key, value });
        if (!ret.second) {
            assert(ret.first->second == value && "Duplicate key with inconsistent mapping in idTopairMap");
            return false;
        }
        return true;
    }

    inline void insertConstraintMapping(NodeID pagid, NodeID svfgid, NodeID fsconsgid) {
        NodePair np(pagid, svfgid);
        insertMappingIntoPairToIDMap(np, fsconsgid);
        insertMappingIntoIDToPairMap(fsconsgid, np);
    }

    typedef Map<NodeID, NodePair> IDToPairMap; // FSConsGNodeID to (PAGNodeID, SVFGNodeID)
    typedef Map<NodePair, NodeID> PairToIDMap; // (PAGNodeID, SVFGNodeID) to FSConsGNodeID

protected:
    u32_t totalCGNode;
    // SVFG* svfg;
    IDToPairMap idTopairMap;
    PairToIDMap pairToidMap;

    void buildSVFG2CG(SVFG* svfg);

public:
    /// Dump graph into dot file
    void dump(std::string name);
    /// Print CG into terminal
    void print();

    /// View graph from the debugger.
    void view();
};

template<>
struct GenericGraphTraits<SVF::FSConsG*> : public GenericGraphTraits<SVF::ConstraintGraph*> {
    using NodeRef = SVF::ConstraintNode*;
};

} // End namespace SVF

#endif // FSCONSG_H