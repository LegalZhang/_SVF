
#include "CFL/CFLAlias.h"
using namespace SVF;
using namespace SVFUtil;

/*!
 * On the fly call graph construction
 * callsites is candidate indirect callsites need to be analyzed based on points-to results
 * newEdges is the new indirect call edges discovered
 */
void CFLAlias::onTheFlyCallGraphSolve(const CallSiteToFunPtrMap& callsites, CallEdgeMap& newEdges)
{
    for(CallSiteToFunPtrMap::const_iterator iter = callsites.begin(), eiter = callsites.end(); iter!=eiter; ++iter)
    {
        const CallICFGNode* cs = iter->first;

        if (cs->isVirtualCall())
        {
            const SVFVar* vtbl = cs->getVtablePtr();

            assert(vtbl != nullptr);
            NodeID vtblId = vtbl->getId();
            resolveCPPIndCalls(cs, getCFLPts(vtblId), newEdges);
        }
        else
            resolveIndCalls(iter->first,getCFLPts(iter->second),newEdges);
    }
}

/*!
 * Connect formal and actual parameters for indirect callsites
 */

void CFLAlias::connectCaller2CalleeParams(const CallICFGNode* cs, const FunObjVar* F)
{
    assert(F);

    DBOUT(DAndersen, outs() << "connect parameters from indirect callsite " << cs->toString() << " to callee " << *F << "\n");

    const CallICFGNode* callBlockNode = cs;
    const RetICFGNode* retBlockNode = cs->getRetICFGNode();

    if(SVFUtil::isHeapAllocExtFunViaRet(F) && svfir->callsiteHasRet(retBlockNode))
    {
        heapAllocatorViaIndCall(cs);
    }

    if (svfir->funHasRet(F) && svfir->callsiteHasRet(retBlockNode))
    {
        const PAGNode* cs_return = svfir->getCallSiteRet(retBlockNode);
        const PAGNode* fun_return = svfir->getFunRet(F);
        if (cs_return->isPointer() && fun_return->isPointer())
        {
            NodeID dstrec = cs_return->getId();
            NodeID srcret = fun_return->getId();
            addCopyEdge(srcret, dstrec);
        }
        else
        {
            DBOUT(DAndersen, outs() << "not a pointer ignored\n");
        }
    }

    if (svfir->hasCallSiteArgsMap(callBlockNode) && svfir->hasFunArgsList(F))
    {

        // connect actual and formal param
        const SVFIR::SVFVarList& csArgList = svfir->getCallSiteArgsList(callBlockNode);
        const SVFIR::SVFVarList& funArgList = svfir->getFunArgsList(F);
        //Go through the fixed parameters.
        DBOUT(DPAGBuild, outs() << "      args:");
        SVFIR::SVFVarList::const_iterator funArgIt = funArgList.begin(), funArgEit = funArgList.end();
        SVFIR::SVFVarList::const_iterator csArgIt  = csArgList.begin(), csArgEit = csArgList.end();
        for (; funArgIt != funArgEit; ++csArgIt, ++funArgIt)
        {
            //Some programs (e.g. Linux kernel) leave unneeded parameters empty.
            if (csArgIt  == csArgEit)
            {
                DBOUT(DAndersen, outs() << " !! not enough args\n");
                break;
            }
            const PAGNode *cs_arg = *csArgIt ;
            const PAGNode *fun_arg = *funArgIt;

            if (cs_arg->isPointer() && fun_arg->isPointer())
            {
                DBOUT(DAndersen, outs() << "process actual parm  " << cs_arg->toString() << " \n");
                NodeID srcAA = cs_arg->getId();
                NodeID dstFA = fun_arg->getId();
                addCopyEdge(srcAA, dstFA);
            }
        }

        //Any remaining actual args must be varargs.
        if (F->isVarArg())
        {
            NodeID vaF = svfir->getVarargNode(F);
            DBOUT(DPAGBuild, outs() << "\n      varargs:");
            for (; csArgIt != csArgEit; ++csArgIt)
            {
                const PAGNode *cs_arg = *csArgIt;
                if (cs_arg->isPointer())
                {
                    NodeID vnAA = cs_arg->getId();
                    addCopyEdge(vnAA,vaF);
                }
            }
        }
        if(csArgIt != csArgEit)
        {
            writeWrnMsg("too many args to non-vararg func.");
            writeWrnMsg("(" + cs->getSourceLoc() + ")");
        }
    }
}

void CFLAlias::heapAllocatorViaIndCall(const CallICFGNode* cs)
{
    assert(cs->getCalledFunction() == nullptr && "not an indirect callsite?");
    const RetICFGNode* retBlockNode = cs->getRetICFGNode();
    const PAGNode* cs_return = svfir->getCallSiteRet(retBlockNode);
    NodeID srcret;
    CallSite2DummyValPN::const_iterator it = callsite2DummyValPN.find(cs);
    if(it != callsite2DummyValPN.end())
    {
        srcret = it->second;
    }
    else
    {
        NodeID valNode = svfir->addDummyValNode();
        NodeID objNode = svfir->addDummyObjNode(cs->getType());
        callsite2DummyValPN.insert(std::make_pair(cs,valNode));
        graph->addCFLNode(valNode, new CFLNode(valNode));
        graph->addCFLNode(objNode, new CFLNode(objNode));
        srcret = valNode;
    }

    NodeID dstrec = cs_return->getId();
    addCopyEdge(srcret, dstrec);
}

/*!
 * Update call graph for the input indirect callsites
 */
bool CFLAlias::updateCallGraph(const CallSiteToFunPtrMap& callsites)
{
    CallEdgeMap newEdges;
    onTheFlyCallGraphSolve(callsites,newEdges);
    for(CallEdgeMap::iterator it = newEdges.begin(), eit = newEdges.end(); it!=eit; ++it )
    {
        for(FunctionSet::iterator cit = it->second.begin(), ecit = it->second.end(); cit!=ecit; ++cit)
        {
            connectCaller2CalleeParams(it->first,*cit);
        }
    }

    return (!solver->isWorklistEmpty());
}

void CFLAlias::initialize()
{
    stat = new CFLStat(this);

    // Parameter Checking
    checkParameter();

    // Build CFL Grammar
    buildCFLGrammar();

    // Build CFL Graph
    buildCFLGraph();

    // Normalize CFL Grammar
    normalizeCFLGrammar();

    // Initialize solver
    initializeSolver();
}

void CFLAlias::initializeSolver()
{
    solver = new CFLSolver(graph, grammar);
}

void CFLAlias::finalize()
{
    numOfChecks = solver->numOfChecks;

    if(Options::PrintCFL() == true)
    {
        if (Options::CFLGraph().empty())
            svfir->dump("IR");
        grammar->dump("Grammar");
        graph->dump("CFLGraph");
    }
    if (Options::CFLGraph().empty())
        PointerAnalysis::finalize();
}

void CFLAlias::solve()
{
    // Start solving
    double start = stat->getClk(true);

    solver->solve();
    if (Options::CFLGraph().empty())
    {
        while (updateCallGraph(svfir->getIndirectCallsites()))
        {
            numOfIteration++;
            solver->solve();
        }
    } // Only cflgraph built from bc could reanalyze by update call graph

    double end = stat->getClk(true);
    timeOfSolving += (end - start) / TIMEINTERVAL;
}

void POCRAlias::initializeSolver()
{
    solver = new POCRSolver(graph, grammar);
}

void POCRHybrid::initializeSolver()
{
    solver = new POCRHybridSolver(graph, grammar);
}
