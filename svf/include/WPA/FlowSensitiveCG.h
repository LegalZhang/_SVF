//
// Created by Jiahao Zhang on 18/11/2024.
//

#ifndef FLOWSENSITIVECG_H
#define FLOWSENSITIVECG_H

namespace SVF
{

class FlowsensiveCG : public Andersen
{
public:

protected:
    virtual void handleLoadStore(ConstraintNode* node);
};

} // namespace SVF

#endif // FLOWSENSITIVECG_H