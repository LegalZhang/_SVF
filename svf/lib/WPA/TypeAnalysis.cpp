

#include "WPA/TypeAnalysis.h"
#include "Graphs/CHG.h"
#include "Graphs/ICFGStat.h"
#include "Graphs/VFG.h"
#include "Util/Options.h"
#include "Util/PTAStat.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;


/// Initialize analysis
void TypeAnalysis::initialize()
{
    AndersenBase::initialize();
    if (Options::DumpICFG())
    {
        icfg = SVFIR::getPAG()->getICFG();
        icfg->dump("icfg_initial");
        icfg->dump("vfg_initial");
        if (print_stat)
        {
            ICFGStat stat(icfg);
            stat.performStat();
        }
    }
}

/// Finalize analysis
void TypeAnalysis::finalize()
{
    AndersenBase::finalize();
    if (print_stat)
        dumpCHAStats();
}

void TypeAnalysis::analyze()
{
    initialize();
    CallEdgeMap newEdges;
    callGraphSolveBasedOnCHA(getIndirectCallsites(), newEdges);
    finalize();
}

void TypeAnalysis::callGraphSolveBasedOnCHA(const CallSiteToFunPtrMap& callsites, CallEdgeMap& newEdges)
{
    for(CallSiteToFunPtrMap::const_iterator iter = callsites.begin(), eiter = callsites.end(); iter!=eiter; ++iter)
    {
        const CallICFGNode* cbn = iter->first;
        if (cbn->isVirtualCall())
        {
            const SVFVar* vtbl = cbn->getVtablePtr();
            (void)vtbl; // Suppress warning of unused variable under release build
            assert(vtbl != nullptr);
            VFunSet vfns;
            getVFnsFromCHA(cbn, vfns);
            connectVCallToVFns(cbn, vfns, newEdges);
        }
    }
}


void TypeAnalysis::dumpCHAStats()
{

    const CHGraph *chgraph = SVFUtil::dyn_cast<CHGraph>(getCHGraph());
    if (chgraph == nullptr)
    {
        SVFUtil::errs() << "dumpCHAStats only implemented for standard CHGraph.\n";
        return;
    }

    u32_t pure_abstract_class_num = 0,
          multi_inheritance_class_num = 0;
    for (CHGraph::const_iterator it = chgraph->begin(), eit = chgraph->end();
            it != eit; ++it)
    {
        CHNode *node = it->second;
        outs() << "class " << node->getName() << "\n";
        if (node->isPureAbstract())
            pure_abstract_class_num++;
        if (node->isMultiInheritance())
            multi_inheritance_class_num++;
    }
    outs() << "class_num:\t" << chgraph->getTotalNodeNum() << '\n';
    outs() << "pure_abstract_class_num:\t" << pure_abstract_class_num << '\n';
    outs() << "multi_inheritance_class_num:\t" << multi_inheritance_class_num << '\n';

    /*
     * count the following info:
     * vtblnum
     * total vfunction
     * vtbl max vfunction
     * pure abstract class
     */
    u32_t vtblnum = 0,
          vfunc_total = 0,
          vtbl_max = 0,
          pure_abstract = 0;
    set<const FunObjVar*> allVirtualFunctions;
    for (CHGraph::const_iterator it = chgraph->begin(), eit = chgraph->end();
            it != eit; ++it)
    {
        CHNode *node = it->second;
        if (node->isPureAbstract())
            pure_abstract++;

        u32_t vfuncs_size = 0;
        const vector<CHNode::FuncVector>& vecs = node->getVirtualFunctionVectors();
        for (vector<CHNode::FuncVector>::const_iterator vit = vecs.begin(),
                veit = vecs.end(); vit != veit; ++vit)
        {
            vfuncs_size += (*vit).size();
            for (vector<const FunObjVar*>::const_iterator fit = (*vit).begin(),
                    feit = (*vit).end(); fit != feit; ++fit)
            {
                const FunObjVar* func = *fit;
                allVirtualFunctions.insert(func);
            }
        }
        if (vfuncs_size > 0)
        {
            vtblnum++;
            if (vfuncs_size > vtbl_max)
            {
                vtbl_max = vfuncs_size;
            }
        }
    }
    vfunc_total = allVirtualFunctions.size();

    outs() << "vtblnum:\t" << vtblnum << '\n';
    outs() << "vtbl_average:\t" << (double)(vfunc_total)/vtblnum << '\n';
    outs() << "vtbl_max:\t" << vtbl_max << '\n';
    outs() << "pure_abstract:\t" << pure_abstract << '\n';
}


