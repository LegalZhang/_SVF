//
// Created by Jiahao Zhang on 18/11/2024.
//

#ifndef FLOWSENSITIVECG_H
#define FLOWSENSITIVECG_H

namespace SVF
{

class FlowsensitiveCG : public Andersen
{
public:

protected:
    virtual void handleLoadStore(ConstraintNode* node);
    virtual bool processStore(NodeID node, const ConstraintEdge* store);
};

} // namespace SVF

#endif // FLOWSENSITIVECG_H