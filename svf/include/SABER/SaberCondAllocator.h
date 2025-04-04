
#ifndef PATHALLOCATOR_H_
#define PATHALLOCATOR_H_

#include "SVFIR/SVFValue.h"
#include "Util/WorkList.h"
#include "Graphs/SVFG.h"
#include "Util/Z3Expr.h"


namespace SVF
{

/**
 * SaberCondAllocator allocates conditions for each basic block of a certain CFG.
 */
class SaberCondAllocator
{

public:

    typedef Z3Expr Condition;   /// z3 condition
    typedef Map<u32_t, const ICFGNode*> IndexToTermInstMap; /// id to instruction map for z3
    typedef Map<u32_t,Condition> CondPosMap;		///< map a branch to its Condition
    typedef Map<const SVFBasicBlock*, CondPosMap > BBCondMap;	/// map bb to a Condition
    typedef Set<const SVFBasicBlock*> BasicBlockSet;
    typedef Map<const FunObjVar*,  BasicBlockSet> FunToExitBBsMap;  ///< map a function to all its basic blocks calling program exit
    typedef Map<const SVFBasicBlock*, Condition> BBToCondMap;	///< map a basic block to its condition during control-flow guard computation
    typedef FIFOWorkList<const SVFBasicBlock*> CFWorkList;	///< worklist for control-flow guard computation
    typedef Map<const SVFGNode*, Set<const SVFGNode*>> SVFGNodeToSVFGNodeSetMap;


    /// Constructor
    SaberCondAllocator();

    /// Destructor
    virtual ~SaberCondAllocator()
    {
    }
    /// Statistics
    //@{
    inline std::string getMemUsage()
    {
        u32_t vmrss, vmsize;
        if (SVFUtil::getMemoryUsageKB(&vmrss, &vmsize))
            return std::to_string(vmsize) + "KB";
        else
            return "cannot read memory usage";
    }
    inline u32_t getCondNum()
    {
        return totalCondNum;
    }
    //@}

    /// Condition operations
    //@{
    inline Condition condAnd(const Condition& lhs, const Condition& rhs)
    {
        return Condition::AND(lhs,rhs);
    }
    inline Condition condOr(const Condition& lhs, const Condition& rhs)
    {
        return Condition::OR(lhs,rhs);
    }
    inline Condition condNeg(const Condition& cond)
    {
        return Condition::NEG(cond);
    }
    inline Condition getTrueCond() const
    {
        return Condition::getTrueCond();
    }
    inline Condition getFalseCond() const
    {
        return Condition::getFalseCond();
    }
    /// Iterator every element of the condition
    inline NodeBS exactCondElem(const Condition& cond)
    {
        NodeBS elems;
        extractSubConds(cond, elems);
        return elems;
    }

    inline std::string dumpCond(const Condition& cond) const
    {
        return Condition::dumpStr(cond);
    }

    /// Allocate a new condition
    Condition newCond(const ICFGNode* inst);

    /// Perform path allocation
    void allocate();

    /// Get/Set instruction based on Z3 expression id
    //{@
    inline const ICFGNode* getCondInst(u32_t id) const
    {
        IndexToTermInstMap::const_iterator it = idToTermInstMap.find(id);
        assert(it != idToTermInstMap.end() && "this should be a fresh condition");
        return it->second;
    }

    inline void setCondInst(const Condition &condition, const ICFGNode* inst)
    {
        assert(idToTermInstMap.find(condition.id()) == idToTermInstMap.end() && "this should be a fresh condition");
        idToTermInstMap[condition.id()] = inst;
    }
    //@}

    bool isNegCond(u32_t id) const
    {
        return negConds.test(id);
    }

    inline bool postDominate(const SVFBasicBlock* bbKey, const SVFBasicBlock* bbValue) const
    {
        const FunObjVar*  keyFunc = bbKey->getParent();
        const FunObjVar*  valueFunc = bbValue->getParent();
        bool funcEq = (keyFunc == valueFunc);
        (void)funcEq; // Suppress warning of unused variable under release build
        assert(funcEq && "two basicblocks should be in the same function!");
        return keyFunc->postDominate(bbKey,bbValue);
    }

    inline bool dominate(const SVFBasicBlock* bbKey, const SVFBasicBlock* bbValue) const
    {
        const FunObjVar*  keyFunc = bbKey->getParent();
        const FunObjVar*  valueFunc = bbValue->getParent();
        bool funcEq = (keyFunc == valueFunc);
        (void)funcEq; // Suppress warning of unused variable under release build
        assert(funcEq && "two basicblocks should be in the same function!");
        return keyFunc->dominate(bbKey,bbValue);
    }

    /// Guard Computation for a value-flow (between two basic blocks)
    //@{
    virtual Condition ComputeIntraVFGGuard(const SVFBasicBlock* src, const SVFBasicBlock* dst);
    virtual Condition ComputeInterCallVFGGuard(const SVFBasicBlock* src, const SVFBasicBlock* dst, const SVFBasicBlock* callBB);
    virtual Condition ComputeInterRetVFGGuard(const SVFBasicBlock* src, const SVFBasicBlock* dst, const SVFBasicBlock* retBB);

    /// Get complement condition (from B1 to B0) according to a complementBB (BB2) at a phi
    /// e.g., B0: dstBB; B1:incomingBB; B2:complementBB
    virtual Condition getPHIComplementCond(const SVFBasicBlock* BB1, const SVFBasicBlock* BB2, const SVFBasicBlock* BB0);

