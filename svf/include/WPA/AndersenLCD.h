//
// Created by Jiahao Zhang on 19/6/2024.
//

#ifndef ANDERSENLCD_H
#define ANDERSENLCD_H

#include "WPA/Andersen.h"

namespace SVF
{

class AndersenLCD : public Andersen
{
private:
    static AndersenLCD* lcdAndersen;

public:
    /// Constructor
    AndersenLCD(SVFIR* _pag, PTATY type = AndersenLCD_WPA)
        : Andersen(_pag, type)
    {
    }

    /// Destructor
    virtual ~AndersenLCD() {}

    virtual void analyze();

    virtual void solveWorklist();

    virtual bool LCDprocessCopy(NodeID node, const ConstraintEdge* edge);

    /// About R
    /// Define a hash structure for ConstraintEdge
    struct ConstraintEdgeHash
    {
        size_t operator()(const ConstraintEdge* edge) const
        {
            return std::hash<EdgeID>()(edge->getEdgeID());
        }
    };

    /// Define equality for ConstraintEdge
    struct ConstraintEdgeEqual
    {
        bool operator()(const ConstraintEdge* lhs,
                        const ConstraintEdge* rhs) const
        {
            return lhs->getEdgeID() == rhs->getEdgeID();
        }
    };

    using ConstraintEdgeSet =
        std::unordered_set<const ConstraintEdge*, ConstraintEdgeHash,
                           ConstraintEdgeEqual>;
};

} // namespace SVF

#endif // ANDERSENLCD_H