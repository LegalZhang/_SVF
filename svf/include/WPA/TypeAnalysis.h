
#ifndef INCLUDE_WPA_TYPEANALYSIS_H_
#define INCLUDE_WPA_TYPEANALYSIS_H_

#include "WPA/Andersen.h"

namespace SVF
{

class TypeAnalysis:  public AndersenBase
{

public:
    /// Constructor
    TypeAnalysis(SVFIR* pag)
        :  AndersenBase(pag, TypeCPP_WPA)
    {
    }

    /// Destructor
    virtual ~TypeAnalysis()
    {
    }

    /// Type analysis
    void analyze() override;

    /// Initialize analysis
    void initialize() override;

    /// Finalize analysis
    virtual inline void finalize() override;

    /// Add copy edge on constraint graph
    inline bool addCopyEdge(NodeID src, NodeID dst) override
    {
        assert(false && "this function should never be executed!");
        return false;
    }

    /// Resolve callgraph based on CHA
    void callGraphSolveBasedOnCHA(const CallSiteToFunPtrMap& callsites, CallEdgeMap& newEdges);

    /// Statistics of CHA and callgraph
    void dumpCHAStats();

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const TypeAnalysis *)
    {
        return true;
    }
    static inline bool classof(const PointerAnalysis *pta)
    {
        return (pta->getAnalysisTy() == TypeCPP_WPA);
    }
    //@}
};

} // End namespace SVF

#endif /* INCLUDE_WPA_TYPEANALYSIS_H_ */
