
#ifndef PROJECT_ANDERSENSFR_H
#define PROJECT_ANDERSENSFR_H


#include "WPA/Andersen.h"
#include "WPA/CSC.h"
#include "MemoryModel/PointsTo.h"

namespace SVF
{

/*!
 * Selective Cycle Detection Based Andersen Analysis
 */
class AndersenSCD : public Andersen
{
public:
    typedef Map<NodeID, NodeID> NodeToNodeMap;

protected:
    static AndersenSCD* scdAndersen;
    NodeSet sccCandidates;
    NodeToNodeMap pwcReps;

public:
    AndersenSCD(SVFIR* _pag, PTATY type = AndersenSCD_WPA) :
        Andersen(_pag,type)
    {
    }

    /// Create an singleton instance directly instead of invoking llvm pass manager
    static AndersenSCD *createAndersenSCD(SVFIR* _pag)
    {
        if (scdAndersen == nullptr)
        {
            scdAndersen = new AndersenSCD(_pag);
            scdAndersen->analyze();
            return scdAndersen;
        }
        return scdAndersen;
    }

    static void releaseAndersenSCD()
    {
        if (scdAndersen)
            delete scdAndersen;
        scdAndersen = nullptr;
    }

protected:
    inline void addSccCandidate(NodeID nodeId)
    {
        sccCandidates.insert(sccRepNode(nodeId));
    }

    virtual NodeStack& SCCDetect();
    virtual void PWCDetect();
    virtual void solveWorklist();
    virtual void handleLoadStore(ConstraintNode* node);
    virtual void processAddr(const AddrCGEdge* addr);
    virtual bool addCopyEdge(NodeID src, NodeID dst);
    virtual bool updateCallGraph(const CallSiteToFunPtrMap& callsites);
    virtual void processPWC(ConstraintNode* rep);
    virtual void handleCopyGep(ConstraintNode* node);

};



/*!
 * Selective Cycle Detection with Stride-based Field Representation
 */
class AndersenSFR : public AndersenSCD
{
public:
    typedef Map<NodeID, NodeBS> NodeStrides;
    typedef Map<NodeID, NodeSet> FieldReps;
    typedef Map<NodeID, std::pair<NodeID, NodeSet>> SFRTrait;

private:
    static AndersenSFR* sfrAndersen;

    CSC* csc;
    NodeSet sfrObjNodes;
    FieldReps fieldReps;

public:
    AndersenSFR(SVFIR* _pag, PTATY type = AndersenSFR_WPA) :
        AndersenSCD(_pag, type), csc(nullptr)
    {
    }

    /// Create an singleton instance directly instead of invoking llvm pass manager
    static AndersenSFR *createAndersenSFR(SVFIR* _pag)
    {
        if (sfrAndersen == nullptr)
        {
            sfrAndersen = new AndersenSFR(_pag);
            sfrAndersen->analyze();
            return sfrAndersen;
        }
        return sfrAndersen;
    }

    static void releaseAndersenSFR()
    {
        if (sfrAndersen)
            delete sfrAndersen;
    }

    ~AndersenSFR()
    {
        if (csc != nullptr)
        {
            delete(csc);
            csc = nullptr;
        }
    }

protected:
    void initialize();
    void PWCDetect();
    void fieldExpand(NodeSet& initials, APOffset offset, NodeBS& strides, PointsTo& expandPts);
    bool processGepPts(const PointsTo& pts, const GepCGEdge* edge);
    bool mergeSrcToTgt(NodeID nodeId, NodeID newRepId);

};

} // End namespace SVF

#endif //PROJECT_ANDERSENSFR_H
