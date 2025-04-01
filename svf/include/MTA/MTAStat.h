
#ifndef MTASTAT_H_
#define MTASTAT_H_

#include "Util/PTAStat.h"

namespace SVF
{

class Instruction;
class ThreadCallGraph;
class TCT;
class MHP;
class LockAnalysis;
class MTAAnnotator;
/*!
 * Statistics for MTA
 */
class MTAStat : public PTAStat
{

public:
    typedef Set<const Instruction*> InstSet;

    /// Constructor
    MTAStat():PTAStat(nullptr),TCTTime(0),MHPTime(0),AnnotationTime(0)
    {
    }
    /// Statistics for thread call graph
    void performThreadCallGraphStat(ThreadCallGraph* tcg);
    /// Statistics for thread creation tree
    void performTCTStat(TCT* tct);
    /// Statistics for MHP statement pairs
    void performMHPPairStat(MHP* mhp, LockAnalysis* lsa);
    /// Statistics for annotation
    //void performAnnotationStat(MTAAnnotator* anno);

    double TCTTime;
    double MHPTime;
    double AnnotationTime;
};

} // End namespace SVF

#endif /* MTASTAT_H_ */