    inline void clearCFCond()
    {
        bbToCondMap.clear();
    }
    /// Set current value for branch condition evaluation
    inline void setCurEvalSVFGNode(const SVFGNode* node)
    {
        curEvalSVFGNode = node;
    }
    /// Get current value for branch condition evaluation
    inline const SVFGNode* getCurEvalSVFGNode() const
    {
        return curEvalSVFGNode;
    }
    //@}

    /// Print out the path condition information
    void printPathCond();

    /// whether condition is satisfiable
    bool isSatisfiable(const Condition& condition);

    /// whether condition is satisfiable for all possible boolean guards
    inline bool isAllPathReachable(Condition& condition)
    {
        return isEquivalentBranchCond(condition, Condition::getTrueCond());
    }

    /// Whether lhs and rhs are equivalent branch conditions
    bool isEquivalentBranchCond(const Condition &lhs, const Condition &rhs) const;

    inline ICFG* getICFG() const
    {
        return PAG::getPAG()->getICFG();
    }

    /// Get/Set control-flow conditions
    //@{
    inline bool setCFCond(const SVFBasicBlock* bb, const Condition& cond)
    {
        BBToCondMap::iterator it = bbToCondMap.find(bb);
        // until a fixed-point is reached (condition is not changed)
        if(it!=bbToCondMap.end() && isEquivalentBranchCond(it->second, cond))
            return false;

        bbToCondMap[bb] = cond;
        return true;
    }
    inline Condition getCFCond(const SVFBasicBlock* bb) const
    {
        BBToCondMap::const_iterator it = bbToCondMap.find(bb);
        if(it==bbToCondMap.end())
        {
            return getFalseCond();
        }
        return it->second;
    }
    //@}


    /// mark neg Z3 expression
    inline void setNegCondInst(const Condition &condition, const ICFGNode* inst)
    {
        setCondInst(condition, inst);
        negConds.set(condition.id());
    }

    SVFGNodeToSVFGNodeSetMap & getRemovedSUVFEdges()
    {
        return removedSUVFEdges;
    }

private:

    /// Allocate path condition for every basic block
    virtual void allocateForBB(const SVFBasicBlock& bb);

    /// Get/Set a branch condition, and its terminator instruction
    //@{
    /// Set branch condition
    void setBranchCond(const SVFBasicBlock* bb, const SVFBasicBlock* succ, const Condition& cond);
    /// Get branch condition
    Condition getBranchCond(const SVFBasicBlock*  bb, const SVFBasicBlock* succ) const;
    ///Get a condition, evaluate the value for conditions if necessary (e.g., testNull like express)
    Condition getEvalBrCond(const SVFBasicBlock*  bb, const SVFBasicBlock* succ);
    //@}
    /// Evaluate branch conditions
    //@{
    /// Evaluate the branch condition
    Condition evaluateBranchCond(const SVFBasicBlock*  bb, const SVFBasicBlock* succ) ;
    /// Evaluate loop exit branch
    Condition evaluateLoopExitBranch(const SVFBasicBlock*  bb, const SVFBasicBlock* succ);
    /// Return branch condition after evaluating test null like expression
    Condition evaluateTestNullLikeExpr(const BranchStmt* branchStmt, const SVFBasicBlock* succ);
    /// Return condition when there is a branch calls program exit
    Condition evaluateProgExit(const BranchStmt* branchStmt, const SVFBasicBlock* succ);
    /// Collect basic block contains program exit function call
    void collectBBCallingProgExit(const SVFBasicBlock& bb);
    bool isBBCallsProgExit(const SVFBasicBlock* bb);
    //@}

    /// Evaluate test null/not null like expressions
    //@{
    /// Return true if the predicate of this compare instruction is equal
    bool isEQCmp(const CmpStmt* cmp) const;
    /// Return true if the predicate of this compare instruction is not equal
    bool isNECmp(const CmpStmt* cmp) const;
    /// Return true if this is a test null expression
    bool isTestNullExpr(const ICFGNode* test) const;
    /// Return true if this is a test not null expression
    bool isTestNotNullExpr(const ICFGNode* test) const;
    /// Return true if two values on the predicate are what we want
    bool isTestContainsNullAndTheValue(const CmpStmt* cmp) const;
    //@}

    /// Release memory
    void destroy()
    {

    }

    /// extract subexpression from a Z3 expression
    void extractSubConds(const Condition &condition, NodeBS &support) const;


    FunToExitBBsMap funToExitBBsMap;		///< map a function to all its basic blocks calling program exit
    BBToCondMap bbToCondMap;				///< map a basic block to its path condition starting from root
    const SVFGNode* curEvalSVFGNode{};			///< current llvm value to evaluate branch condition when computing guards
    IndexToTermInstMap idToTermInstMap;     ///key: z3 expression id, value: instruction
    NodeBS negConds;                        ///bit vector for distinguish neg
    std::vector<Condition> conditionVec;          /// vector storing z3expression
    static u32_t totalCondNum; /// a counter for fresh condition
    SVFGNodeToSVFGNodeSetMap removedSUVFEdges;

protected:
    BBCondMap bbConds;						///< map basic block to its successors/predecessors branch conditions

};

} // End namespace SVF

#endif /* PATHALLOCATOR_H_ */
