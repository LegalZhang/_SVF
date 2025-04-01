
#ifndef LEAKCHECKER_H_
#define LEAKCHECKER_H_

#include "SABER/SrcSnkDDA.h"
#include "SABER/SaberCheckerAPI.h"

namespace SVF
{

/*!
 * Static Memory Leak Detector
 */
class LeakChecker : public SrcSnkDDA
{

public:
    typedef Map<const SVFGNode*,const CallICFGNode*> SVFGNodeToCSIDMap;
    typedef FIFOWorkList<const CallICFGNode*> CSWorkList;
    typedef ProgSlice::VFWorkList WorkList;
    typedef NodeBS SVFGNodeBS;
    enum LEAK_TYPE
    {
        NEVER_FREE_LEAK,
        CONTEXT_LEAK,
        PATH_LEAK,
        GLOBAL_LEAK
    };

    /// Constructor
    LeakChecker()
    {
    }
    /// Destructor
    virtual ~LeakChecker()
    {
    }

    /// We start from here
    virtual bool runOnModule(SVFIR* pag)
    {
        /// start analysis
        analyze();
        return false;
    }

    /// Initialize sources and sinks
    //@{
    /// Initialize sources and sinks
    virtual void initSrcs() override;
    virtual void initSnks() override;
    /// Whether the function is a heap allocator/reallocator (allocate memory)
    virtual inline bool isSourceLikeFun(const FunObjVar* fun) override
    {
        return SaberCheckerAPI::getCheckerAPI()->isMemAlloc(fun);
    }
    /// Whether the function is a heap deallocator (free/release memory)
    virtual inline bool isSinkLikeFun(const FunObjVar* fun) override
    {
        return SaberCheckerAPI::getCheckerAPI()->isMemDealloc(fun);
    }
    //@}

protected:
    /// Report leaks
    //@{
    virtual void reportBug(ProgSlice* slice) override;
    //@}

    /// Validate test cases for regression test purpose
    void testsValidation(const ProgSlice* slice);
    void validateSuccessTests(const SVFGNode* source, const FunObjVar* fun);
    void validateExpectedFailureTests(const SVFGNode* source, const FunObjVar* fun);

    /// Record a source to its callsite
    //@{
    inline void addSrcToCSID(const SVFGNode* src, const CallICFGNode* cs)
    {
        srcToCSIDMap[src] = cs;
    }
    inline const CallICFGNode* getSrcCSID(const SVFGNode* src)
    {
        SVFGNodeToCSIDMap::iterator it =srcToCSIDMap.find(src);
        assert(it!=srcToCSIDMap.end() && "source node not at a callsite??");
        return it->second;
    }
    //@}
private:
    SVFGNodeToCSIDMap srcToCSIDMap;
};

} // End namespace SVF

#endif /* LEAKCHECKER_H_ */
