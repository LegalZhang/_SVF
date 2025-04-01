
#ifndef INCLUDE_CFL_CFGNORMALIZER_H_
#define INCLUDE_CFL_CFGNORMALIZER_H_

#include "CFGrammar.h"

namespace SVF
{

/*!
 *  Generate Normalized Grammar (backus naur form) from a grammarbase (Extended extended Backus–Naur form )
 *
 *  To Do:
 *      Error Notice for ill formed production,
 *      e.g. not end with ';' and '*' not preceding with '()' and extra space before ';'
 *      '|' sign support
 */

class CFGNormalizer
{

public:
    CFGNormalizer()
    {
    }

    /// Binary Normal Form(BNF) normalization with variable attribute expanded
    CFGrammar* normalize(GrammarBase *generalGrammar);

    /// Expand every variable attribute in rawProductions of grammarbase
    CFGrammar* fillAttribute(CFGrammar *grammar, const Map<CFGrammar::Kind, Set<CFGrammar::Attribute>>& kindToAttrsMap);

private:
    /// Add nonterminal to tranfer long rules to binary rules
    void ebnf_bin(CFGrammar *grammar);

    void ebnfSignReplace(char sign, CFGrammar *grammar);

    void barReplace(CFGrammar *grammar);

    void insertToCFLGrammar(CFGrammar *grammar, GrammarBase::Production &prod);

    int ebnfBracketMatch(GrammarBase::Production& prod, int i, CFGrammar *grammar) ;

    GrammarBase::Symbol check_head(GrammarBase::SymbolMap<GrammarBase::Symbol, GrammarBase::Productions>& grammar, GrammarBase::Production& rule);

    void strTrans(std::string strPro, CFGrammar *grammar, GrammarBase::Production& normalProd);

    void getFilledProductions(GrammarBase::Production &prod,const NodeSet& nodeSet, CFGrammar *grammar, GrammarBase::Productions& normalProds);

    void removeFirstSymbol(CFGrammar *grammar);
};

} // End namespace SVF

#endif /* INCLUDE_CFL_CFGNORMALIZER_H_*/