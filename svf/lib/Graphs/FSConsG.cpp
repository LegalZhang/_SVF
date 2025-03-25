//
// Created by Jiahao Zhang on 19/8/2024.
//

#include "Graphs/FSConsG.h"

#include "Graphs/ConsG.h"
#include "Graphs/SVFG.h"
#include "Util/Options.h"
#include "WPA/Andersen.h"

using namespace SVF;
using namespace SVFUtil;

/*!
 * Start building constraint graph
 */
void FSConsG::buildSVFG2CG(SVFG* svfg)
{
    totalCGNode = SVFIR::getPAG()->getTotalNodeNum();

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
                insertConstraintMapping(src, svfgID, src);
                // idTopairMap[src] = NodePair(src, svfgID);
                // pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = addrNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                insertConstraintMapping(dst, svfgID, dst);
                // idTopairMap[dst] = NodePair(dst, svfgID);
                // pairToidMap[NodePair(dst, svfgID)] = dst;
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
                insertConstraintMapping(src, svfgID, src);
                // idTopairMap[src] = NodePair(src, svfgID);
                // pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = copyNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                insertConstraintMapping(dst, svfgID, dst);
                // idTopairMap[dst] = NodePair(dst, svfgID);
                // pairToidMap[NodePair(dst, svfgID)] = dst;
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
                insertConstraintMapping(src, svfgID, src);
                // idTopairMap[src] = NodePair(src, svfgID);
                // pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = loadNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                insertConstraintMapping(dst, svfgID, dst);
                // idTopairMap[dst] = NodePair(dst, svfgID);
                // pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            ConstraintEdge* existingEdge = getEdge(getConstraintNode(src), getConstraintNode(dst), ConstraintEdge::Load);
            if (existingEdge) {
                if (LoadCGEdge* loadEdge = SVFUtil::dyn_cast<LoadCGEdge>(existingEdge)) {
                    loadEdge->setSVFGID(svfgID);
                }
            } else {
                addLoadCGEdge(src, dst, svfgID);
            }
        }

        if (StoreSVFGNode* storeNode = SVFUtil::dyn_cast<StoreSVFGNode>(node))
        {
            auto svfgID = storeNode->getId();
            auto src = storeNode->getPAGSrcNode()->getId();
            if (!hasConstraintNode(src))
            {
                insertConstraintMapping(src, svfgID, src);
                // idTopairMap[src] = NodePair(src, svfgID);
                // pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = storeNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                insertConstraintMapping(dst, svfgID, dst);
                // idTopairMap[dst] = NodePair(dst, svfgID);
                // pairToidMap[NodePair(dst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            ConstraintEdge* existingEdge = getEdge(getConstraintNode(src), getConstraintNode(dst), ConstraintEdge::Store);
            if (existingEdge) {
                if (StoreCGEdge* storeEdge = SVFUtil::dyn_cast<StoreCGEdge>(existingEdge)) {
                    storeEdge->setSVFGID(svfgID);
                }
            } else {
                addStoreCGEdge(src, dst, svfgID);
            }
        }

        if (GepSVFGNode* gepNode = SVFUtil::dyn_cast<GepSVFGNode>(node))
        {
            auto svfgID = gepNode->getId();
            auto src = gepNode->getPAGSrcNode()->getId();
            if (!hasConstraintNode(src))
            {
                insertConstraintMapping(src, svfgID, src);
                // idTopairMap[src] = NodePair(src, svfgID);
                // pairToidMap[NodePair(src, svfgID)] = src;
                addConstraintNode(new ConstraintNode(src), src);
            }
            NodeID dst = gepNode->getPAGDstNode()->getId();
            if (!hasConstraintNode(dst))
            {
                insertConstraintMapping(dst, svfgID, dst);
                // idTopairMap[dst] = NodePair(dst, svfgID);
                // pairToidMap[NodePair(dst, svfgID)] = dst;
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
                insertConstraintMapping(dst, svfgID, dst);
                // idTopairMap[dst] = NodePair(dst, svfgID);
                // pairToidMap[NodePair(dst, svfgID)] = dst;
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
                    insertConstraintMapping(src, svfgID, src);
                    // idTopairMap[src] = NodePair(src, svfgID);
                    // pairToidMap[NodePair(src, svfgID)] = src;
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
            auto apnode = aparm->getParam()->getId();
            if (!hasConstraintNode(apnode))
            {
                insertConstraintMapping(apnode, svfgID, apnode);
                // idTopairMap[apnode] = NodePair(apnode, svfgID);
                // pairToidMap[NodePair(apnode, svfgID)] = apnode;
                addConstraintNode(new ConstraintNode(apnode), apnode);
            }
        }

        // if (FormalParmSVFGNode* fparm =
        //         SVFUtil::dyn_cast<FormalParmSVFGNode>(node))
        // {
        //     auto svfgID = fparm->getId();
        //     auto fpnode = fparm->getParam()->getId();
        //     if (!hasConstraintNode(fpnode))
        //     {
        //         insertConstraintMapping(fpnode, svfgID, fpnode);
        //         // idTopairMap[fpnode] = NodePair(fpnode, svfgID);
        //         // pairToidMap[NodePair(fpnode, svfgID)] = fpnode;
        //         addConstraintNode(new ConstraintNode(fpnode), fpnode);
        //     }
        //     auto i = 0;
        //     for (auto iter = fparm->callPEBegin(), eiter = fparm->callPEEnd();
        //          iter != eiter; ++iter, i++)
        //     {
        //         auto* callPE = *iter;
        //         auto callICFGNode = callPE->getCallSite();
        //         // auto funEntryICFGNode = callPE->getFunEntryICFGNode();
        //         auto src = callICFGNode->getActualParms();
        //         auto id = src[i]->getId();
        //         addCopyCGEdge(id, fpnode);
        //     }
        // }

        if (ActualRetVFGNode* aret = SVFUtil::dyn_cast<ActualRetVFGNode>(node))
        {
            auto svfgID = aret->getId();
            auto arnode = aret->getRev()->getId();
            if (!hasConstraintNode(arnode))
            {
                insertConstraintMapping(arnode, svfgID, arnode);
                // idTopairMap[arnode] = NodePair(arnode, svfgID);
                // pairToidMap[NodePair(arnode, svfgID)] = arnode;
                addConstraintNode(new ConstraintNode(arnode), arnode);
            }
        }

        if (FormalRetVFGNode* fret = SVFUtil::dyn_cast<FormalRetVFGNode>(node))
        {
            auto svfgID = fret->getId();
            auto frnode = fret->getRet()->getId();
            if (!hasConstraintNode(frnode))
            {
                insertConstraintMapping(frnode, svfgID, frnode);
                // idTopairMap[frnode] = NodePair(frnode, svfgID);
                // pairToidMap[NodePair(frnode, svfgID)] = frnode;
                addConstraintNode(new ConstraintNode(frnode), frnode);
            }

            for (auto iter = fret->retPEBegin(), eiter = fret->retPEEnd();
                 iter != eiter; ++iter)
            {
                auto* RetPE = *iter;
                auto callICFGNode = RetPE->getCallSite();
                // auto funEntryICFGNode = RetPE->getFunExitICFGNode();
                auto src = callICFGNode->getRetICFGNode()->getActualRet();
                auto id = src->getId();
                addCopyCGEdge(id, frnode);
            }
        }

        if (NullPtrVFGNode* nptr = SVFUtil::dyn_cast<NullPtrVFGNode>(node))
        {
            auto svfgID = nptr->getId();
            auto node = nptr->getPAGNode()->getId();
            if (!hasConstraintNode(node))
            {
                insertConstraintMapping(node, svfgID, node);
                // idTopairMap[node] = NodePair(node, svfgID);
                // pairToidMap[NodePair(node, svfgID)] = node;
                addConstraintNode(new ConstraintNode(node), node);
            }
        }

        /// address-taken phi
        if (MSSAPHISVFGNode* mphiNode =
                SVFUtil::dyn_cast<MSSAPHISVFGNode>(node))
        {
            totalCGNode++;
            auto svfgID = mphiNode->getId();
            NodeID fdst = mphiNode->getResVer()->getID();
            NodeID dst = totalCGNode;
            if (!hasConstraintNode(dst))
            {
                insertConstraintMapping(fdst, svfgID, dst);
                // idTopairMap[dst] = NodePair(fdst, svfgID);
                // pairToidMap[NodePair(fdst, svfgID)] = dst;
                addConstraintNode(new ConstraintNode(dst), dst);
            }
            for (auto iter = mphiNode->opVerBegin(),
                      eiter = mphiNode->opVerEnd();
                 iter != eiter; ++iter)
            {
                totalCGNode++;
                NodeID fsrc = iter->first;
                NodeID src = totalCGNode;
                if (!hasConstraintNode(src))
                {
                    insertConstraintMapping(fsrc, svfgID, src);
                    // idTopairMap[src] = NodePair(fsrc, svfgID);
                    // pairToidMap[NodePair(fsrc, svfgID)] = src;
                    addConstraintNode(new ConstraintNode(src), src);
                }
                addCopyCGEdge(src, dst);
            }
        }

    }

    /// connect indirect constraint edges
    for (IndirectSVFGEdge* edge : svfg->indirectEdgeSet)
    {
        auto srcIndirect = edge->getSrcID(); // SrcSVFGID
        auto dstIndirect = edge->getDstID(); // DstSVFGID
        for (NodeID i : edge->getPointsTo())
        {
            if (!hasConstraintNodePair(i, srcIndirect))
            {
                totalCGNode++;
                NodeID src = totalCGNode;
                auto srcsvfgID = srcIndirect;
                insertConstraintMapping(i, srcsvfgID, src);
                // pairToidMap[NodePair(i, srcsvfgID)] = src;
                // idTopairMap[src] = NodePair(i, srcsvfgID);
                // SVFGNode* srcNodeType = svfg->getSVFGNode(srcsvfgID);
                // initialPts(src);
                addConstraintNode(new ConstraintNode(src), src);
            }

            if (!hasConstraintNodePair(i, dstIndirect))
            {
                totalCGNode++;
                NodeID dst = totalCGNode;
                auto dstsvfgID = dstIndirect;
                insertConstraintMapping(i, dstsvfgID, dst);
                // pairToidMap[NodePair(i, dstsvfgID)] = dst;
                // idTopairMap[dst] = NodePair(i, dstsvfgID);
                // SVFGNode* dstNodeType = svfg->getSVFGNode(dstsvfgID);
                // initialPts(dst);
                addConstraintNode(new ConstraintNode(dst), dst);
            }

            NodeID srcNode = pairToidMap[NodePair(i, srcIndirect)];
            NodeID dstNode = pairToidMap[NodePair(i, dstIndirect)];
            addCopyCGEdge(srcNode, dstNode);
        }
    }

}

