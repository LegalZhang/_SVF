
#ifndef INCLUDE_CFL_CFLALIAS_H_
#define INCLUDE_CFL_CFLALIAS_H_

#include "CFL/CFLBase.h"
#include "CFL/CFLStat.h"

namespace SVF
{

class CFLStat;

class CFLAlias : public CFLBase
{

public:
    typedef OrderedMap<const CallICFGNode*, NodeID> CallSite2DummyValPN;

    CFLAlias(SVFIR* ir) : CFLBase(ir, PointerAnalysis::CFLFICI_WPA)
    {
    }

    /// Initialize the grammar, graph, solver
    virtual void initialize();

    /// Initialize Solver
    virtual void initializeSolver();


    /// Print grammar and graph
    virtual void finalize();

    /// Solving CFL Reachability
    virtual void solve();

    /// Interface exposed to users of our Alias analysis, given PAGNodeID
    virtual AliasResult alias(NodeID node1, NodeID node2)
    {
        if(graph->hasEdge(graph->getGNode(node1), graph->getGNode(node2), graph->startKind))
            return AliasResult::MayAlias;
        else
            return AliasResult::NoAlias;
    }

    /// Get points-to targets of a pointer.  V In this context
    virtual const PointsTo& getCFLPts(NodeID ptr)
    {
        /// Check V Dst of ptr.
        CFLNode *funNode = graph->getGNode(ptr);
        for(auto outedge = funNode->getOutEdges().begin(); outedge!=funNode->getOutEdges().end(); outedge++)
        {
            if((*outedge)->getEdgeKind() == graph->getStartKind())
            {
                // Need to Find dst addr src
                SVFVar *vNode = svfir->getGNode((*outedge)->getDstID());
                NodeID basevNodeID;
                // Remove svfir->getBaseValVar, SVF IR api change
                if (vNode->hasIncomingEdges(SVFStmt::Gep))
                {
                    SVFStmt::SVFStmtSetTy& geps = vNode->getIncomingEdges(SVFStmt::Gep);
                    SVFVar::iterator it = geps.begin();
                    basevNodeID = (*it)->getSrcID();
                }
                else
                    basevNodeID = vNode->getId();
                addPts(ptr, basevNodeID);
                for(auto inEdge = vNode->getInEdges().begin(); inEdge!=vNode->getInEdges().end(); inEdge++)
                {
                    if((*inEdge)->getEdgeKind() == 0)
                    {
                        addPts(ptr, (*inEdge)->getSrcID());
                    }
                }
            }
        }
        return getPts(ptr);
    }

    /// Need Original one for virtual table

    /// Add copy edge on constraint graph
    virtual inline bool addCopyEdge(NodeID src, NodeID dst)
    {
        const CFLEdge *edge = graph->hasEdge(graph->getGNode(src),graph->getGNode(dst), 1);
        if (edge != nullptr )
        {
            return false;
        }
        CFGrammar::Kind copyKind = grammar->strToKind("copy");
        CFGrammar::Kind copybarKind = grammar->strToKind("copybar");
        solver->pushIntoWorklist(graph->addCFLEdge(graph->getGNode(src),graph->getGNode(dst), copyKind));
        solver->pushIntoWorklist(graph->addCFLEdge(graph->getGNode(dst),graph->getGNode(src), copybarKind));
        return true;
    }

    /// Given an object, get all the nodes having whose pointsto contains the object
    virtual const NodeSet& getRevPts(NodeID nodeId)
    {
        /// Check Outgoing flowtobar edge dst of ptr
        abort(); // to be implemented
    }

    /// Update call graph for the input indirect callsites
    virtual bool updateCallGraph(const CallSiteToFunPtrMap& callsites);

    /// On the fly call graph construction
    virtual void onTheFlyCallGraphSolve(const CallSiteToFunPtrMap& callsites, CallEdgeMap& newEdges);

    /// Connect formal and actual parameters for indirect callsites
    void connectCaller2CalleeParams(const CallICFGNode* cs, const FunObjVar* F);

    void heapAllocatorViaIndCall(const CallICFGNode* cs);

private:
    CallSite2DummyValPN callsite2DummyValPN;        ///< Map an instruction to a dummy obj which created at an indirect callsite, which invokes a heap allocator
};

class POCRAlias : public CFLAlias
{
public:
    POCRAlias(SVFIR* ir) : CFLAlias(ir)
    {
    }
    /// Initialize POCR Solver
    virtual void initializeSolver();
};

class POCRHybrid : public CFLAlias
{
public:
    POCRHybrid(SVFIR* ir) : CFLAlias(ir)
    {
    }

    /// Initialize POCRHybrid Solver
    virtual void initializeSolver();
};
} // End namespace SVF

#endif /* INCLUDE_CFL_CFLALIAS_H_*/
