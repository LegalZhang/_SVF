
#ifndef FILECHECK_H_
#define FILECHECK_H_

#include "SABER/LeakChecker.h"

namespace SVF
{

/*!
 * File open/close checker to check consistency of file operations
 */

class FileChecker : public LeakChecker
{

public:

    /// Constructor
    FileChecker(): LeakChecker()
    {
    }

    /// Destructor
    virtual ~FileChecker()
    {
    }

    /// We start from here
    virtual bool runOnModule(SVFIR* pag)
    {
        /// start analysis
        analyze();
        return false;
    }

    inline bool isSourceLikeFun(const FunObjVar* fun)
    {
        return SaberCheckerAPI::getCheckerAPI()->isFOpen(fun);
    }
    /// Whether the function is a heap deallocator (free/release memory)
    inline bool isSinkLikeFun(const FunObjVar* fun)
    {
        return SaberCheckerAPI::getCheckerAPI()->isFClose(fun);
    }
    /// Report file/close bugs
    void reportBug(ProgSlice* slice);
};

} // End namespace SVF

#endif /* FILECHECK_H_ */
