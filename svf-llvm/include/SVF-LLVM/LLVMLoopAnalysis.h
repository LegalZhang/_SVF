
#ifndef SVF_LLVMLOOPANALYSIS_H
#define SVF_LLVMLOOPANALYSIS_H

#include "SVFIR/SVFIR.h"
#include "MemoryModel/SVFLoop.h"
#include "SVF-LLVM/BasicTypes.h"

namespace SVF
{
class LLVMLoopAnalysis
{
public:

    /// Constructor
    LLVMLoopAnalysis() = default;;

    /// Destructor
    virtual ~LLVMLoopAnalysis() = default;

    /// Build llvm loops based on LoopInfo analysis
    virtual void buildLLVMLoops(ICFG* icfg);

    /// Start from here
    virtual void build(ICFG *icfg);

    /// Build SVF loops based on llvm loops
    virtual void buildSVFLoops(ICFG *icfg, std::vector<const Loop *> &llvmLoops);
};
} // End namespace SVF

#endif //SVF_LLVMLOOPANALYSIS_H
