//
// Created by Jiahao Zhang on 18/11/2024.
//

#ifndef FLOWSENSITIVECG_H
#define FLOWSENSITIVECG_H

#include "Graphs/FSConsG.h"

namespace SVF
{

class FlowSensitiveCG : public Andersen
{
public:
    /// Constructor
    FlowSensitiveCG(SVFIR* _pag, PTATY type = AndersenFS_WPA, bool alias_check = false) : Andersen(_pag, type) {}

    ~FlowSensitiveCG() override
    {
    }

    /// Initialize analysis
    virtual void initialize() override;
    /// Finalize analysis
    virtual void finalize() override;
    /// Solve constraints
    virtual void solveConstraints() override;

    ConstraintGraph* getFSConsG()
    {
        return fsconsCG;
    }

    // virtual void graphFolding();

    virtual bool isRepNode(NodeID nodeId)
    {
        return fsconsCG->nodeToRepMap.find(nodeId) != fsconsCG->nodeToRepMap.end();;
    }

protected:
    virtual void solveWorklist() override;
    virtual void processNode(NodeID nodeId) override;
    virtual void processAllAddr();
    virtual void processAddr(const AddrCGEdge* addr) override;

    virtual NodeStack& SCCDetect() override;
    void mergeSccCycle();
    void mergeSccNodes(NodeID repNodeId, const NodeBS& subNodes);
    virtual bool mergeSrcToTgt(NodeID srcId,NodeID tgtId) override;
    void updateNodeRepAndSubs(NodeID nodeId,NodeID newRepId);

    virtual void collapsePWCNode(NodeID nodeId) override;
    bool collapseNodePts(NodeID nodeId);
    void collapseFields() override;
    bool collapseField(NodeID nodeId);
    void mergeNodeToRep(NodeID nodeId,NodeID newRepId) override;

    void handleCopyGep(ConstraintNode* node) override;
    bool processCopy(NodeID node, const ConstraintEdge* edge) override;
    bool processGep(NodeID node, const GepCGEdge* edge) override;\
    bool processGepPts(const PointsTo& pts, const GepCGEdge* edge) override;

    virtual void postProcessNode(NodeID nodeId);
    virtual bool handleStore(NodeID node, const ConstraintEdge* store);
    virtual bool processStore(NodeID node, const ConstraintEdge* store) override;
    virtual bool handleLoad(NodeID node, const ConstraintEdge* load);
    virtual bool processLoad(NodeID node, const ConstraintEdge* load) override;

    NodeID getAddrDef(NodeID consgid, NodeID svfgid);
    bool isStrongUpdate(const StoreCGEdge* store, NodeID& singleton);

    /// SCC methods
    //@{
    inline NodeID sccRepNode(NodeID id) const override
    {
        return fsconsCG->sccRepNode(id);
    }
    inline NodeBS& sccSubNodes(NodeID repId) override
    {
        return fsconsCG->sccSubNodes(repId);
    }
    //@}

    /// Handle diff points-to set.
    virtual inline void computeDiffPts(NodeID id) override
    {
        if (Options::DiffPts())
        {
            NodeID rep = sccRepNode(id);
            getDiffPTDataTy()->computeDiffPts(rep, getDiffPTDataTy()->getPts(rep));
        }
    }
    virtual inline const PointsTo& getDiffPts(NodeID id) override
    {
        NodeID rep = sccRepNode(id);
        if (Options::DiffPts())
            return getDiffPTDataTy()->getDiffPts(rep);
        else
            return getPTDataTy()->getPts(rep);
    }

    /// Operation of points-to set
    virtual inline const PointsTo& getPts(NodeID id) override
    {
        return getPTDataTy()->getPts(sccRepNode(id));
    }
    virtual inline bool unionPts(NodeID id, const PointsTo& target) override
    {
        id = sccRepNode(id);
        return getPTDataTy()->unionPts(id, target);
    }
    virtual inline bool unionPts(NodeID id, NodeID ptd) override
    {
        id = sccRepNode(id);
        ptd = sccRepNode(ptd);
        return getPTDataTy()->unionPts(id,ptd);
    }

    /// Add copy edge on constraint graph
    virtual inline bool addCopyEdge(NodeID src, NodeID dst) override
    {
        if (fsconsCG->addCopyCGEdge(src, dst))
        {
            updatePropaPts(src, dst);
            return true;
        }
        return false;
    }

    inline void updatePropaPts(NodeID dstId, NodeID srcId)
    {
        if (!Options::DiffPts())
            return;
        NodeID srcRep = sccRepNode(srcId);
        NodeID dstRep = sccRepNode(dstId);
        getDiffPTDataTy()->updatePropaPtsMap(srcRep, dstRep);
    }

    SVFGBuilder memSSA;
    AndersenWaveDiff* ander;
    SVFG* svfg;
    FSConsG* fsconsCG;
};

} // namespace SVF

#endif // FLOWSENSITIVECG_H