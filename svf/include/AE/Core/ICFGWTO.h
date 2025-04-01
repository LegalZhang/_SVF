
#ifndef SVF_ICFGWTO_H
#define SVF_ICFGWTO_H

#include "Graphs/ICFG.h"
#include "Graphs/WTO.h"

namespace SVF
{

typedef WTOComponent<ICFG> ICFGWTOComp;
typedef WTONode<ICFG> ICFGSingletonWTO;
typedef WTOCycle<ICFG> ICFGCycleWTO;

class ICFGWTO : public WTO<ICFG>
{
public:
    typedef WTO<ICFG> Base;
    typedef WTOComponentVisitor<ICFG>::WTONodeT ICFGWTONode;

    explicit ICFGWTO(ICFG* graph, const ICFGNode* node) : Base(graph, node) {}

    virtual ~ICFGWTO()
    {
    }

    inline void forEachSuccessor(
        const ICFGNode* node,
        std::function<void(const ICFGNode*)> func) const override
    {
        if (const auto* callNode = SVFUtil::dyn_cast<CallICFGNode>(node))
        {
            const ICFGNode* succ = callNode->getRetICFGNode();
            func(succ);
        }
        else
        {
            for (const auto& e : node->getOutEdges())
            {
                if (!e->isIntraCFGEdge() ||
                        node->getFun() != e->getDstNode()->getFun())
                    continue;
                func(e->getDstNode());
            }
        }
    }
};
} // namespace SVF

#endif // SVF_ICFGWTO_H
