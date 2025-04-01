
#include "SVF-LLVM/SVFIRBuilder.h"
#include "Util/CommandLine.h"
#include "Util/Options.h"


using namespace std;
using namespace SVF;

#include <iostream>
#include <string>

std::string replaceExtension(const std::string& path)
{
    size_t pos = path.rfind('.');
    if (pos == std::string::npos ||
            (path.substr(pos) != ".bc" && path.substr(pos) != ".ll"))
    {
        SVFUtil::errs() << "Error: expect file with extension .bc or .ll\n";
        exit(EXIT_FAILURE);
    }
    return path.substr(0, pos) + ".svf.json";
}

int main(int argc, char** argv)
{
    auto moduleNameVec = OptionBase::parseOptions(
                             argc, argv, "llvm2svf", "[options] <input-bitcode...>");

    if (Options::WriteAnder() == "ir_annotator")
    {
        LLVMModuleSet::preProcessBCs(moduleNameVec);
    }

    const std::string jsonPath = replaceExtension(moduleNameVec.front());
    // PAG is borrowed from a unique_ptr, so we don't need to delete it.

    assert(false && "Please implement SVFIRWriter::writeJsonToPath");
    SVFUtil::outs() << "SVF IR is written to '" << jsonPath << "'\n";
    LLVMModuleSet::releaseLLVMModuleSet();
    return 0;
}
