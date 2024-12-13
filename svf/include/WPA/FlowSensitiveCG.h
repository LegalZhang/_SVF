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

    ~FlowSensitiveCG()
    {
        std::cerr << "Destroying FlowSensitiveCG..." << std::endl;
        if (fsconsCG) {
            delete fsconsCG;
            fsconsCG = nullptr;
        }
        if (svfg) {
            delete svfg;
            svfg = nullptr;
        }
    }

    /// Initialize analysis
    virtual void initialize() override;
    /// Finalize analysis
    virtual void finalize() override;

protected:
    virtual void solveWorklist() override;
    virtual void processNode(NodeID nodeId) override;
    virtual void postProcessNode(NodeID nodeId);
    virtual bool handleStore(NodeID node, const ConstraintEdge* store);
    virtual bool handleLoad(NodeID node, const ConstraintEdge* load);

    NodeID getAddrDef(NodeID consgid, NodeID svfgid);
    bool isStrongUpdate(const StoreCGEdge* store, NodeID& singleton);

    SVFGBuilder memSSA;
    AndersenWaveDiff *ander;
    SVFG* svfg;
    FSConsG* fsconsCG;
};

} // namespace SVF

#endif // FLOWSENSITIVECG_H