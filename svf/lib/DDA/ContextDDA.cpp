
#include "Util/Options.h"
#include "DDA/ContextDDA.h"
#include "DDA/FlowDDA.h"
#include "DDA/DDAClient.h"
#include "MemoryModel/PointsTo.h"

using namespace SVF;
using namespace SVFUtil;

/*!
 * Constructor
 */
ContextDDA::ContextDDA(SVFIR* _pag,  DDAClient* client)
    : CondPTAImpl<ContextCond>(_pag, PointerAnalysis::Cxt_DDA),DDAVFSolver<CxtVar,CxtPtSet,CxtLocDPItem>(),
      _client(client)
{
    flowDDA = new FlowDDA(_pag, client);
}

/*!
 * Destructor
 */
ContextDDA::~ContextDDA()
{
    if(flowDDA)
        delete flowDDA;
    flowDDA = nullptr;
}

/*!
 * Analysis initialization
 */
void ContextDDA::initialize()
{
    CondPTAImpl<ContextCond>::initialize();
    buildSVFG(pag);
    setCallGraph(getCallGraph());
    setCallGraphSCC(getCallGraphSCC());
    stat = setDDAStat(new DDAStat(this));
    flowDDA->initialize();
}

/*!
 * Compute points-to set for a context-sensitive pointer
 */
const CxtPtSet& ContextDDA::computeDDAPts(const CxtVar& var)
{

    resetQuery();
    LocDPItem::setMaxBudget(Options::CxtBudget());

    NodeID id = var.get_id();
    PAGNode* node = getPAG()->getGNode(id);
    CxtLocDPItem dpm = getDPIm(var, getDefSVFGNode(node));

    // start DDA analysis
    DOTIMESTAT(double start = DDAStat::getClk(true));
    const CxtPtSet& cpts = findPT(dpm);
    DOTIMESTAT(ddaStat->_AnaTimePerQuery = DDAStat::getClk(true) - start);
    DOTIMESTAT(ddaStat->_TotalTimeOfQueries += ddaStat->_AnaTimePerQuery);

    if(isOutOfBudgetQuery() == false)
        unionPts(var,cpts);
    else
        handleOutOfBudgetDpm(dpm);

    if (this->printStat())
        DOSTAT(stat->performStatPerQuery(id));
    DBOUT(DGENERAL, stat->printStatPerQuery(id,getBVPointsTo(getPts(var))));
    return this->getPts(var);
}

/*!
 *  Compute points-to set for an unconditional pointer
 */
void ContextDDA::computeDDAPts(NodeID id)
{
    ContextCond cxt;
    CxtVar var(cxt, id);
    computeDDAPts(var);
}

/*!
 * Handle out-of-budget dpm
 */
void ContextDDA::handleOutOfBudgetDpm(const CxtLocDPItem& dpm)
{

    DBOUT(DGENERAL,outs() << "~~~Out of budget query, downgrade to flow sensitive analysis \n");
    flowDDA->computeDDAPts(dpm.getCurNodeID());
    const PointsTo& flowPts = flowDDA->getPts(dpm.getCurNodeID());
    CxtPtSet cxtPts;
    for(PointsTo::iterator it = flowPts.begin(), eit = flowPts.end(); it!=eit; ++it)
    {
        ContextCond cxt;
        CxtVar var(cxt, *it);
        cxtPts.set(var);
    }
    updateCachedPointsTo(dpm,cxtPts);
    unionPts(dpm.getCondVar(),cxtPts);
    addOutOfBudgetDpm(dpm);
}

/*!
 * context conditions of local(not in recursion)  and global variables are compatible
 */
bool ContextDDA::isCondCompatible(const ContextCond& cxt1, const ContextCond& cxt2, bool singleton) const
{
    if(singleton)
        return true;

    int i = cxt1.cxtSize() - 1;
    int j = cxt2.cxtSize() - 1;
    for(; i >= 0 && j>=0; i--, j--)
    {
        if(cxt1[i] != cxt2[j])
            return false;
    }
    return true;
}

/*!
 * Generate field objects for structs
 */
