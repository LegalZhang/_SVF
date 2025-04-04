
#ifndef SRCSNKANALYSIS_H_
#define SRCSNKANALYSIS_H_

#include "Graphs/SVFGOPT.h"
#include "SABER/ProgSlice.h"
#include "SABER/SaberSVFGBuilder.h"
#include "Util/GraphReachSolver.h"
#include "Util/SVFBugReport.h"

namespace SVF
{

typedef GraphReachSolver<SVFG*,CxtDPItem> CFLSrcSnkSolver;

/*!
 * General source-sink analysis, which serves as a base analysis to be extended for various clients
 */
class SrcSnkDDA : public CFLSrcSnkSolver
{

public:
    typedef ProgSlice::SVFGNodeSet SVFGNodeSet;
    typedef Map<const SVFGNode*,ProgSlice*> SVFGNodeToSliceMap;
    typedef SVFGNodeSet::const_iterator SVFGNodeSetIter;
    typedef CxtDPItem DPIm;
    typedef Set<DPIm> DPImSet;							///< dpitem set
    typedef Map<const SVFGNode*, DPImSet> SVFGNodeToDPItemsMap; 	///< map a SVFGNode to its visited dpitems
    typedef Set<const CallICFGNode*> CallSiteSet;
    typedef NodeBS SVFGNodeBS;
    typedef ProgSlice::VFWorkList WorkList;

private:
    ProgSlice* _curSlice;		/// current program slice
    SVFGNodeSet sources;		/// source nodes
    SVFGNodeSet sinks;		/// source nodes
    std::unique_ptr<SaberCondAllocator> saberCondAllocator;
    SVFGNodeToDPItemsMap nodeToDPItemsMap;	///<  record forward visited dpitems
    SVFGNodeSet visitedSet;	///<  record backward visited nodes

protected:
    SaberSVFGBuilder memSSA;
    SVFG* svfg;
    CallGraph* callgraph;
    SVFBugReport report; /// Bug Reporter

public:

    /// Constructor
    SrcSnkDDA() : _curSlice(nullptr), svfg(nullptr), callgraph(nullptr)
    {
        saberCondAllocator = std::make_unique<SaberCondAllocator>();
    }
    /// Destructor
    ~SrcSnkDDA() override
    {
        svfg = nullptr;

        delete _curSlice;
        _curSlice = nullptr;

        /// the following shared by multiple checkers, thus can not be released.
        //if (callgraph != nullptr)
        //    delete callgraph;
        //callgraph = nullptr;

        //if(pathCondAllocator)
        //    delete pathCondAllocator;
        //pathCondAllocator = nullptr;
    }

    /// Start analysis here
    virtual void analyze();

    /// Initialize analysis
    virtual void initialize();

    /// Finalize analysis
    virtual void finalize()
    {
        dumpSlices();
    }

    /// Get SVFIR
    SVFIR* getPAG() const
    {
        return SVFIR::getPAG();
    }

    /// Get SVFG
    inline const SVFG* getSVFG() const
    {
        return graph();
    }

    /// Get Callgraph
    inline CallGraph* getCallgraph() const
    {
        return callgraph;
    }

    /// Whether this svfg node may access global variable
    inline bool isGlobalSVFGNode(const SVFGNode* node) const
    {
        return memSSA.isGlobalSVFGNode(node);
    }
    /// Slice operations
    //@{
    virtual void setCurSlice(const SVFGNode* src);

    inline ProgSlice* getCurSlice() const
    {
        return _curSlice;
    }
    inline void addSinkToCurSlice(const SVFGNode* node)
    {
        _curSlice->addToSinks(node);
        addToCurForwardSlice(node);
    }
    inline bool isInCurForwardSlice(const SVFGNode* node)
    {
        return _curSlice->inForwardSlice(node);
    }
    inline bool isInCurBackwardSlice(const SVFGNode* node)
    {
        return _curSlice->inBackwardSlice(node);
    }
    inline void addToCurForwardSlice(const SVFGNode* node)
    {
        _curSlice->addToForwardSlice(node);
    }
    inline void addToCurBackwardSlice(const SVFGNode* node)
    {
        _curSlice->addToBackwardSlice(node);
    }
    //@}