/*!
 * Add Load edge

LoadCGEdge* ConstraintGraph::addLoadCGEdge(NodeID src, NodeID dst, NodeID fsID)
{
    ConstraintNode* srcNode = getConstraintNode(src);
    ConstraintNode* dstNode = getConstraintNode(dst);
    if (hasEdge(srcNode, dstNode, ConstraintEdge::Load))
        return nullptr;

    LoadCGEdge* edge = new LoadCGEdge(srcNode, dstNode, edgeIndex++, fsID);

    bool inserted = LoadCGEdgeSet.insert(edge).second;
    (void)inserted; // Suppress warning of unused variable under release build
    assert(inserted && "new LoadCGEdge not added??");

    srcNode->addOutgoingLoadEdge(edge);
    dstNode->addIncomingLoadEdge(edge);
    return edge;
}
*/

/*!
 * Add Store edge

StoreCGEdge* ConstraintGraph::addStoreCGEdge(NodeID src, NodeID dst, NodeID fsID)
{
    ConstraintNode* srcNode = getConstraintNode(src);
    ConstraintNode* dstNode = getConstraintNode(dst);
    if (hasEdge(srcNode, dstNode, ConstraintEdge::Store))
        return nullptr;

    StoreCGEdge* edge = new StoreCGEdge(srcNode, dstNode, edgeIndex++, fsID);

    bool inserted = StoreCGEdgeSet.insert(edge).second;
    (void)inserted; // Suppress warning of unused variable under release build
    assert(inserted && "new StoreCGEdge not added??");

    srcNode->addOutgoingStoreEdge(edge);
    dstNode->addIncomingStoreEdge(edge);
    return edge;
}
*/

