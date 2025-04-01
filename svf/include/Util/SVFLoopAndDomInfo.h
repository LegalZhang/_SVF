
#ifndef SVFLOOPANDDOMINFO_H
#define SVFLOOPANDDOMINFO_H

#include "SVFIR/SVFType.h"
#include "Graphs/GraphPrinter.h"
#include "Util/Casting.h"
#include "Graphs/BasicBlockG.h"

namespace SVF
{
class SVFLoopAndDomInfo
{
    friend class SVFIRWriter;
    friend class SVFIRReader;
public:
    typedef Set<const SVFBasicBlock*> BBSet;
    typedef std::vector<const SVFBasicBlock*> BBList;
    typedef BBList LoopBBs;

private:
    BBList reachableBBs;    ///< reachable BasicBlocks from the function entry.
    Map<const SVFBasicBlock*,BBSet> dtBBsMap;   ///< map a BasicBlock to BasicBlocks it Dominates
    Map<const SVFBasicBlock*,BBSet> pdtBBsMap;   ///< map a BasicBlock to BasicBlocks it PostDominates
    Map<const SVFBasicBlock*,BBSet> dfBBsMap;    ///< map a BasicBlock to its Dominate Frontier BasicBlocks
    Map<const SVFBasicBlock*, LoopBBs> bb2LoopMap;  ///< map a BasicBlock (if it is in a loop) to all the BasicBlocks in this loop
    Map<const SVFBasicBlock*, u32_t> bb2PdomLevel;  ///< map a BasicBlock to its level in pdom tree, used in findNearestCommonPDominator
    Map<const SVFBasicBlock*, const SVFBasicBlock*> bb2PIdom;  ///< map a BasicBlock to its immediate dominator in pdom tree, used in findNearestCommonPDominator

public:
    SVFLoopAndDomInfo()
    {
    }

    virtual ~SVFLoopAndDomInfo() {}

    inline const Map<const SVFBasicBlock*,BBSet>& getDomFrontierMap() const
    {
        return dfBBsMap;
    }

    inline Map<const SVFBasicBlock*,BBSet>& getDomFrontierMap()
    {
        return dfBBsMap;
    }

    inline bool hasLoopInfo(const SVFBasicBlock* bb) const
    {
        return bb2LoopMap.find(bb) != bb2LoopMap.end();
    }

    const LoopBBs& getLoopInfo(const SVFBasicBlock* bb) const;

    inline const SVFBasicBlock* getLoopHeader(const LoopBBs& lp) const
    {
        assert(!lp.empty() && "this is not a loop, empty basic block");
        return lp.front();
    }

    inline bool loopContainsBB(const LoopBBs& lp, const SVFBasicBlock* bb) const
    {
        return std::find(lp.begin(), lp.end(), bb) != lp.end();
    }

    inline void addToBB2LoopMap(const SVFBasicBlock* bb, const SVFBasicBlock* loopBB)
    {
        bb2LoopMap[bb].push_back(loopBB);
    }

    inline const Map<const SVFBasicBlock*,BBSet>& getPostDomTreeMap() const
    {
        return pdtBBsMap;
    }

    inline Map<const SVFBasicBlock*,BBSet>& getPostDomTreeMap()
    {
        return pdtBBsMap;
    }

    inline const Map<const SVFBasicBlock*,u32_t>& getBBPDomLevel() const
    {
        return bb2PdomLevel;
    }

    inline Map<const SVFBasicBlock*,u32_t>& getBBPDomLevel()
    {
        return bb2PdomLevel;
    }

    inline const Map<const SVFBasicBlock*,const SVFBasicBlock*>& getBB2PIdom() const
    {
        return bb2PIdom;
    }

    inline Map<const SVFBasicBlock*,const SVFBasicBlock*>& getBB2PIdom()
    {
        return bb2PIdom;
    }


    inline Map<const SVFBasicBlock*,BBSet>& getDomTreeMap()
    {
        return dtBBsMap;
    }

    inline const Map<const SVFBasicBlock*,BBSet>& getDomTreeMap() const
    {
        return dtBBsMap;
    }

    inline bool isUnreachable(const SVFBasicBlock* bb) const
    {
        return std::find(reachableBBs.begin(), reachableBBs.end(), bb) ==
               reachableBBs.end();
    }

    inline const BBList& getReachableBBs() const
    {
        return reachableBBs;
    }

    inline void setReachableBBs(BBList& bbs)
    {
        reachableBBs = bbs;
    }

    void getExitBlocksOfLoop(const SVFBasicBlock* bb, BBList& exitbbs) const;

    bool isLoopHeader(const SVFBasicBlock* bb) const;

    bool dominate(const SVFBasicBlock* bbKey, const SVFBasicBlock* bbValue) const;

    bool postDominate(const SVFBasicBlock* bbKey, const SVFBasicBlock* bbValue) const;

    /// find nearest common post dominator of two basic blocks
    const SVFBasicBlock *findNearestCommonPDominator(const SVFBasicBlock *A, const SVFBasicBlock *B) const;
};
}

#endif //SVFLOOPANDDOMINFO_H
