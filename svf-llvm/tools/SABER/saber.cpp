
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "SABER/LeakChecker.h"
#include "SABER/FileChecker.h"
#include "SABER/DoubleFreeChecker.h"
#include "Util/CommandLine.h"
#include "Util/Options.h"
#include "Util/Z3Expr.h"


using namespace llvm;
using namespace SVF;

int main(int argc, char ** argv)
{

    std::vector<std::string> moduleNameVec;
    moduleNameVec = OptionBase::parseOptions(
                        argc, argv, "Source-Sink Bug Detector", "[options] <input-bitcode...>"
                    );

    if (Options::WriteAnder() == "ir_annotator")
    {
        LLVMModuleSet::preProcessBCs(moduleNameVec);
    }

    LLVMModuleSet::buildSVFModule(moduleNameVec);
    SVFIRBuilder builder;
    SVFIR* pag = builder.build();


    std::unique_ptr<LeakChecker> saber;

    if(Options::MemoryLeakCheck())
        saber = std::make_unique<LeakChecker>();
    else if(Options::FileCheck())
        saber = std::make_unique<FileChecker>();
    else if(Options::DFreeCheck())
        saber = std::make_unique<DoubleFreeChecker>();
    else
        saber = std::make_unique<LeakChecker>();  // if no checker is specified, we use leak checker as the default one.

    saber->runOnModule(pag);
    LLVMModuleSet::releaseLLVMModuleSet();


    return 0;

}