/*!
 * Dump flow-sensitive constraint graph
 */
void FSConsG::dump(std::string name)
{
    GraphPrinter::WriteGraphToFile(outs(), name, this);
}

/*!
 * GraphTraits specialization for flow-sensitive constraint graph
 */
namespace SVF
{
template<>
struct DOTGraphTraits<FSConsG*> : public DOTGraphTraits<SVFIR*>
{

    typedef ConstraintNode NodeType;
    DOTGraphTraits(bool isSimple = false) :
        DOTGraphTraits<SVFIR*>(isSimple)
    {
    }

    /// Return name of the graph
    static std::string getGraphName(FSConsG*)
    {
        return "Flow-Sensitive ConstraintG";
    }

    static bool isNodeHidden(NodeType *n, FSConsG *)
    {
        if (Options::ShowHiddenNode()) return false;
        else return (n->getInEdges().empty() && n->getOutEdges().empty());
    }

    /// Return label of a VFG node with two display mode
    /// Either you can choose to display the name of the value or the whole instruction
    static std::string getNodeLabel(NodeType *n, FSConsG *fs)
    {
        ConstraintNode* node = fs->getConstraintNode(n->getId());
        NodeID pagNodeID = fs->getPAGNodeID(n->getId());
        PAGNode* pagNode = SVFIR::getPAG()->getGNode(pagNodeID);
        bool briefDisplay = Options::BriefConsCGDotGraph();
        bool nameDisplay = true;
        std::string str;
        std::stringstream rawstr(str);

        if (briefDisplay)
        {
            if (SVFUtil::isa<ValVar>(pagNode))
            {
                if (nameDisplay)
                    rawstr << node->getId() << ":" << pagNode->getValueName();
                else
                    rawstr << node->getId();
            }
            else
                rawstr << node->getId();
        }
        else
        {
            // print the whole value
            if (!SVFUtil::isa<DummyValVar>(pagNode) && !SVFUtil::isa<DummyObjVar>(pagNode))
                rawstr << node->toString();
            else
                rawstr << node->getId() << ":";

        }

        return rawstr.str();
    }

