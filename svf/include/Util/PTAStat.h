
#ifndef ANDERSENSTAT_H_
#define ANDERSENSTAT_H_

#include "SVFIR/SVFValue.h"
#include "MemoryModel/PointsTo.h"
#include "Util/SVFStat.h"
#include <iostream>
#include <map>
#include <string>

namespace SVF
{

class PointerAnalysis;

/*!
 * Pointer Analysis Statistics
 */
class PTAStat: public SVFStat
{
public:
    PTAStat(PointerAnalysis* p);
    virtual ~PTAStat() {}

    NodeBS localVarInRecursion;

    inline void setMemUsageBefore(u32_t vmrss, u32_t vmsize)
    {
        _vmrssUsageBefore = vmrss;
        _vmsizeUsageBefore = vmsize;
    }

    inline void setMemUsageAfter(u32_t vmrss, u32_t vmsize)
    {
        _vmrssUsageAfter = vmrss;
        _vmsizeUsageAfter = vmsize;
    }

    void performStat() override;

    void callgraphStat() override;


protected:
    PointerAnalysis* pta;
    u32_t _vmrssUsageBefore;
    u32_t _vmrssUsageAfter;
    u32_t _vmsizeUsageBefore;
    u32_t _vmsizeUsageAfter;
};

} // End namespace SVF

#endif /* ANDERSENSTAT_H_ */