    /// Initialize sources and sinks
    ///@{
    virtual void initSrcs() = 0;
    virtual void initSnks() = 0;
    virtual bool isSourceLikeFun(const FunObjVar* fun)
    {
        return false;
    }

    virtual bool isSinkLikeFun(const FunObjVar* fun)
    {
        return false;
    }

    bool isSource(const SVFGNode* node) const
    {
        return getSources().find(node)!=getSources().end();
    }

    bool isSink(const SVFGNode* node) const
    {
        return getSinks().find(node)!=getSinks().end();
    }
    ///@}

    /// Identify allocation wrappers
    bool isInAWrapper(const SVFGNode* src, CallSiteSet& csIdSet);

    /// report bug on the current analyzed slice
    virtual void reportBug(ProgSlice* slice) = 0;

    /// Get sources/sinks
    //@{
    inline const SVFGNodeSet& getSources() const
    {
        return sources;
    }
    inline SVFGNodeSetIter sourcesBegin() const
    {
        return sources.begin();
    }
    inline SVFGNodeSetIter sourcesEnd() const
    {
        return sources.end();
    }
    inline void addToSources(const SVFGNode* node)
    {
        sources.insert(node);
    }
    inline const SVFGNodeSet& getSinks() const
    {
        return sinks;
    }
    inline SVFGNodeSetIter sinksBegin() const
    {
        return sinks.begin();
    }
    inline SVFGNodeSetIter sinksEnd() const
    {
        return sinks.end();
    }
    inline void addToSinks(const SVFGNode* node)
    {
        sinks.insert(node);
    }
    //@}

    /// Get saber condition allocator
    SaberCondAllocator* getSaberCondAllocator() const
    {
        return saberCondAllocator.get();
    }

    inline const SVFBugReport& getBugReport() const
    {
        return report;
    }

protected:
    /// Forward traverse
    inline void FWProcessCurNode(const DPIm& item) override
    {
        const SVFGNode* node = getNode(item.getCurNodeID());
        if(isSink(node))
        {
            addSinkToCurSlice(node);
            _curSlice->setPartialReachable();
        }
        else
            addToCurForwardSlice(node);
    }
    /// Backward traverse
    inline void BWProcessCurNode(const DPIm& item) override
    {
        const SVFGNode* node = getNode(item.getCurNodeID());
        if(isInCurForwardSlice(node))
        {
            addToCurBackwardSlice(node);
        }
    }
    /// Propagate information forward by matching context
    void FWProcessOutgoingEdge(const DPIm& item, SVFGEdge* edge) override;
    /// Propagate information backward without matching context, as forward analysis already did it
    void BWProcessIncomingEdge(const DPIm& item, SVFGEdge* edge) override;
    /// Whether has been visited or not, in order to avoid recursion on SVFG
    //@{
    inline bool forwardVisited(const SVFGNode* node, const DPIm& item)
    {
        SVFGNodeToDPItemsMap::const_iterator it = nodeToDPItemsMap.find(node);
        if(it!=nodeToDPItemsMap.end())
            return it->second.find(item)!=it->second.end();
        else
            return false;
    }
    inline void addForwardVisited(const SVFGNode* node, const DPIm& item)
    {
        nodeToDPItemsMap[node].insert(item);
    }
    inline bool backwardVisited(const SVFGNode* node)
    {
        return visitedSet.find(node)!=visitedSet.end();
    }
    inline void addBackwardVisited(const SVFGNode* node)
    {
        visitedSet.insert(node);
    }
    inline void clearVisitedMap()
    {
        nodeToDPItemsMap.clear();
        visitedSet.clear();
    }
    //@}

    /// Whether it is all path reachable from a source
    virtual bool isAllPathReachable()
    {
        return _curSlice->isAllReachable();
    }
    /// Whether it is some path reachable from a source
    virtual bool isSomePathReachable()
    {
        return _curSlice->isPartialReachable();
    }
    /// Dump SVFG with annotated slice information
    //@{
    void dumpSlices();
    void annotateSlice(ProgSlice* slice);
    void printZ3Stat();
    //@}

};

} // End namespace SVF

#endif /* SRCSNKANALYSIS_H_ */
