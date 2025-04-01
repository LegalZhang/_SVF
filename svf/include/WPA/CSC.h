
#ifndef PROJECT_CSC_H
#define PROJECT_CSC_H

#include "Graphs/ConsG.h"
#include "Graphs/SCC.h"
#include "SVFIR/SVFValue.h" // for NodeBS
#include "Util/WorkList.h"
#include <limits.h>
#include <map>
#include <stack>

namespace SVF
{

typedef SCCDetection<ConstraintGraph *> CGSCC;

/*!
 * class CSC: cycle stride calculation
 */
class CSC
{
public:
    typedef Map<NodeID, NodeID> IdToIdMap;
    typedef FILOWorkList<NodeID> WorkStack;
    typedef typename IdToIdMap::iterator iterator;

private:
    ConstraintGraph* _consG;
    CGSCC* _scc;

    NodeID _I;
    IdToIdMap _D;       // the sum of weight of a path relevant to a certain node, while accessing the node via DFS
    NodeStack _S;       // a stack holding a DFS branch
    NodeSet _visited;   // a set holding visited nodes
//    NodeStrides nodeStrides;
//    IdToIdMap pwcReps;

public:
    CSC(ConstraintGraph* g, CGSCC* c)
        : _consG(g), _scc(c), _I(0) {}

    void find(NodeStack& candidates);
    void visit(NodeID nodeId, s32_t _w);
    void clear();

    bool isVisited(NodeID nId)
    {
        return _visited.find(nId) != _visited.end();
    }

    void setVisited(NodeID nId)
    {
        _visited.insert(nId);
    }
//    inline iterator begin() { return pwcReps.begin(); }
//
//    inline iterator end() { return pwcReps.end(); }

//    NodeStrides &getNodeStrides() { return nodeStrides; }

//    const NodeSet& getPWCReps() const { return  _pwcReps; }
};

} // End namespace SVF

#endif //PROJECT_CSC_H
