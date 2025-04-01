
#ifndef SABERSVFGBUILDER_H_
#define SABERSVFGBUILDER_H_

#include "MSSA/SVFGBuilder.h"
#include "SVFIR/SVFValue.h"
#include "Util/WorkList.h"


namespace SVF
{

class SaberCondAllocator;

class SaberSVFGBuilder : public SVFGBuilder
{

public:
    typedef Set<const SVFGNode*> SVFGNodeSet;
    typedef Map<NodeID, PointsTo> NodeToPTSSMap;
    typedef FIFOWorkList<NodeID> WorkList;

    /// Constructor
    SaberSVFGBuilder(): SVFGBuilder(true) {}

    /// Destructor
    virtual ~SaberSVFGBuilder() {}

    inline bool isGlobalSVFGNode(const SVFGNode* node) const
    {
        return globSVFGNodes.find(node)!=globSVFGNodes.end();
    }

    /// Add ActualParmVFGNode
    inline void addActualParmVFGNode(const PAGNode* pagNode, const CallICFGNode* cs)
    {
        svfg->addActualParmVFGNode(pagNode, cs);
    }

    void setSaberCondAllocator(SaberCondAllocator* allocator)
    {
        saberCondAllocator = allocator;
    }

protected:
    /// Re-write create SVFG method
    virtual void buildSVFG();

    /// Return TRUE if this is a strong update STORE statement.
    bool isStrongUpdate(const SVFGNode* node, NodeID& singleton, BVDataPTAImpl* pta);

protected:
    /// Remove direct value-flow edge to a dereference point for Saber source-sink memory error detection
    /// for example, given two statements: p = alloc; q = *p, the direct SVFG edge between them is deleted
    /// Because those edges only stand for values used at the dereference points but they can not pass the value to other definitions
    void rmDerefDirSVFGEdges(BVDataPTAImpl* pta);

    /// Remove Incoming Edge for strong-update (SU) store instruction
    /// Because the SU node does not receive indirect value
    virtual void rmIncomingEdgeForSUStore(BVDataPTAImpl* pta);

    /// Add actual parameter SVFGNode for 1st argument of a deallocation like external function
    /// In order to path sensitive leak detection
    virtual void AddExtActualParmSVFGNodes(CallGraph* callgraph);

    /// Collect memory pointed global pointers,
    /// note that this collection is recursively performed, for example gp-->obj-->obj'
    /// obj and obj' are both considered global memory
    void collectGlobals(BVDataPTAImpl* pta);

    /// Whether points-to of a PAGNode points-to global variable
    bool accessGlobal(BVDataPTAImpl* pta,const PAGNode* pagNode);

    /// Collect objects along points-to chains
    PointsTo& CollectPtsChain(BVDataPTAImpl* pta,NodeID id, NodeToPTSSMap& cachedPtsMap);

    PointsTo globs;
    /// Store all global SVFG nodes
    SVFGNodeSet globSVFGNodes;

    SaberCondAllocator* saberCondAllocator;
};

} // End namespace SVF

#endif /* SABERSVFGBUILDER_H_ */
