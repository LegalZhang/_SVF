
#include "Util/SVFUtil.h"
#include "Util/CallGraphBuilder.h"
#include "Graphs/ICFG.h"
#include "SVFIR/SVFIR.h"
#include "Graphs/CallGraph.h"
#include "Graphs/ThreadCallGraph.h"

using namespace SVF;
using namespace SVFUtil;

CallGraph* CallGraphBuilder::buildSVFIRCallGraph(const std::vector<const FunObjVar*>& funset)
{
    CallGraph* callgraph = new CallGraph();
    for (const FunObjVar* svfFunc: funset)
    {
        callgraph->addCallGraphNode(svfFunc);
    }

    for (const auto& item : *callgraph)
    {
        for (auto it : *(item.second)->getFunction())
        {
            const SVFBasicBlock* svfbb = it.second;
            for (const ICFGNode* inst : svfbb->getICFGNodeList())
            {
                if (SVFUtil::isNonInstricCallSite(inst))
                {
                    const CallICFGNode* callBlockNode = cast<CallICFGNode>(inst);
                    if(const FunObjVar* callee = callBlockNode->getCalledFunction())
                    {
                        callgraph->addDirectCallGraphEdge(callBlockNode,(item.second)->getFunction(),callee);
                    }
                }
            }
        }
    }
    return callgraph;
}

CallGraph* CallGraphBuilder::buildPTACallGraph()
{
    CallGraph* svfirCallGraph = PAG::getPAG()->getCallGraph();
    return new CallGraph(*svfirCallGraph);
}

ThreadCallGraph* CallGraphBuilder::buildThreadCallGraph()
{
    CallGraph* svfirCallGraph = PAG::getPAG()->getCallGraph();
    ThreadCallGraph* cg = new ThreadCallGraph(*svfirCallGraph);

    ThreadAPI* tdAPI = ThreadAPI::getThreadAPI();
    for (const auto& item: *svfirCallGraph)
    {
        for (auto it : *(item.second)->getFunction())
        {
            const SVFBasicBlock* svfbb = it.second;
            for (const ICFGNode* inst : svfbb->getICFGNodeList())
            {
                if (SVFUtil::isa<CallICFGNode>(inst) && tdAPI->isTDFork(SVFUtil::cast<CallICFGNode>(inst)))
                {
                    const CallICFGNode* cs = cast<CallICFGNode>(inst);
                    cg->addForksite(cs);
                    const ValVar* svfVar = tdAPI->getForkedFun(cs);
                    if (SVFUtil::isa<FunValVar>(svfVar))
                    {
                        cg->addDirectForkEdge(cs);
                    }
                    // indirect call to the start routine function
                    else
                    {
                        cg->addThreadForkEdgeSetMap(cs, nullptr);
                    }
                }
            }
        }
    }
    // record join sites
    for (const auto& item: *svfirCallGraph)
    {
        for (auto it : *(item.second)->getFunction())
        {
            const SVFBasicBlock* svfbb = it.second;
            for (const ICFGNode* node : svfbb->getICFGNodeList())
            {
                if (SVFUtil::isa<CallICFGNode>(node) && tdAPI->isTDJoin(SVFUtil::cast<CallICFGNode>(node)))
                {
                    const CallICFGNode* cs = SVFUtil::cast<CallICFGNode>(node);
                    cg->addJoinsite(cs);
                }
            }
        }
    }

    return cg;
}
