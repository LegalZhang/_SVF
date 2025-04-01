
#include "CFL/CFLStat.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;

/*!
 * Constructor
 */
CFLStat::CFLStat(CFLBase* p): PTAStat(p),pta(p)
{
    startClk();
}

/*!
 * Collect CFLGraph information
 */
void  CFLStat::CFLGraphStat()
{
    pta->countSumEdges();
    CFLGraph* CFLGraph = pta->getCFLGraph();

    timeStatMap["BuildingTime"] = pta->timeOfBuildCFLGraph;
    PTNumStatMap["NumOfNodes"] = CFLGraph->getTotalNodeNum();
    PTNumStatMap["NumOfEdges"] = CFLGraph->getCFLEdges().size();

    PTAStat::printStat("CFLGraph Stats");
}

void CFLStat::CFLGrammarStat()
{
    timeStatMap["BuildingTime"] = pta->timeOfBuildCFLGrammar;
    timeStatMap["NormalizationTime"] = pta->timeOfNormalizeGrammar;

    PTAStat::printStat("CFLGrammar Stats");
}

void CFLStat::CFLSolverStat()
{
    timeStatMap["AnalysisTime"] = pta->timeOfSolving;
    PTNumStatMap["numOfChecks"] = pta->numOfChecks;
    PTNumStatMap["numOfIteration"] = pta->numOfIteration;
    PTNumStatMap["SumEdges"] = pta->numOfStartEdges;

    PTAStat::printStat("CFL-reachability Solver Stats");
}

/*!
 * Start here
 */
void CFLStat::performStat()
{
    assert((SVFUtil::isa<CFLAlias, CFLVF>(pta)) && "not an CFLAlias pass!! what else??");
    endClk();

    // Grammar stat
    CFLGrammarStat();

    // CFLGraph stat
    CFLGraphStat();

    // Solver stat
    CFLSolverStat();

    // Stat about Call graph and General stat
    PTAStat::performStat();
}