CxtPtSet ContextDDA::processGepPts(const GepSVFGNode* gep, const CxtPtSet& srcPts)
{
    CxtPtSet tmpDstPts;
    for (CxtPtSet::iterator piter = srcPts.begin(); piter != srcPts.end(); ++piter)
    {

        CxtVar ptd = *piter;
        if (isBlkObjOrConstantObj(ptd.get_id()))
            tmpDstPts.set(ptd);
        else
        {
            const GepStmt* gepStmt = SVFUtil::cast<GepStmt>(gep->getPAGEdge());
            if (gepStmt->isVariantFieldGep())
            {
                setObjFieldInsensitive(ptd.get_id());
                CxtVar var(ptd.get_cond(),getFIObjVar(ptd.get_id()));
                tmpDstPts.set(var);
            }
            else
            {
                CxtVar var(ptd.get_cond(),getGepObjVar(ptd.get_id(),
                                                       gepStmt->getAccessPath().getConstantStructFldIdx()));
                tmpDstPts.set(var);
            }
        }
    }

    DBOUT(DDDA, outs() << "\t return created gep objs ");
    DBOUT(DDDA, outs() << srcPts.toString());
    DBOUT(DDDA, outs() << " --> ");
    DBOUT(DDDA, outs() << tmpDstPts.toString());
    DBOUT(DDDA, outs() << "\n");
    return tmpDstPts;
}

bool ContextDDA::testIndCallReachability(CxtLocDPItem& dpm, const FunObjVar* callee, const CallICFGNode* cs)
{
    if(getPAG()->isIndirectCallSites(cs))
    {
        NodeID id = getPAG()->getFunPtr(cs);
        PAGNode* node = getPAG()->getGNode(id);
        CxtVar funptrVar(dpm.getCondVar().get_cond(), id);
        CxtLocDPItem funptrDpm = getDPIm(funptrVar,getDefSVFGNode(node));
        PointsTo pts = getBVPointsTo(findPT(funptrDpm));
        if(pts.test(callee->getId()))
            return true;
        else
            return false;
    }
    return true;
}

/*!
 * get callsite id from call, return 0 if it is a spurious call edge
 * translate the callsite id from pre-computed callgraph on SVFG to the one on current callgraph
 */
CallSiteID ContextDDA::getCSIDAtCall(CxtLocDPItem&, const SVFGEdge* edge)
{

    CallSiteID svfg_csId = 0;
    if (const CallDirSVFGEdge* callEdge = SVFUtil::dyn_cast<CallDirSVFGEdge>(edge))
        svfg_csId = callEdge->getCallSiteId();
    else
        svfg_csId = SVFUtil::cast<CallIndSVFGEdge>(edge)->getCallSiteId();

    const CallICFGNode* cbn = getSVFG()->getCallSite(svfg_csId);
    const FunObjVar* callee = edge->getDstNode()->getFun();

    if(getCallGraph()->hasCallSiteID(cbn,callee))
    {
        return getCallGraph()->getCallSiteID(cbn,callee);
    }

    return 0;
}

/*!
 * get callsite id from return, return 0 if it is a spurious return edge
 * translate the callsite id from pre-computed callgraph on SVFG to the one on current callgraph
 */
CallSiteID ContextDDA::getCSIDAtRet(CxtLocDPItem&, const SVFGEdge* edge)
{

    CallSiteID svfg_csId = 0;
    if (const RetDirSVFGEdge* retEdge = SVFUtil::dyn_cast<RetDirSVFGEdge>(edge))
        svfg_csId = retEdge->getCallSiteId();
    else
        svfg_csId = SVFUtil::cast<RetIndSVFGEdge>(edge)->getCallSiteId();

    const CallICFGNode* cbn = getSVFG()->getCallSite(svfg_csId);
    const FunObjVar* callee = edge->getSrcNode()->getFun();

    if(getCallGraph()->hasCallSiteID(cbn,callee))
    {
        return getCallGraph()->getCallSiteID(cbn,callee);
    }

    return 0;
}


