
#ifndef INCLUDE_GRAPHS_GRAPHPRINTER_H_
#define INCLUDE_GRAPHS_GRAPHPRINTER_H_

#include <system_error>
#include "Graphs/GraphWriter.h"	// for graph write
#include <fstream>

namespace SVF
{

/*
 * Dump and print the graph for debugging
 */
class GraphPrinter
{

public:
    GraphPrinter()
    {
    }

    /*!
     *  Write the graph into dot file for debugging purpose
     */
    template<class GraphType>
    static void WriteGraphToFile(SVF::OutStream &O,
                                 const std::string &GraphName, const GraphType &GT, bool simple = false)
    {
        // Filename of the output dot file
        std::string Filename = GraphName + ".dot";
        std::ofstream outFile(Filename);
        if (outFile.fail())
        {
            O << "  error opening file for writing!\n";
            outFile.close();
            return;
        }

        O << "Writing '" << Filename << "'...";

        WriteGraph(outFile, GT, simple);
        outFile.close();
    }

    /*!
     * Print the graph to command line
     */
    template<class GraphType>
    static void PrintGraph(SVF::OutStream &O, const std::string &GraphName,
                           const GraphType &GT)
    {
        ///Define the GTraits and node iterator for printing
        typedef GenericGraphTraits<GraphType> GTraits;

        typedef typename GTraits::NodeRef NodeRef;
        typedef typename GTraits::nodes_iterator node_iterator;
        typedef typename GTraits::ChildIteratorType child_iterator;

        O << "Printing VFG Graph" << "'...\n";
        // Print each node name and its edges
        node_iterator I = GTraits::nodes_begin(GT);
        node_iterator E = GTraits::nodes_end(GT);
        for (; I != E; ++I)
        {
            NodeRef *Node = *I;
            O << "node :" << Node << "'\n";
            child_iterator EI = GTraits::child_begin(Node);
            child_iterator EE = GTraits::child_end(Node);
            for (unsigned i = 0; EI != EE && i != 64; ++EI, ++i)
            {
                O << "child :" << *EI << "'\n";
            }
        }
    }
};

} // End namespace llvm



#endif /* INCLUDE_GRAPHS_GRAPHPRINTER_H_ */
