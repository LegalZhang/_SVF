
#ifndef INCLUDE_CFL_CFLGRAMGRAPHCHECKER_H_
#define INCLUDE_CFL_CFLGRAMGRAPHCHECKER_H_

#include "CFL/CFGrammar.h"
#include "Graphs/CFLGraph.h"
namespace SVF
{

class CFLGramGraphChecker
{
public:
    void check(GrammarBase *grammar, CFLGraphBuilder *graphBuilder, CFLGraph *graph)
    {
        /// Check all kinds in grammar in graphBuilder with the same label
        for(auto pairV : grammar->getTerminals())
        {
            if (graphBuilder->getLabelToKindMap().find(pairV.first) != graphBuilder->getLabelToKindMap().end())
            {
                assert(graphBuilder->getLabelToKindMap()[pairV.first] == pairV.second);
                assert(graphBuilder->getKindToLabelMap()[pairV.second] == pairV.first);
            }
        }

        for(auto pairV : grammar->getNonterminals())
        {
            if (graphBuilder->getLabelToKindMap().find(pairV.first) != graphBuilder->getLabelToKindMap().end())
            {
                assert(graphBuilder->getLabelToKindMap()[pairV.first] == pairV.second);
                assert(graphBuilder->getKindToLabelMap()[pairV.second] == pairV.first);
            }
            else
            {
                graphBuilder->getLabelToKindMap().insert(std::make_pair (pairV.first,pairV.second));
                graphBuilder->getKindToLabelMap().insert(std::make_pair (pairV.second, pairV.first));
            }
        }

        /// Get KindToAttrs Map from Graph to Grammar
        grammar->setKindToAttrsMap(graphBuilder->getKindToAttrsMap());
        graph->startKind = grammar->getStartKind();
    }
};

}// SVF

#endif /* INCLUDE_CFL_CFLGRAMGRAPHCHECKER_H_*/