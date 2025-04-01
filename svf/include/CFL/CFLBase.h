
#ifndef INCLUDE_CFL_CFLBASE_H_
#define INCLUDE_CFL_CFLBASE_H_

#include "CFL/CFLSolver.h"
#include "CFL/CFGNormalizer.h"
#include "CFL/GrammarBuilder.h"
#include "CFL/CFLGraphBuilder.h"
#include "CFL/CFLGramGraphChecker.h"
#include "MemoryModel/PointerAnalysis.h"
#include "Graphs/ConsG.h"
#include "Util/Options.h"
#include "SVFIR/SVFType.h"

namespace SVF
{

class CFLStat;

/// CFL Client Base Class
class CFLBase : public BVDataPTAImpl
{

public:
    CFLBase(SVFIR* ir, PointerAnalysis::PTATY pty) : BVDataPTAImpl(ir, pty), svfir(ir), graph(nullptr), grammar(nullptr), solver(nullptr)
    {
    }

    /// Destructor
    virtual ~CFLBase()
    {
        delete solver;
        delete grammarBase;
    }

    /// Parameter Checking
    virtual void checkParameter();

    /// Build Grammar from text file
    virtual void buildCFLGrammar();

    /// Build CFLGraph based on Option
    virtual void buildCFLGraph();

    /// Normalize grammar
    virtual void normalizeCFLGrammar();

    /// Get CFL graph
    CFLGraph* getCFLGraph();

    /// Count the num of Nonterminal Edges
    virtual void countSumEdges();

    /// Solving CFL Reachability
    virtual void solve();

    /// Finalize extra stat info passing
    virtual void finalize();

    /// Perform analyze (main part of CFLR Analysis)
    virtual void analyze();

    /// Statistics
    //@{
    // Grammar
    static double timeOfBuildCFLGrammar;            // Time of building grammarBase from text file
    static double timeOfNormalizeGrammar;           // Time of normalizing grammarBase to CFGrammar
    // Graph
    static double timeOfBuildCFLGraph;              // Time of building CFLGraph
    static double numOfTerminalEdges;               // Number of terminal labeled edges
    static double numOfTemporaryNonterminalEdges;   // Number of temporary (ie. X0, X1..) nonterminal labeled edges
    static double numOfNonterminalEdges;            // Number of nonterminal labeled edges
    static double numOfStartEdges;                  // Number of start nonterminal labeled edges
    // Solver
    static double numOfIteration;                   // Number solving Iteration
    static double numOfChecks;                  // Number of checks
    static double timeOfSolving;                    // time of solving CFL Reachability
    //@}

protected:
    SVFIR* svfir;
    CFLGraph* graph;
    GrammarBase* grammarBase;
    CFGrammar* grammar;
    CFLSolver* solver;
};

} // End namespace SVF

#endif /* INCLUDE_CFL_CFLBASE_H_*/
