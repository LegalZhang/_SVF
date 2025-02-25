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

protected:
    virtual void solveWorklist() override;
    virtual void processNode(NodeID nodeId) override;
    // virtual void processAllAddr();
    // virtual void processAddr(const AddrCGEdge* addr) override;
    // virtual void collapsePWCNode(NodeID nodeId) override;
    // bool collapseNodePts(NodeID nodeId);
    // void collapseFields() override;
    // bool collapseField(NodeID nodeId);
    // virtual void mergeNodeToRep(NodeID nodeId,NodeID newRepId) override;
    // virtual bool processCopy(NodeID node, const ConstraintEdge* edge) override;
    // virtual bool processGep(NodeID node, const GepCGEdge* edge) override;
    // virtual bool processGepPts(const PointsTo& pts, const GepCGEdge* edge) override;
    // virtual void handleCopyGep(ConstraintNode* node) override;

    virtual void postProcessNode(NodeID nodeId);
    virtual bool handleStore(NodeID node, const ConstraintEdge* store);
    virtual bool processStore(NodeID node, const ConstraintEdge* store) override;
    virtual bool handleLoad(NodeID node, const ConstraintEdge* load);
    virtual bool processLoad(NodeID node, const ConstraintEdge* load) override;

    NodeID getAddrDef(NodeID consgid, NodeID svfgid);
    bool isStrongUpdate(const StoreCGEdge* store, NodeID& singleton);

    SVFGBuilder memSSA;
    AndersenWaveDiff* ander;
    SVFG* svfg;
    FSConsG* consCG;
};

} // namespace SVF

#endif // FLOWSENSITIVECG_H