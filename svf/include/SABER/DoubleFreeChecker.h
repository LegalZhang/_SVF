
#ifndef DOUBLEFREECHECKER_H_
#define DOUBLEFREECHECKER_H_

#include "SABER/LeakChecker.h"

namespace SVF
{

/*!
 * Double free checker to check deallocations of memory
 */

class DoubleFreeChecker : public LeakChecker
{

public:
    /// Constructor
    DoubleFreeChecker(): LeakChecker()
    {
    }

    /// Destructor
    virtual ~DoubleFreeChecker()
    {
    }

    /// We start from here
    virtual bool runOnModule(SVFIR* pag) override
    {
        /// start analysis
        analyze();
        return false;
    }

    /// Report file/close bugs
    void reportBug(ProgSlice* slice) override;


    /// Validate test cases for regression test purpose
    void testsValidation(ProgSlice* slice);
    void validateSuccessTests(ProgSlice* slice, const FunObjVar* fun);
    void validateExpectedFailureTests(ProgSlice* slice, const FunObjVar* fun);
};

} // End namespace SVF

#endif /* DOUBLEFREECHECKER_H_ */
