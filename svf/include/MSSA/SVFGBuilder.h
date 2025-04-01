
#ifndef ANDERSENMEMSSA_H_
#define ANDERSENMEMSSA_H_

#include "MemoryModel/PointerAnalysis.h"
#include "Graphs/SVFGOPT.h"

namespace SVF
{

/*!
 * SVFG Builder
 */
class SVFGBuilder
{

public:
    typedef PointerAnalysis::CallSiteSet CallSiteSet;
    typedef PointerAnalysis::CallEdgeMap CallEdgeMap;
    typedef PointerAnalysis::FunctionSet FunctionSet;
    typedef SVFG::SVFGEdgeSetTy SVFGEdgeSet;

    /// Constructor
    explicit SVFGBuilder(bool _SVFGWithIndCall = false): svfg(nullptr), SVFGWithIndCall(_SVFGWithIndCall) {}

    /// Destructor
    virtual ~SVFGBuilder() = default;

    SVFG* buildPTROnlySVFG(BVDataPTAImpl* pta);
    SVFG* buildFullSVFG(BVDataPTAImpl* pta);

    /// Get SVFG instance
    inline SVFG* getSVFG() const
    {
        return svfg.get();
    }

    /// Mark feasible VF edge by removing it from set vfEdgesAtIndCallSite
    inline void markValidVFEdge(SVFGEdgeSet& edges)
    {
        for(SVFGEdgeSet::iterator it = edges.begin(), eit = edges.end(); it!=eit; ++it)
            vfEdgesAtIndCallSite.erase(*it);
    }
    /// Return true if this is an VF Edge pre-connected by Andersen's analysis
    inline bool isSpuriousVFEdgeAtIndCallSite(const SVFGEdge* edge)
    {
        return vfEdgesAtIndCallSite.find(const_cast<SVFGEdge*>(edge))!=vfEdgesAtIndCallSite.end();
    }

    /// Build Memory SSA
    virtual std::unique_ptr<MemSSA> buildMSSA(BVDataPTAImpl* pta, bool ptrOnlyMSSA);

protected:
    /// Create a DDA SVFG. By default actualOut and FormalIN are removed, unless withAOFI is set true.
    SVFG* build(BVDataPTAImpl* pta, VFG::VFGK kind);
    /// Can be rewritten by subclasses
    virtual void buildSVFG();
    /// Release global SVFG
    virtual void releaseMemory();

    /// SVFG Edges connected at indirect call/ret sites
    SVFGEdgeSet vfEdgesAtIndCallSite;
    std::unique_ptr<SVFG> svfg;
    /// SVFG with precomputed indirect call edges
    bool SVFGWithIndCall;
};

} // End namespace SVF

#endif /* ANDERSENMEMSSA_H_ */
