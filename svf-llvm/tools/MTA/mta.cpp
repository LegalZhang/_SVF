
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "MTA/MTA.h"
#include "Util/CommandLine.h"
#include "Util/Options.h"

using namespace llvm;
using namespace std;
using namespace SVF;

int main(int argc, char ** argv)
{

    std::vector<std::string> moduleNameVec;
    moduleNameVec = OptionBase::parseOptions(
                        argc, argv, "MTA Analysis", "[options] <input-bitcode...>"
                    );

    if (Options::WriteAnder() == "ir_annotator")
    {
        LLVMModuleSet::preProcessBCs(moduleNameVec);
    }

    LLVMModuleSet::buildSVFModule(moduleNameVec);
    SVFIRBuilder builder;
    SVFIR* pag = builder.build();


    MTA mta;
    mta.runOnModule(pag);

    LLVMModuleSet::releaseLLVMModuleSet();


    return 0;
}
