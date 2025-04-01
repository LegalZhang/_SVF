
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "DDA/DDAPass.h"
#include "Util/Options.h"

using namespace llvm;
using namespace SVF;

//static cl::list<const PassInfo*, bool, PassNameParser>
//PassList(cl::desc("Optimizations available:"));

static Option<bool> DAA(
    "daa",
    "Demand-Driven Alias Analysis Pass",
    false
);

int main(int argc, char ** argv)
{
    std::vector<std::string> moduleNameVec;
    moduleNameVec = OptionBase::parseOptions(
                        argc, argv, "Demand-Driven Points-to Analysis", "[options] <input-bitcode...>"
                    );

    if (Options::WriteAnder() == "ir_annotator")
    {
        LLVMModuleSet::preProcessBCs(moduleNameVec);
    }

    LLVMModuleSet::buildSVFModule(moduleNameVec);
    SVFIRBuilder builder;
    SVFIR* pag = builder.build();

    DDAPass dda;
    dda.runOnModule(pag);

    LLVMModuleSet::releaseLLVMModuleSet();
    return 0;

}

