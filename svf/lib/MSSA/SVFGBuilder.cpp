
#include "Util/Options.h"
#include "Util/SVFUtil.h"
#include "MSSA/MemSSA.h"
#include "Graphs/SVFG.h"
#include "MSSA/SVFGBuilder.h"
#include "WPA/Andersen.h"
#include "Graphs/CallGraph.h"


using namespace SVF;
using namespace SVFUtil;


SVFG* SVFGBuilder::buildPTROnlySVFG(BVDataPTAImpl* pta)
{
    if(Options::OPTSVFG())
        return build(pta, VFG::PTRONLYSVFG_OPT);
    else
        return build(pta, VFG::PTRONLYSVFG);
}

SVFG* SVFGBuilder::buildFullSVFG(BVDataPTAImpl* pta)
{
    return build(pta, VFG::FULLSVFG);
}


/*!
 * Create SVFG
 */
void SVFGBuilder::buildSVFG()
{
    svfg->buildSVFG();
}

/// Create DDA SVFG
SVFG* SVFGBuilder::build(BVDataPTAImpl* pta, VFG::VFGK kind)
{

    auto mssa = buildMSSA(pta, (VFG::PTRONLYSVFG==kind || VFG::PTRONLYSVFG_OPT==kind));

    DBOUT(DGENERAL, outs() << pasMsg("Build Sparse Value-Flow Graph \n"));
    if(kind == VFG::FULLSVFG_OPT || kind == VFG::PTRONLYSVFG_OPT)
        svfg = std::make_unique<SVFGOPT>(std::move(mssa), kind);
    else
        svfg = std::unique_ptr<SVFG>(new SVFG(std::move(mssa),kind));
    buildSVFG();

    /// Update call graph using pre-analysis results
    if(Options::SVFGWithIndirectCall() || SVFGWithIndCall)
        svfg->updateCallGraph(pta);

    if(svfg->getMSSA()->getPTA()->printStat())
        svfg->performStat();

    if(Options::DumpVFG())
        svfg->dump("svfg_final");

    return svfg.get();
}

/*!
 * Release memory
 */
void SVFGBuilder::releaseMemory()
{
    svfg->clearMSSA();
}

std::unique_ptr<MemSSA> SVFGBuilder::buildMSSA(BVDataPTAImpl* pta, bool ptrOnlyMSSA)
{

    DBOUT(DGENERAL, outs() << pasMsg("Build Memory SSA \n"));

    auto mssa = std::make_unique<MemSSA>(pta, ptrOnlyMSSA);

    CallGraph* svfirCallGraph = PAG::getPAG()->getCallGraph();
    for (const auto& item: *svfirCallGraph)
    {

        const FunObjVar *fun = item.second->getFunction();
        if (isExtCall(fun))
            continue;

        mssa->buildMemSSA(*fun);
    }

    mssa->performStat();
    if (Options::DumpMSSA())
    {
        mssa->dumpMSSA();
    }

    return mssa;
}


