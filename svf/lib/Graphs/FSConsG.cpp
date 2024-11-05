//
// Created by Jiahao Zhang on 19/8/2024.
//

#include "Graphs/FSConsG.h"

#include "Graphs/ConsG.h"
#include "Graphs/SVFG.h"
#include "Util/Options.h"
#include "WPA/Andersen.h"
#include "WPA/FlowSensitive.h"

using namespace SVF;
using namespace SVFUtil;

/*!
 * Start building constraint graph
 */
void FSConsG::buildSVFG2CG(SVFG* svfg)
{
    /// build constraint nodes
    for (auto it : *svfg)
    {

        VFGNode* node = it.second;

        if (StmtVFGNode* stmtNode = SVFUtil::dyn_cast<StmtVFGNode>(node))
        {
            if (SVFUtil::isa<AddrVFGNode>(stmtNode))
            {
                totalCGNode++;
                auto src = stmtNode->getPAGSrcNode()->getId();
                auto dst = stmtNode->getPAGDstNode()->getId();
                if (!hasConstraintNode(src))
                {
                    addConstraintNode(new ConstraintNode(src), src);
                }
                if (!hasConstraintNode(dst))
                {
                    addConstraintNode(new ConstraintNode(dst), dst);
                }
                addAddrCGEdge(src, dst);
            }

            if (SVFUtil::isa<CopyVFGNode>(stmtNode))
            {
                auto src = stmtNode->getPAGSrcNode()->getId();
                auto dst = stmtNode->getPAGDstNode()->getId();
            }

                /*
                NodeID addFSCGNode(svfgNodeID, pagNodeID)
                {
                    topLevel:
                    if (hasFSCGNode(getdef, pagNodeID))
                        return;
                    totalCGNode++;
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                    PairToIDMap[pair] = id;
                    IDToPairMap[id] = pair;
                    return totalCGNode;

                    pagIDTosvfgIDMap;



                    ID(pagID (top) / totalCGNodeID (addr));
                    Pair<pagID , svfgID >;

                    addrTaken:
                    if (hasFSCGNode(cgID, svfgID))
                        return;
                    totalCGNode++;
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                    PairToIDMap[pair] = id;
                    IDToPairMap[id] = pair;
                    return totalCGNode;
                }

                src = addFSCFNode(...);
                dst = addFSCGNode(..);
                addCopyCGEdge(src,dst);

                addFSNode
                totalCGNode++;

                if (!hasConstraintNode(totalCGNode))
                {
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                }
                totalCGNode++;
                if (!hasConstraintNode(totalCGNode))
                {
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                }
                addCopyCGEdge(src, dst);
                */

            if (SVFUtil::isa<LoadVFGNode>(stmtNode))
            {
                auto src = stmtNode->getPAGSrcNode()->getId();
                auto dst = stmtNode->getPAGDstNode()->getId();
                if (!hasConstraintNode(src))
                {
                    addConstraintNode(new ConstraintNode(src), src);
                }
                if (!hasConstraintNode(dst))
                {
                    addConstraintNode(new ConstraintNode(dst), dst);
                }
                addLoadCGEdge(src, dst);
            }
            if (SVFUtil::isa<StoreVFGNode>(stmtNode))
            {
                auto src = stmtNode->getPAGSrcNode()->getId();
                auto dst = stmtNode->getPAGDstNode()->getId();
                if (!hasConstraintNode(src))
                {
                    addConstraintNode(new ConstraintNode(src), src);
                }
                if (!hasConstraintNode(dst))
                {
                    addConstraintNode(new ConstraintNode(dst), dst);
                }
                addStoreCGEdge(src, dst);
            }
            if (SVFUtil::isa<GepVFGNode>(stmtNode))
            {
                auto src = stmtNode->getPAGSrcNode()->getId();
                auto dst = stmtNode->getPAGDstNode()->getId();
                if (!hasConstraintNode(src))
                {
                    addConstraintNode(new ConstraintNode(src), src);
                }
                if (!hasConstraintNode(dst))
                {
                    addConstraintNode(new ConstraintNode(dst), dst);
                }
                const GepStmt* edge =
                    SVFUtil::cast<GepStmt>(stmtNode->getPAGEdge());
                if (edge->isVariantFieldGep())
                {
                    addVariantGepCGEdge(src, dst);
                }
                else
                {
                    addNormalGepCGEdge(src, dst, edge->getAccessPath());
                }
            }
        }
        if (IntraPHIVFGNode* phiNode = SVFUtil::dyn_cast<IntraPHIVFGNode>(node))
        {
            auto dst = phiNode->getRes()->getId();
            if (!hasConstraintNode(dst))
            {
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            for (Map<u32_t,const PAGNode*>::const_iterator iter = phiNode->opVerBegin(), eiter = phiNode->opVerEnd();
                 iter != eiter; ++iter)
            {
                auto src = iter->second->getId();
                if (!hasConstraintNode(src))
                {
                    addConstraintNode(new ConstraintNode(src), src);
                }
                addCopyCGEdge(iter->second->getId(), dst);
            }
        }

        /// top-level call and ret
        if (ActualParmVFGNode* aparm = SVFUtil::dyn_cast<ActualParmVFGNode>(node))
        {
            auto node = aparm->getParam()->getId();
            if (!hasConstraintNode(node))
            {
                addConstraintNode(new ConstraintNode(node), node);
            }
        }

        if (FormalParmVFGNode* fparm = SVFUtil::dyn_cast<FormalParmVFGNode>(node))
        {
            auto node = fparm->getParam()->getId();
            if (!hasConstraintNode(node))
            {
                addConstraintNode(new ConstraintNode(node), node);
            }

            auto i = 0;
            for (auto iter = fparm->callPEBegin(), eiter = fparm->callPEEnd();
                 iter != eiter; ++iter, i++)
            {
                auto* callPE = *iter;
                auto callICFGNode = callPE->getCallSite();
                // auto funEntryICFGNode = callPE->getFunEntryICFGNode();
                auto src = callICFGNode->getActualParms();
                auto id = src[i]->getId();
                addCopyCGEdge(id, node);
            }
        }

        if (ActualRetVFGNode* aret = SVFUtil::dyn_cast<ActualRetVFGNode>(node))
        {
            auto node = aret->getRev()->getId();
            if (!hasConstraintNode(node))
            {
                addConstraintNode(new ConstraintNode(node), node);
            }
        }

        if (FormalRetVFGNode* fret = SVFUtil::dyn_cast<FormalRetVFGNode>(node))
        {
            auto node = fret->getRet()->getId();
            if (!hasConstraintNode(node))
            {
                addConstraintNode(new ConstraintNode(node), node);
            }

            for (auto iter = fret->retPEBegin(), eiter = fret->retPEEnd();
                 iter != eiter; ++iter)
            {
                 auto* RetPE = *iter;
                 auto callICFGNode = RetPE->getCallSite();
                // auto funEntryICFGNode = RetPE->getFunExitICFGNode();
                 auto src = callICFGNode->getRetICFGNode()->getActualRet();
                 auto id = src->getId();
                addCopyCGEdge(id, node);
            }
        }

        if (NullPtrVFGNode* nptr = SVFUtil::dyn_cast<NullPtrVFGNode>(node))
        {
            auto node = nptr->getPAGNode()->getId();
            if (!hasConstraintNode(node))
            {
                addConstraintNode(new ConstraintNode(node), node);
            }
        }

        /// address-taken call and ret
        if (MSSAPHISVFGNode* mphiNode = SVFUtil::dyn_cast<MSSAPHISVFGNode>(node))
        {
            auto dst = mphiNode->getResVer()->getID();
            if (!hasConstraintNode(dst))
            {
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            for (auto iter = mphiNode->opVerBegin(), eiter = mphiNode->opVerEnd();
                 iter != eiter; ++iter)
            {
                auto src = iter->first;
                if (!hasConstraintNode(src))
                {
                    addConstraintNode(new ConstraintNode(src), src);
                }
                addCopyCGEdge(iter->first, dst);
            }
        }

        if (ActualINSVFGNode* ain = SVFUtil::dyn_cast<ActualINSVFGNode>(node))
        {
            auto node = ain->getMRVer()->getMR()->getPointsTo();
            for(auto i : node)
            {
                if (!hasConstraintNode(i))
                {
                    addConstraintNode(new ConstraintNode(i), i);
                }
            }
        }

        if (FormalINSVFGNode* fparm = SVFUtil::dyn_cast<FormalINSVFGNode>(node))
        {
            auto node = fparm->getMRVer()->getMR()->getPointsTo();
            for(auto i : node)
            {
                if (!hasConstraintNode(i))
                {
                    addConstraintNode(new ConstraintNode(i), i);
                }
            }
        }

        if (ActualOUTSVFGNode* aret = SVFUtil::dyn_cast<ActualOUTSVFGNode>(node))
        {
            auto node = aret->getMRVer()->getMR()->getPointsTo();
            for(auto i : node)
            {
                if (!hasConstraintNode(i))
                {
                    addConstraintNode(new ConstraintNode(i), i);
                }
            }
        }

        if (FormalOUTSVFGNode* fret = SVFUtil::dyn_cast<FormalOUTSVFGNode>(node))
        {
            auto node = fret->getMRVer()->getMR()->getPointsTo();
            for(auto i : node)
            {
                if (!hasConstraintNode(i))
                {
                    addConstraintNode(new ConstraintNode(i), i);
                }
            }
        }
    }

    /// connect indirect constraint edges
    std::set<IndirectSVFGEdge> indirectEdges;
    for (auto edge : indirectEdges)
    {
        auto src = edge.getSrcID();
        auto dst = edge.getDstID();
        addCopyCGEdge(src, dst);
    }
}
