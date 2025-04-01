
#ifndef INCLUDE_CFL_GRAMMARBUILDER_H_
#define INCLUDE_CFL_GRAMMARBUILDER_H_

#include "CFL/CFGrammar.h"

namespace SVF
{

/**
 * Build Grammar from a user specified grammar text
 *
 * Symbol Format:
 *      <kind> [bar] [ _alpha | _number ]
 *      kind: any nonspace string start with alphabet, epsilon stand for empty string
 *      bar: stand for reverse edge
 *      alpha: any single alpha
 *      number: any number
 *      start with capital: nonterminal
 *      start with noncapital: terminal
 *
 * Production Format:
 *      <symbol> -> <symbol> *;
 *      LHS and RHS, Separate by '->', symbol separate by ' ', end by ';'
 *      support '*', '?', '(', ')'
 *
 * Input Format:
 *      Start:
 *      M                   // Specify Start Symbol in Second Line
 *      Terminal:
 *      Addr Copy Store Load Gep Vgep // Specify the order of terminal Addr->0, Copy->1 ..
 *      Productions:        // Each Symbol separate by 'Space', production end with ';'
 *      M -> V d;           // Terminal in NonCapital
 *      M -> dbar V d;      // NonTerminal in Capital
 *      V -> M abar M a M;  // LHS and RHS, Separate by '->'
 *      V -> ( M ? abar ) * M ? ( a M ? ) *;    // Support '(' ')' '?' '*' four regular expression sign
 *      Gep_j -> Gep_i F vgep; // Support variable attribute with variable attribute
 *      Gep_1 -> Gep_2;      // Support fix number attribute
 *
 */

class GrammarBuilder
{
private:
    std::string fileName;
    GrammarBase *grammar;

    /// Parse start symbol and production from file string
    const inline std::string parseProductionsString() const;

    /// Parse whole production string to production vector
    const inline std::vector<std::string> loadWordProductions() const;

    /// Strip front and tail space
    const inline std::string stripSpace(std::string s) const;

public:
    GrammarBuilder(std::string fileName): fileName(fileName), grammar(nullptr)
    {
        grammar = new GrammarBase();
    };

    /// Build grammarBase from fileName
    GrammarBase* build() const;

    /// Build grammarBase from fileName with preset str2KindMap
    GrammarBase* build(Map<std::string, SVF::GrammarBase::Symbol> &preMap) const;
};

} // SVF

#endif /* INCLUDE_CFL_GRAMMARBUILDER_H_ */
