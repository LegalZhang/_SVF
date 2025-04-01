

#include "Graphs/CDG.h"

using namespace SVF;

CDG *CDG::controlDg = nullptr;

void CDG::addCDGEdgeFromSrcDst(const ICFGNode *src, const ICFGNode *dst, const SVFVar *pNode, s32_t branchID)
{
    if (!hasCDGNode(src->getId()))
    {
        addGNode(src->getId(), new CDGNode(src));
    }
    if (!hasCDGNode(dst->getId()))
    {
        addGNode(dst->getId(), new CDGNode(dst));
    }
    if (!hasCDGEdge(getCDGNode(src->getId()), getCDGNode(dst->getId())))
    {
        CDGEdge *pEdge = new CDGEdge(getCDGNode(src->getId()),
                                     getCDGNode(dst->getId()));
        pEdge->insertBranchCondition(pNode, branchID);
        addCDGEdge(pEdge);
        incEdgeNum();
    }
    else
    {
        CDGEdge *pEdge = getCDGEdge(getCDGNode(src->getId()),
                                    getCDGNode(dst->getId()));
        pEdge->insertBranchCondition(pNode, branchID);
    }
}