    static std::string getNodeAttributes(NodeType *n, FSConsG* fs)
    {
        // ConstraintNode* node = fs->getConstraintNode(n->getId());
        NodeID pagNodeID = fs->getPAGNodeID(n->getId());
        PAGNode* node = SVFIR::getPAG()->getGNode(pagNodeID);
        if (SVFUtil::isa<ValVar>(node))
        {
            if(SVFUtil::isa<GepValVar>(node))
                return "shape=hexagon";
            else if (SVFUtil::isa<DummyValVar>(node))
                return "shape=diamond";
            else
                return "shape=box";
        }
        else if (SVFUtil::isa<ObjVar>(node))
        {
            if(SVFUtil::isa<GepObjVar>(node))
                return "shape=doubleoctagon";
            else if(SVFUtil::isa<BaseObjVar>(node))
                return "shape=box3d";
            else if (SVFUtil::isa<DummyObjVar>(node))
                return "shape=tab";
            else
                return "shape=component";
        }
        else if (SVFUtil::isa<RetValPN>(node))
        {
            return "shape=Mrecord";
        }
        else if (SVFUtil::isa<VarArgValPN>(node))
        {
            return "shape=octagon";
        }
        else
        {
            assert(0 && "no such kind!!");
        }
        return "";
    }

    template<class EdgeIter>
    static std::string getEdgeAttributes(NodeType*, EdgeIter EI, FSConsG* fs)
    {
        ConstraintEdge* edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");
        if (edge->getEdgeKind() == ConstraintEdge::Addr)
        {
            return "color=green";
        }
        else if (edge->getEdgeKind() == ConstraintEdge::Copy)
        {
            return "color=black";
        }
        else if (edge->getEdgeKind() == ConstraintEdge::NormalGep
                 || edge->getEdgeKind() == ConstraintEdge::VariantGep)
        {
            return "color=purple";
        }
        else if (edge->getEdgeKind() == ConstraintEdge::Store)
        {
            return "color=blue";
        }
        else if (edge->getEdgeKind() == ConstraintEdge::Load)
        {
            return "color=red";
        }
        else
        {
            assert(0 && "No such kind edge!!");
        }
        return "";
    }

    template<class EdgeIter>
    static std::string getEdgeSourceLabel(NodeType*, EdgeIter)
    {
        return "";
    }
};
} // End namespace llvm

    /* function-related address-taken nodes
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
                    addConstraintNode(new ConstraintNode(totalCGNode),
    totalCGNode);
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
                    addConstraintNode(new ConstraintNode(totalCGNode),
    totalCGNode);
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
                    addConstraintNode(new ConstraintNode(totalCGNode),
    totalCGNode);
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
                    addConstraintNode(new ConstraintNode(totalCGNode),
    totalCGNode);
                    // addAddrCGEdge(totalCGNode, i);
                }
            }
        }
    }
    */

    /* add chi/mu address-taken nodes

     getinComingIndirectEdges(indirectEdges);
     srcIndirect = inComingIndirectEdges.getSrcID();
     dstIndirect = inComingIndirectEdges.getDstID();

     for each srcIndirect
        for each i = srcIndirect.getPtsTo(); {
            totalCGNode++;
            src = totalCGNode;
            idTopairMap[src] = NodePair(i, svfgID);
            pairToidMap[NodePair(i, svfgID)] = src;
            if(srcNodeType == Store)
                addConstraintNode(new ConstraintNode(src), src);
                initialPts(src);
            if(srcNodeType == Load)
                addConstraintNode(new ConstraintNode(src), src);
                addCopyEdge(previous, src);
        }

     for each dstIndirect
        for each i = dstIndirect.getPtsTo(); {
            totalCGNode++;
            dst = totalCGNode;
            idTopairMap[dst] = NodePair(i, svfgID);
            pairToidMap[NodePair(i, svfgID)] = dst;
            if(dstNodeType == Store)
                addConstraintNode(new ConstraintNode(dst), dst);
                initialPts(dst);
            if(dstNodeType == Load)
                addConstraintNode(new ConstraintNode(dst), dst);
                addCopyEdge(previous, dst);
        }

     */
