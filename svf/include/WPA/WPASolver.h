
#ifndef GRAPHSOLVER_H_
#define GRAPHSOLVER_H_

#include "Util/WorkList.h"

namespace SVF
{

/*
 * Generic graph solver for whole program pointer analysis
 */
template<class GraphType>
class WPASolver
{

public:
    ///Define the GTraits and node iterator for printing
    typedef SVF::GenericGraphTraits<GraphType> GTraits;
    typedef typename GTraits::NodeRef           GNODE;
    typedef typename GTraits::EdgeType          GEDGE;
    typedef typename GTraits::ChildIteratorType child_iterator;

    typedef SCCDetection<GraphType> SCC;

    typedef FIFOWorkList<NodeID> WorkList;

protected:

    /// Constructor
    WPASolver(): reanalyze(false), iterationForPrintStat(1000), _graph(nullptr), numOfIteration(0)
    {
    }
    /// Destructor
    virtual ~WPASolver() = default;

    /// Get SCC detector
    inline SCC* getSCCDetector() const
    {
        return scc.get();
    }

    /// Get/Set graph methods
    //@{
    const inline GraphType graph()
    {
        return _graph;
    }
    inline void setGraph(GraphType g)
    {
        _graph = g;
        scc = std::make_unique<SCC>(_graph);
    }
    //@}

    /// SCC detection
    virtual inline NodeStack& SCCDetect()
    {
        getSCCDetector()->find();
        return getSCCDetector()->topoNodeStack();
    }
    virtual inline NodeStack& SCCDetect(NodeSet& candidates)
    {
        getSCCDetector()->find(candidates);
        return getSCCDetector()->topoNodeStack();
    }

    virtual inline void initWorklist()
    {
        NodeStack& nodeStack = SCCDetect();
        while (!nodeStack.empty())
        {
            NodeID nodeId = nodeStack.top();
            nodeStack.pop();
            pushIntoWorklist(nodeId);
        }
    }

    virtual inline void solveWorklist()
    {
        while (!isWorklistEmpty())
        {
            NodeID nodeId = popFromWorklist();
            // Keep solving until workList is empty.
            processNode(nodeId);
            collapseFields();
        }
    }

    /// Following methods are to be implemented in child class, in order to achieve a fully worked PTA
    //@{
    /// Process each node on the graph, to be implemented in the child class
    virtual inline void processNode(NodeID) {}
    /// collapse positive weight cycles of a graph
    virtual void collapseFields() {}
    /// dump statistics
    /// Propagation for the solving, to be implemented in the child class
    virtual void propagate(GNODE* v)
    {
        child_iterator EI = GTraits::direct_child_begin(*v);
        child_iterator EE = GTraits::direct_child_end(*v);
        for (; EI != EE; ++EI)
        {
            if (propFromSrcToDst(*(EI.getCurrent())))
                pushIntoWorklist(Node_Index(*EI));
        }
    }
    /// Propagate information from source to destination node, to be implemented in the child class
    virtual bool propFromSrcToDst(GEDGE*)
    {
        return false;
    }
    //@}

    virtual NodeID sccRepNode(NodeID id) const
    {
        return getSCCDetector()->repNode(id);
    }

    /// Worklist operations
    //@{
    inline NodeID popFromWorklist()
    {
        return sccRepNode(worklist.pop());
    }

    virtual inline void pushIntoWorklist(NodeID id)
    {
        worklist.push(sccRepNode(id));
    }
    inline bool isWorklistEmpty()
    {
        return worklist.empty();
    }
    inline bool isInWorklist(NodeID id)
    {
        return worklist.find(id);
    }
    //@}

    /// Reanalyze if any constraint value changed
    bool reanalyze;
    /// print out statistics for i-th iteration
    u32_t iterationForPrintStat;


    /// Get node on the graph
    inline GNODE* Node(NodeID id)
    {
        return GTraits::getNode(_graph, id);
    }

    /// Get node ID
    inline NodeID Node_Index(GNODE node)
    {
        return GTraits::getNodeID(node);
    }

protected:
    /// Graph
    GraphType _graph;

    /// SCC
    std::unique_ptr<SCC> scc;

    /// Worklist for resolution
    WorkList worklist;

public:
    /// num of iterations during constraint solving
    u32_t numOfIteration;
};

} // End namespace SVF

#endif /* GRAPHSOLVER_H_ */
