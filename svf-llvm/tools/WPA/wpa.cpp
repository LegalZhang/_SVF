
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "WPA/WPAPass.h"
#include "Util/CommandLine.h"
#include "Util/Options.h"


using namespace llvm;
using namespace std;
using namespace SVF;

int main(int argc, char** argv)
{
    auto moduleNameVec =
        OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                 "[options] <input-bitcode...>");

    // Refers to content of a singleton unique_ptr<SVFIR> in SVFIR.
    SVFIR* pag;

    if (Options::ReadJson())
    {
        assert(false && "please implement SVFIRReader::read");
    }
    else
    {
        if (Options::WriteAnder() == "ir_annotator")
        {
            LLVMModuleSet::preProcessBCs(moduleNameVec);
        }

        LLVMModuleSet::buildSVFModule(moduleNameVec);

        /// Build SVFIR
        SVFIRBuilder builder;
        pag = builder.build();

    }

    WPAPass wpa;
    wpa.runOnModule(pag);

    LLVMModuleSet::releaseLLVMModuleSet();
    return 0;
}
