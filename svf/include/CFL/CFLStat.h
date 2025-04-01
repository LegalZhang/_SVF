
#ifndef CFL_CFLSTAT_H_
#define CFL_CFLSTAT_H_

#include "Util/PTAStat.h"
#include "CFL/CFLAlias.h"
#include "CFL/CFLVF.h"

namespace SVF
{

/*!
 * Statistics of CFL's analysis
 */
class CFLStat : public PTAStat
{
private:
    CFLBase* pta;

public:
    CFLStat(CFLBase* p);

    virtual ~CFLStat()
    {
    }

    virtual void performStat();

    void CFLGraphStat();

    void CFLGrammarStat();

    void CFLSolverStat();
};

} // End namespace SVF

#endif /* CFL_CFLSTAT_H_ */
