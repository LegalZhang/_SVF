
#ifndef MTA_H_
#define MTA_H_

#include <set>
#include <vector>
#include "SVFIR/SVFValue.h"

namespace SVF
{

class PointerAnalysis;
class AndersenWaveDiff;
class ThreadCallGraph;
class MTAStat;
class TCT;
class MHP;
class LockAnalysis;

/*!
 * Base data race detector
 */
class MTA
{

public:
    /// Constructor
    MTA();

    /// Destructor
    virtual ~MTA();


    /// We start the pass here
    virtual bool runOnModule(SVFIR* module);
    /// Compute MHP
    virtual MHP* computeMHP();
    /// Compute locksets
    virtual LockAnalysis* computeLocksets(TCT* tct);
    /// Perform detection
    virtual void detect();

    // Not implemented for now
    // void dump(Module &module, MHP *mhp, LockAnalysis *lsa);

    MHP* getMHP()
    {
        return mhp;
    }

    LockAnalysis* getLockAnalysis()
    {
        return lsa;
    }
private:
    ThreadCallGraph* tcg;
    std::unique_ptr<TCT> tct;
    std::unique_ptr<MTAStat> stat;
    MHP* mhp;
    LockAnalysis* lsa;
};

} // End namespace SVF

#endif /* MTA_H_ */
