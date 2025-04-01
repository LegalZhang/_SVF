
#ifndef SVF_CDGBUILDER_H
#define SVF_CDGBUILDER_H

#include "Graphs/CDG.h"
#include "SVFIR/SVFValue.h"

// control dependence builder
namespace SVF
{
class CDGBuilder
{
public:

    /// constructor
    CDGBuilder() : _controlDG(CDG::getCDG())
    {

    }

    /// destructor
    ~CDGBuilder()
    {

    }

    /// start here
    void build();

    /// build control dependence for each function
    void buildControlDependence();

    /// build map at icfg node level
    void buildICFGNodeControlMap();



private:


    /// extract basic block edges to be processed
    static void
    extractBBS(const FunObjVar *func,
               Map<const SVFBasicBlock *, std::vector<const SVFBasicBlock *>> &res);

    /// extract nodes between two nodes in pdom tree
    void
    extractNodesBetweenPdomNodes(const SVFBasicBlock *succ, const SVFBasicBlock *LCA,
                                 std::vector<const SVFBasicBlock *> &tgtNodes);


    void dfsNodesBetweenPdomNodes(const SVFBasicBlock *cur,
                                  const SVFBasicBlock *tgt,
                                  std::vector<const SVFBasicBlock *> &path,
                                  std::vector<const SVFBasicBlock *> &tgtNodes,
                                  SVFLoopAndDomInfo *ld);


    s64_t getBBSuccessorBranchID(const SVFBasicBlock *BB, const SVFBasicBlock *Succ);


    /// update map
    inline void updateMap(const SVFBasicBlock *pred, const SVFBasicBlock *bb, s32_t pos)
    {
        _svfcontrolMap[pred][bb].insert(pos);
        _svfdependentOnMap[bb][pred].insert(pos);
    }


private:
    CDG *_controlDG;
    Map<const SVFBasicBlock *, Map<const SVFBasicBlock *, Set<s32_t>>> _svfcontrolMap; ///< map a basicblock to its controlling BBs (position, set of BBs)
    Map<const SVFBasicBlock *, Map<const SVFBasicBlock *, Set<s32_t>>> _svfdependentOnMap; ///< map a basicblock to its dependent on BBs (position, set of BBs)
    Map<const ICFGNode *, Map<const ICFGNode *, Set<s32_t>>> _nodeControlMap; ///< map an ICFG node to its controlling ICFG nodes (position, set of Nodes)
    Map<const ICFGNode *, Map<const ICFGNode *, Set<s32_t>>> _nodeDependentOnMap; ///< map an ICFG node to its dependent on ICFG nodes (position, set of Nodes)
};
}


#endif //SVF_CDGBUILDER_H