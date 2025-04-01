
#ifndef SVF_SVFSTAT_H
#define SVF_SVFSTAT_H

namespace SVF
{


/*!
 * Pointer Analysis Statistics
 */
class SVFStat
{
public:

    typedef OrderedMap<std::string, u32_t> NUMStatMap;

    typedef OrderedMap<std::string, double> TIMEStatMap;

    enum ClockType
    {
        Wall,
        CPU,
    };

    SVFStat();

    virtual ~SVFStat() {}

    virtual inline void startClk()
    {
        startTime = getClk(true);
    }

    virtual inline void endClk()
    {
        endTime = getClk(true);
    }

    /// When mark is true, real clock is always returned. When mark is false, it is
    /// only returned when Options::MarkedClocksOnly is not set.
    /// Default call for getClk is unmarked, while MarkedClocksOnly is false by default.
    static double getClk(bool mark = false);

    /// SVF's general statistics are only printed once even if you run multiple anayses
    static bool printGeneralStats;

    NUMStatMap generalNumMap;
    NUMStatMap PTNumStatMap;
    TIMEStatMap timeStatMap;

    double startTime;
    double endTime;

    virtual void performStat() = 0;

    virtual void printStat(std::string str = "");

    virtual void performStatPerQuery(NodeID) {}

    virtual void printStatPerQuery(NodeID, const PointsTo &) {}

    virtual void callgraphStat() {}

    static double timeOfBuildingLLVMModule;
    static double timeOfBuildingSymbolTable;
    static double timeOfBuildingSVFIR;

private:
    void branchStat();
    std::string moduleName;
}; // End class SVFStat

} // End namespace SVF
#endif //SVF_SVFSTAT_H
