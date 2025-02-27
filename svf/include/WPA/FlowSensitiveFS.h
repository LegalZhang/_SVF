//
// Created by Jiahao Zhang on 27/2/2025.
//

#ifndef FLOWSENSITIVEFS_H
#define FLOWSENSITIVEFS_H

#include "WPA/Andersen.h"
#include "Graphs/FSConsG.h"

namespace SVF
{

class FlowSensitiveFS : public Andersen
{
public:
    /// Constructor
    FlowSensitiveFS(SVFIR* _pag, PTATY type = AndersenFSCG_WPA, bool alias_check = false) : Andersen(_pag, type) {}

    ~FlowSensitiveFS() override
    {
    }

    /// Initialize analysis
    virtual void initialize() override;

    SVFGBuilder memSSA;
    AndersenWaveDiff* ander;
    SVFG* svfg;
    FSConsG* fsconsCG;
};

} // End namespace SVF



#endif //FLOWSENSITIVEFS_H
