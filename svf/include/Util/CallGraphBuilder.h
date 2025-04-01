
#ifndef INCLUDE_SVF_FE_CALLGRAPHBUILDER_H_
#define INCLUDE_SVF_FE_CALLGRAPHBUILDER_H_

#include "Graphs/ThreadCallGraph.h"

namespace SVF
{

class ICFG;
class CallGraph;
class ThreadCallGraph;

class CallGraphBuilder
{
public:
    CallGraphBuilder()=default;

    /// Buidl SVFIR callgraoh
    CallGraph* buildSVFIRCallGraph(const std::vector<const FunObjVar*>& funset);

    /// Buidl PTA callgraoh
    CallGraph* buildPTACallGraph();

    /// Build thread-aware callgraph
    ThreadCallGraph* buildThreadCallGraph();
};

} // End namespace SVF


#endif /* INCLUDE_UTIL_CALLGRAPHBUILDER_H_ */
