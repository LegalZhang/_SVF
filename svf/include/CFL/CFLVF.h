
#ifndef INCLUDE_CFL_CFLVF_H_
#define INCLUDE_CFL_CFLVF_H_


#include "CFL/CFLBase.h"
#include "CFL/CFLStat.h"
#include "CFL/CFLSVFGBuilder.h"
#include "WPA/Andersen.h"

namespace SVF
{
class CFLVF : public CFLBase
{

public:
    CFLVF(SVFIR* ir) : CFLBase(ir, PointerAnalysis::CFLFSCS_WPA)
    {
    }

    /// Parameter Checking
    virtual void checkParameter();

    /// Initialize the grammar, graph, solver
    virtual void initialize();

    /// Print grammar and graph
    virtual void finalize();

    /// Build CFLGraph via VFG
    void buildCFLGraph();

private:
    CFLSVFGBuilder memSSA;
    SVFG* svfg;
};

} // End namespace SVF

#endif /* INCLUDE_CFL_CFLVF_H_*/
