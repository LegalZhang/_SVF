
#include "CFL/CFLVF.h"

using namespace SVF;
using namespace SVFUtil;

void CFLVF::buildCFLGraph()
{
    // Build CFL Graph
    VFCFLGraphBuilder cflGraphBuilder = VFCFLGraphBuilder();
    if (Options::CFLGraph().empty()) // built from svfir
    {
        PointerAnalysis::initialize();
        AndersenWaveDiff* ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
        svfg =  memSSA.buildFullSVFG(ander);
        graph = cflGraphBuilder.buildBigraph(svfg, grammarBase->getStartKind(), grammarBase);
    }
    else
        graph = cflGraphBuilder.build(Options::CFLGraph(), grammarBase);

    // Check CFL Graph and Grammar are accordance with grammar
    CFLGramGraphChecker cflChecker = CFLGramGraphChecker();
    cflChecker.check(grammarBase, &cflGraphBuilder, graph);
}

void CFLVF::initialize()
{
    // Parameter Checking
    checkParameter();

    // Build CFL Grammar
    buildCFLGrammar();

    // Build CFL Graph
    buildCFLGraph();

    // Normalize grammar
    normalizeCFLGrammar();

    // Initialize solver
    solver = new CFLSolver(graph, grammar);
}

void CFLVF::checkParameter()
{
    // Check for valid grammar file before parsing other options
    std::string filename = Options::GrammarFilename();
    bool vfgfile = (filename.rfind("VFGGrammar.txt") == filename.length() - std::string("VFGGrammar.txt").length());
    if (!Options::Customized()  && !vfgfile)
    {
        SVFUtil::errs() << "Invalid VFG grammar file: " << Options::GrammarFilename() << "\n"
                        << "Please use a file that ends with 'VFG.txt', "
                        << "or use the -customized flag to allow custom grammar files.\n";
        assert(false && "grammar loading failed!");  // exit with error
    }
}


void CFLVF::finalize()
{
    if(Options::PrintCFL())
    {
        if (Options::CFLGraph().empty())
            svfir->dump("IR");
        grammar->dump("Grammar");
        graph->dump("CFLGraph");
    }
}
