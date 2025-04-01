

#ifndef FLOWSENSITIVEFSCG_H
#define FLOWSENSITIVEFSCG_H

#include "Graphs/FSConsG.h"
#include "WPA/AndersenPWC.h"

namespace SVF
{

class FlowSensitiveSCD : public Andersen
{
public:
    typedef Map<NodeID, NodeID> NodeToNodeMap;

protected:
    NodeSet sccCandidates;
    NodeToNodeMap pwcReps;

public:
    /// Constructor
    FlowSensitiveSCD(SVFIR* _pag, PTATY type = FlowSensitiveSCD_WPA, bool alias_check = false) : Andersen(_pag, type) {}

    ~FlowSensitiveSCD() override
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

protected:
    virtual bool addCopyEdge(NodeID src, NodeID dst) override;
    virtual bool addCopyEdgeOriginal(NodeID src, NodeID dst);

    inline void addSccCandidate(NodeID nodeId)
    {
        sccCandidates.insert(sccRepNode(nodeId));
    }
    virtual NodeStack& SCCDetect() override;
    virtual void PWCDetect();
    virtual void solveWorklist() override;
    virtual void handleLoadStore(ConstraintNode* node) override;
    virtual void processAddr(const AddrCGEdge* addr) override;
    // virtual bool addCopyEdge(NodeID src, NodeID dst);
    virtual bool updateCallGraph(const CallSiteToFunPtrMap& callsites) override;
    virtual void processPWC(ConstraintNode* rep);
    virtual void handleCopyGep(ConstraintNode* node) override;

    virtual void processNode(NodeID nodeId) override;
    virtual void processAllAddr();

    void mergeSccCycle();
    void mergeSccNodes(NodeID repNodeId, const NodeBS& subNodes);
    virtual bool mergeSrcToTgt(NodeID srcId,NodeID tgtId) override;
    void updateNodeRepAndSubs(NodeID nodeId,NodeID newRepId);

    virtual void collapsePWCNode(NodeID nodeId) override;
    bool collapseNodePts(NodeID nodeId);
    void collapseFields() override;
    bool collapseField(NodeID nodeId);
    void mergeNodeToRep(NodeID nodeId,NodeID newRepId) override;

    void handleCopyGepOriginal(ConstraintNode* node);
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

} // End namespace SVF


#endif //FLOWSENSITIVEFSCG_H
