
#include "SABER/DoubleFreeChecker.h"
#include "Util/SVFUtil.h"
#include "Util/Options.h"

using namespace SVF;
using namespace SVFUtil;

void DoubleFreeChecker::reportBug(ProgSlice* slice)
{

    if(slice->isSatisfiableForPairs() == false)
    {
        GenericBug::EventStack eventStack;
        slice->evalFinalCond2Event(eventStack);
        eventStack.push_back(
            SVFBugEvent(SVFBugEvent::SourceInst, getSrcCSID(slice->getSource())));
        report.addSaberBug(GenericBug::DOUBLEFREE, eventStack);
    }
    if(Options::ValidateTests())
        testsValidation(slice);
}



void DoubleFreeChecker::testsValidation(ProgSlice *slice)
{
    const SVFGNode* source = slice->getSource();
    const CallICFGNode* cs = getSrcCSID(source);
    const FunObjVar* fun = cs->getCalledFunction();
    if(fun==nullptr)
        return;
    validateSuccessTests(slice,fun);
    validateExpectedFailureTests(slice,fun);
}

void DoubleFreeChecker::validateSuccessTests(ProgSlice *slice, const FunObjVar *fun)
{
    const SVFGNode* source = slice->getSource();
    const CallICFGNode* cs = getSrcCSID(source);

    bool success = false;

    if(fun->getName() == "SAFEMALLOC")
    {
        if(slice->isSatisfiableForPairs() == true)
            success = true;
    }
    else if(fun->getName() == "DOUBLEFREEMALLOC")
    {
        if(slice->isSatisfiableForPairs() == false)
            success = true;
    }
    else if(fun->getName() == "DOUBLEFREEMALLOCFN" || fun->getName() == "SAFEMALLOCFP")
    {
        return;
    }
    else
    {
        writeWrnMsg("\t can not validate, check function not found, please put it at the right place!!");
        return;
    }

    std::string funName = source->getFun()->getName();

    if (success)
    {
        outs() << sucMsg("\t SUCCESS :") << funName << " check <src id:" << source->getId()
               << ", cs id:" << (getSrcCSID(source))->valueOnlyToString() << "> at ("
               << cs->getSourceLoc() << ")\n";
        outs() << "\t\t double free path: \n" << slice->evalFinalCond() << "\n";
    }
    else
    {
        SVFUtil::errs() << errMsg("\t FAILURE :") << funName << " check <src id:" << source->getId()
                        << ", cs id:" << (getSrcCSID(source))->valueOnlyToString() << "> at ("
                        << cs->getSourceLoc() << ")\n";
        SVFUtil::errs() << "\t\t double free path: \n" << slice->evalFinalCond() << "\n";
        assert(false && "test case failed!");
    }
}

void DoubleFreeChecker::validateExpectedFailureTests(ProgSlice *slice, const FunObjVar *fun)
{
    const SVFGNode* source = slice->getSource();
    const CallICFGNode* cs = getSrcCSID(source);

    bool expectedFailure = false;
    /// output safe but should be double free
    if(fun->getName() == "DOUBLEFREEMALLOCFN")
    {
        if(slice->isSatisfiableForPairs() == true)
            expectedFailure = true;
    } /// output double free but should be safe
    else if(fun->getName() == "SAFEMALLOCFP")
    {
        if(slice->isSatisfiableForPairs() == false)
            expectedFailure = true;
    }
    else if(fun->getName() == "SAFEMALLOC" || fun->getName() == "DOUBLEFREEMALLOC")
    {
        return;
    }
    else
    {
        writeWrnMsg("\t can not validate, check function not found, please put it at the right place!!");
        return;
    }

    std::string funName = source->getFun()->getName();

    if (expectedFailure)
    {
        outs() << sucMsg("\t EXPECTED-FAILURE :") << funName << " check <src id:" << source->getId()
               << ", cs id:" << (getSrcCSID(source))->valueOnlyToString() << "> at ("
               << cs->getSourceLoc() << ")\n";
        outs() << "\t\t double free path: \n" << slice->evalFinalCond() << "\n";
    }
    else
    {
        SVFUtil::errs() << errMsg("\t UNEXPECTED FAILURE :") << funName
                        << " check <src id:" << source->getId()
                        << ", cs id:" << (getSrcCSID(source))->valueOnlyToString() << "> at ("
                        << cs->getSourceLoc() << ")\n";
        SVFUtil::errs() << "\t\t double free path: \n" << slice->evalFinalCond() << "\n";
        assert(false && "test case failed!");
    }
}
