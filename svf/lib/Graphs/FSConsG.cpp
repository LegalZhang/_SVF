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
    /// build top-level constraint nodes
    for (auto it : *svfg)
    {
        SVFGNode* node = it.second;

        if (AddrSVFGNode* addrNode = SVFUtil::dyn_cast<AddrSVFGNode>(node))
        {
            auto svfgID = addrNode->getId();
            auto src = addrNode->getPAGSrcNode()->getId();
            if (!hasConstraintNode(src))
            {
                totalCGNode++;
                idTopairMap[src] = NodePair(src, svfgID);
                pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = addrNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                totalCGNode++;
                idTopairMap[dst] = NodePair(dst, svfgID);
                pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            addAddrCGEdge(src, dst);
        }

        if (CopySVFGNode* copyNode = SVFUtil::dyn_cast<CopySVFGNode>(node))
        {
            auto svfgID = copyNode->getId();
            auto src = copyNode->getPAGSrcNode()->getId();
            if (!hasConstraintNode(src))
            {
                totalCGNode++;
                idTopairMap[src] = NodePair(src, svfgID);
                pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = copyNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                totalCGNode++;
                idTopairMap[dst] = NodePair(dst, svfgID);
                pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            addCopyCGEdge(src, dst);
        }

        if (LoadSVFGNode* loadNode = SVFUtil::dyn_cast<LoadSVFGNode>(node))
        {
            auto svfgID = loadNode->getId();
            auto src = loadNode->getPAGSrcNode()->getId();
            if (!hasConstraintNode(src))
            {
                totalCGNode++;
                idTopairMap[src] = NodePair(src, svfgID);
                pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = loadNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                totalCGNode++;
                idTopairMap[dst] = NodePair(dst, svfgID);
                pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            addLoadCGEdge(src, dst, svfgID);
        }

        if (StoreSVFGNode* storeNode = SVFUtil::dyn_cast<StoreSVFGNode>(node))
        {
            auto svfgID = storeNode->getId();
            auto src = storeNode->getPAGSrcNode()->getId();
            if (!hasConstraintNode(src))
            {
                totalCGNode++;
                idTopairMap[src] = NodePair(src, svfgID);
                pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = storeNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                totalCGNode++;
                idTopairMap[dst] = NodePair(dst, svfgID);
                pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            addStoreCGEdge(src, dst, svfgID);
        }

        if (GepSVFGNode* gepNode = SVFUtil::dyn_cast<GepSVFGNode>(node))
        {
            auto svfgID = gepNode->getId();
            auto src = gepNode->getPAGSrcNode()->getId();
            if (!hasConstraintNode(src))
            {
                totalCGNode++;
                idTopairMap[src] = NodePair(src, svfgID);
                pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = gepNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                totalCGNode++;
                idTopairMap[dst] = NodePair(dst, svfgID);
                pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            const GepStmt* edge = SVFUtil::cast<GepStmt>(gepNode->getPAGEdge());
            if (edge->isVariantFieldGep())
            {
                addVariantGepCGEdge(src, dst);
            }
            else
            {
                addNormalGepCGEdge(src, dst, edge->getAccessPath());
            }
        }

        if (IntraPHISVFGNode* phiNode =
                SVFUtil::dyn_cast<IntraPHISVFGNode>(node))
        {
            auto svfgID = phiNode->getId();
            auto dst = phiNode->getRes()->getId();
            if (!hasConstraintNode(dst))
            {
                totalCGNode++;
                idTopairMap[dst] = NodePair(dst, svfgID);
                pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            for (Map<u32_t, const PAGNode*>::const_iterator
                     iter = phiNode->opVerBegin(),
                     eiter = phiNode->opVerEnd();
                 iter != eiter; ++iter)
            {
                auto src = iter->second->getId();
                if (!hasConstraintNode(src))
                {
                    totalCGNode++;
                    idTopairMap[src] = NodePair(src, svfgID);
                    pairToidMap[NodePair(src, svfgID)] = src;
                    addConstraintNode(new ConstraintNode(src), src);
                }
                addCopyCGEdge(src, dst);
            }
        }

        /// top-level call and ret
        if (ActualParmSVFGNode* aparm =
                SVFUtil::dyn_cast<ActualParmSVFGNode>(node))
        {
            auto svfgID = aparm->getId();
            auto node = aparm->getParam()->getId();
            if (!hasConstraintNode(node))
            {
                totalCGNode++;
                idTopairMap[node] = NodePair(node, svfgID);
                pairToidMap[NodePair(node, svfgID)] = node;
                addConstraintNode(new ConstraintNode(node), node);
            }
        }

        if (FormalParmSVFGNode* fparm =
                SVFUtil::dyn_cast<FormalParmSVFGNode>(node))
        {
            auto svfgID = fparm->getId();
            auto node = fparm->getParam()->getId();
            if (!hasConstraintNode(node))
            {
                totalCGNode++;
                idTopairMap[node] = NodePair(node, svfgID);
                pairToidMap[NodePair(node, svfgID)] = node;
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
            auto svfgID = aret->getId();
            auto node = aret->getRev()->getId();
            if (!hasConstraintNode(node))
            {
                totalCGNode++;
                idTopairMap[node] = NodePair(node, svfgID);
                pairToidMap[NodePair(node, svfgID)] = node;
                addConstraintNode(new ConstraintNode(node), node);
            }
        }

        if (FormalRetVFGNode* fret = SVFUtil::dyn_cast<FormalRetVFGNode>(node))
        {
            auto svfgID = fret->getId();
            auto node = fret->getRet()->getId();
            if (!hasConstraintNode(node))
            {
                totalCGNode++;
                idTopairMap[node] = NodePair(node, svfgID);
                pairToidMap[NodePair(node, svfgID)] = node;
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
            auto svfgID = nptr->getId();
            auto node = nptr->getPAGNode()->getId();
            if (!hasConstraintNode(node))
            {
                totalCGNode++;
                idTopairMap[node] = NodePair(node, svfgID);
                pairToidMap[NodePair(node, svfgID)] = node;
                addConstraintNode(new ConstraintNode(node), node);
            }
        }

        /*
            if (StmtVFGNode* stmtNode = SVFUtil::dyn_cast<StmtVFGNode>(node))
            {
                NodeID addFSCGNode(svfgNodeID, pagNodeID)
                {
                    topLevel:
                    if (hasFSCGNode(getdef, pagNodeID))
                        return;
                    totalCGNode++;
                    addConstraintNode(new ConstraintNode(totalCGNode),
                totalCGNode); PairToIDMap[pair] = id; IDToPairMap[id] = pair;
                    return totalCGNode;

                    pagIDTosvfgIDMap;



                    ID(pagID (top) / totalCGNodeID (addr));
                    Pair<pagID , svfgID >;

                    addrTaken:
                    if (hasFSCGNode(cgID, svfgID))
                        return;
                    totalCGNode++;
                    addConstraintNode(new ConstraintNode(totalCGNode),
                totalCGNode); PairToIDMap[pair] = id; IDToPairMap[id] = pair;
                    return totalCGNode;
                }

                src = addFSCFNode(...);
                dst = addFSCGNode(..);
                addCopyCGEdge(src,dst);

                addFSNode
                totalCGNode++;

            }
        */
    }

    /// build address-taken constraint nodes
    for (auto it : *svfg)
    {
        SVFGNode* node = it.second;

        /// address-taken call and ret
        if (MSSAPHISVFGNode* mphiNode =
                SVFUtil::dyn_cast<MSSAPHISVFGNode>(node))
        {
            totalCGNode++;
            auto svfgID = mphiNode->getId();
            auto fdst = mphiNode->getResVer()->getID();
            auto dst = totalCGNode;
            if (!hasConstraintNode(dst))
            {
                idTopairMap[dst] = NodePair(fdst, svfgID);
                pairToidMap[NodePair(fdst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            for (auto iter = mphiNode->opVerBegin(),
                      eiter = mphiNode->opVerEnd();
                 iter != eiter; ++iter)
            {
                totalCGNode++;
                auto fsrc = iter->first;
                auto src = totalCGNode;
                if (!hasConstraintNode(src))
                {
                    idTopairMap[src] = NodePair(fsrc, svfgID);
                    pairToidMap[NodePair(fsrc, svfgID)] = src;
                    addConstraintNode(new ConstraintNode(src), src);
                }
                addCopyCGEdge(src, dst);
            }
        }

        if (ActualINSVFGNode* ain = SVFUtil::dyn_cast<ActualINSVFGNode>(node))
        {
            auto svfgID = ain->getId();
            auto node = ain->getMRVer()->getMR()->getPointsTo();
            for (auto i : node)
            {
                // std::ignore = i;
                totalCGNode++;
                if (!hasConstraintNode(totalCGNode))
                {
                    idTopairMap[totalCGNode] = NodePair(i, svfgID);
                    pairToidMap[NodePair(i, svfgID)] = totalCGNode;
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                    // addAddrCGEdge(totalCGNode, i);
                }
            }
        }

        if (FormalINSVFGNode* fparm = SVFUtil::dyn_cast<FormalINSVFGNode>(node))
        {
            auto svfgID = fparm->getId();
            auto node = fparm->getMRVer()->getMR()->getPointsTo();
            for (auto i : node)
            {
                // std::ignore = i;
                totalCGNode++;
                if (!hasConstraintNode(totalCGNode))
                {
                    idTopairMap[totalCGNode] = NodePair(i, svfgID);
                    pairToidMap[NodePair(i, svfgID)] = totalCGNode;
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                    // addAddrCGEdge(totalCGNode, i);
                }
            }
        }

        if (ActualOUTSVFGNode* aret =
                SVFUtil::dyn_cast<ActualOUTSVFGNode>(node))
        {
            auto svfgID = aret->getId();
            auto node = aret->getMRVer()->getMR()->getPointsTo();
            for (auto i : node)
            {
                // std::ignore = i;
                totalCGNode++;
                if (!hasConstraintNode(totalCGNode))
                {
                    idTopairMap[totalCGNode] = NodePair(i, svfgID);
                    pairToidMap[NodePair(i, svfgID)] = totalCGNode;
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                    // addAddrCGEdge(totalCGNode, i);
                }
            }
        }

        if (FormalOUTSVFGNode* fret =
                SVFUtil::dyn_cast<FormalOUTSVFGNode>(node))
        {
            auto svfgID = fret->getId();
            auto node = fret->getMRVer()->getMR()->getPointsTo();
            for (auto i : node)
            {
                // std::ignore = i;
                totalCGNode++;
                if (!hasConstraintNode(totalCGNode))
                {
                    idTopairMap[totalCGNode] = NodePair(i, svfgID);
                    pairToidMap[NodePair(i, svfgID)] = totalCGNode;
                    addConstraintNode(new ConstraintNode(totalCGNode), totalCGNode);
                    // addAddrCGEdge(totalCGNode, i);
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

/*
FSLoadCGEdge* FSConsG::addFSLoadCGEdge(NodeID src, NodeID dst, NodeID fsID)
{
    ConstraintNode* srcNode = getConstraintNode(src);
    ConstraintNode* dstNode = getConstraintNode(dst);
    if (hasEdge(srcNode, dstNode, ConstraintEdge::Load))
        return nullptr;

    FSLoadCGEdge* edge = new FSLoadCGEdge(srcNode, dstNode, edgeIndex++, fsID);

    bool inserted = LoadCGEdgeSet.insert(edge).second;
    (void)inserted; // Suppress warning of unused variable under release build
    assert(inserted && "new FSLoadCGEdge not added??");

    srcNode->addOutgoingLoadEdge(edge);
    dstNode->addIncomingLoadEdge(edge);
    return edge;
}

FSStoreCGEdge* FSConsG::addFSStoreCGEdge(NodeID src, NodeID dst, NodeID fsID)
{
    ConstraintNode* srcNode = getConstraintNode(src);
    ConstraintNode* dstNode = getConstraintNode(dst);
    if (hasEdge(srcNode, dstNode, ConstraintEdge::Store))
        return nullptr;

    FSStoreCGEdge* edge =
        new FSStoreCGEdge(srcNode, dstNode, edgeIndex++, fsID);

    bool inserted = StoreCGEdgeSet.insert(edge).second;
    (void)inserted; // Suppress warning of unused variable under release build
    assert(inserted && "new StoreCGEdge not added??");

    srcNode->addOutgoingStoreEdge(edge);
    dstNode->addIncomingStoreEdge(edge);
    return edge;
}
*/
