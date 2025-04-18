

#ifndef WPAFSSOLVER_H_
#define WPAFSSOLVER_H_

#include "WPA/WPASolver.h"

namespace SVF
{

/*!
 * Flow-sensitive Solver
 */
template<class GraphType>
class WPAFSSolver : public WPASolver<GraphType>
{
public:
    /// Constructor
    WPAFSSolver() : WPASolver<GraphType>()
    {}
    /// Destructor
    virtual ~WPAFSSolver() {}

    /// SCC methods
    virtual inline NodeID sccRepNode(NodeID id) const
    {
        return id;
    }

protected:
    NodeStack nodeStack;	///< stack used for processing nodes.

    /// SCC detection
    virtual NodeStack& SCCDetect()
    {
        /// SCC detection
        this->getSCCDetector()->find();

        assert(nodeStack.empty() && "node stack is not empty, some nodes are not popped properly.");

        /// Both rep and sub nodes need to be processed later.
        /// Collect sub nodes from SCCDetector.
        FIFOWorkList<NodeID> revTopoStack = this->getSCCDetector()->revTopoNodeStack();
        while (!revTopoStack.empty())
        {
            NodeID nodeId = revTopoStack.front();
            revTopoStack.pop();
            const NodeBS& subNodes = this->getSCCDetector()->subNodes(nodeId);
            for (NodeBS::iterator it = subNodes.begin(), eit = subNodes.end(); it != eit; ++it)
            {
                /// restore the topological order.
                nodeStack.push(*it);
            }
        }

        return nodeStack;
    }
};



/*!
 * Solver based on SCC cycles.
 */
template<class GraphType>
class WPASCCSolver : public WPAFSSolver<GraphType>
{
public:
    typedef typename WPASolver<GraphType>::GTraits GTraits;
    typedef typename WPASolver<GraphType>::GNODE GNODE;
    typedef typename WPASolver<GraphType>::child_iterator child_iterator;

    WPASCCSolver() : WPAFSSolver<GraphType>() {}

    virtual ~WPASCCSolver() {}

protected:
    virtual void solve()
    {
        /// All nodes will be solved afterwards, so the worklist
        /// can be cleared before each solve iteration.
        while (!this->isWorklistEmpty())
            this->popFromWorklist();

        NodeStack& nodeStack = this->SCCDetect();

        while (!nodeStack.empty())
        {
            NodeID rep = nodeStack.top();
            nodeStack.pop();

            setCurrentSCC(rep);

            const NodeBS& sccNodes = this->getSCCDetector()->subNodes(rep);
            for (NodeBS::iterator it = sccNodes.begin(), eit = sccNodes.end(); it != eit; ++it)
                this->pushIntoWorklist(*it);

            while (!this->isWorklistEmpty())
                this->processNode(this->popFromWorklist());
        }
    }

    /// Propagation for the solving, to be implemented in the child class
    virtual void propagate(GNODE* v)
    {
        child_iterator EI = GTraits::direct_child_begin(v);
        child_iterator EE = GTraits::direct_child_end(v);
        for (; EI != EE; ++EI)
        {
            if (this->propFromSrcToDst(*(EI.getCurrent())))
                addNodeIntoWorkList(this->Node_Index(*EI));
        }
    }

    virtual inline void addNodeIntoWorkList(NodeID node)
    {
        if (isInCurrentSCC(node))
            this->pushIntoWorklist(node);
    }

    inline bool isInCurrentSCC(NodeID node)
    {
        return (const_cast<NodeBS&>(this->getSCCDetector()->subNodes(curSCCID))).test(node);
    }
    inline void setCurrentSCC(NodeID id)
    {
        curSCCID = this->getSCCDetector()->repNode(id);
    }

    NodeID curSCCID;	///< index of current SCC.
};



/*!
 * Only solve nodes which need to be analyzed.
 */
template<class GraphType>
class WPAMinimumSolver : public WPASCCSolver<GraphType>
{
public:
    typedef typename WPASolver<GraphType>::GTraits GTraits;
    typedef typename WPASolver<GraphType>::GNODE GNODE;
    typedef typename WPASolver<GraphType>::child_iterator child_iterator;

    WPAMinimumSolver() : WPASCCSolver<GraphType>() {}

    virtual ~WPAMinimumSolver() {}

protected:
    virtual void solve()
    {
        bool solveAll = true;
        /// If the worklist is not empty, then only solve these nodes contained in
        /// worklist. Otherwise all nodes in the graph will be processed.
        if (!this->isWorklistEmpty())
        {
            solveAll = false;
            while (!this->isWorklistEmpty())
                addNewCandidate(this->popFromWorklist());
        }

        NodeStack& nodeStack = this->SCCDetect();

        while (!nodeStack.empty())
        {
            NodeID rep = nodeStack.top();
            nodeStack.pop();

            this->setCurrentSCC(rep);

            NodeBS sccNodes = this->getSCCDetector()->subNodes(rep);
            if (solveAll == false)
                sccNodes &= getCandidates();	/// get nodes which need to be processed in this SCC cycle

            for (NodeBS::iterator it = sccNodes.begin(), eit = sccNodes.end(); it != eit; ++it)
                this->pushIntoWorklist(*it);

            while (!this->isWorklistEmpty())
                this->processNode(this->popFromWorklist());

            removeCandidates(sccNodes);		/// remove nodes which have been processed from the candidate set
        }
    }

    virtual inline void addNodeIntoWorkList(NodeID node)
    {
        if (this->isInCurrentSCC(node))
            this->pushIntoWorklist(node);
        else
            addNewCandidate(node);
    }

private:
    inline void addNewCandidate(NodeID node)
    {
        candidates.set(node);
    }
    inline const NodeBS& getCandidates() const
    {
        return candidates;
    }
    inline void removeCandidates(const NodeBS& nodes)
    {
        candidates.intersectWithComplement(nodes);
    }

    NodeBS candidates;	///< nodes which need to be analyzed in current iteration.
};

} // End namespace SVF

#endif /* WPAFSSOLVER_H_ */