/// Handle conditions during backward traversing
bool ContextDDA::handleBKCondition(CxtLocDPItem& dpm, const SVFGEdge* edge)
{
    _client->handleStatement(edge->getSrcNode(), dpm.getCurNodeID());

    if (edge->isCallVFGEdge())
    {
        /// we don't handle context in recursions, they treated as assignments
        if(CallSiteID csId = getCSIDAtCall(dpm,edge))
        {

            if(isEdgeInRecursion(csId))
            {
                DBOUT(DDDA,outs() << "\t\t call edge " << getCallGraph()->getCallerOfCallSite(csId)->getName() <<
                      "=>" << getCallGraph()->getCalleeOfCallSite(csId)->getName() << "in recursion \n");
                popRecursiveCallSites(dpm);
            }
            else
            {
                if (dpm.matchContext(csId) == false)
                {
                    DBOUT(DDDA,	outs() << "\t\t context not match, edge "
                          << edge->getDstID() << " --| " << edge->getSrcID() << " \t");
                    DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    return false;
                }

                DBOUT(DDDA, outs() << "\t\t match contexts ");
                DBOUT(DDDA, dumpContexts(dpm.getCond()));
            }
        }
    }

    else if (edge->isRetVFGEdge())
    {
        /// we don't handle context in recursions, they treated as assignments
        if(CallSiteID csId = getCSIDAtRet(dpm,edge))
        {

            if(isEdgeInRecursion(csId))
            {
                DBOUT(DDDA,outs() << "\t\t return edge " << getCallGraph()->getCalleeOfCallSite(csId)->getName() <<
                      "=>" << getCallGraph()->getCallerOfCallSite(csId)->getName() << "in recursion \n");
                popRecursiveCallSites(dpm);
            }
            else
            {
                /// TODO: When this call site id is contained in current call string, we may find a recursion. Try
                ///       to solve this later.
                if (dpm.getCond().containCallStr(csId))
                {
                    outOfBudgetQuery = true;
                    SVFUtil::writeWrnMsg("Call site ID is contained in call string. Is this a recursion?");
                    return false;
                }
                else
                {
                    assert(dpm.getCond().containCallStr(csId) ==false && "contain visited call string ??");
                    if(dpm.pushContext(csId))
                    {
                        DBOUT(DDDA, outs() << "\t\t push context ");
                        DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    }
                    else
                    {
                        DBOUT(DDDA, outs() << "\t\t context is full ");
                        DBOUT(DDDA, dumpContexts(dpm.getCond()));
                    }
                }
            }
        }
    }

    return true;
}


/// we exclude concrete heap given the following conditions:
/// (1) concrete calling context (not involved in recursion and not exceed the maximum context limit)
/// (2) not inside loop
bool ContextDDA::isHeapCondMemObj(const CxtVar& var, const StoreSVFGNode*)
{
    const BaseObjVar* obj = _pag->getBaseObject(getPtrNodeID(var));
    assert(obj && "base object is null??");
    const BaseObjVar* baseVar = _pag->getBaseObject(getPtrNodeID(var));
    assert(baseVar && "base object is null??");
    if (SVFUtil::isa<HeapObjVar, DummyObjVar>(baseVar))
    {
        if (!isa<DummyObjVar>(baseVar))
        {
            PAGNode *pnode = _pag->getGNode(getPtrNodeID(var));
            GepObjVar* gepobj = SVFUtil::dyn_cast<GepObjVar>(pnode);
            if (gepobj != nullptr)
            {
                assert(SVFUtil::isa<DummyObjVar>(_pag->getGNode(gepobj->getBaseNode()))
                       && "empty refVal in a gep object whose base is a non-dummy object");
            }
            else
            {
                assert((SVFUtil::isa<DummyObjVar, DummyValVar>(pnode))
                       && "empty refVal in non-dummy object");
            }
            return true;
        }
        else if(const ICFGNode* node = obj->getICFGNode())
        {
            const FunObjVar* svfFun = node->getFun();
            if(_ander->isInRecursion(svfFun))
                return true;
            if(var.get_cond().isConcreteCxt() == false)
                return true;
            if(_pag->getICFG()->isInLoop(node))
                return true;
        }
    }
    return false;
}
