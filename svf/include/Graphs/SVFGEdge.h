
#ifndef INCLUDE_MSSA_SVFGEDGE_H_
#define INCLUDE_MSSA_SVFGEDGE_H_

#include "MSSA/MemSSA.h"
#include "Graphs/VFGEdge.h"

namespace SVF
{

/*!
 * SVFG edge representing indirect value-flows from a caller to its callee at a callsite
 */
class IndirectSVFGEdge : public VFGEdge
{

public:
    typedef Set<const MRVer*> MRVerSet;
private:
    NodeBS cpts;
public:
    /// Constructor
    IndirectSVFGEdge(VFGNode* s, VFGNode* d, GEdgeFlag k): VFGEdge(s,d,k)
    {
    }
    /// Handle memory region
    //@{
    inline bool addPointsTo(const NodeBS& c)
    {
        return (cpts |= c);
    }
    inline const NodeBS& getPointsTo() const
    {
        return cpts;
    }
    //@}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const IndirectSVFGEdge *)
    {
        return true;
    }
    static inline bool classof(const VFGEdge *edge)
    {
        return edge->getEdgeKind() == IntraIndirectVF  ||
               edge->getEdgeKind() == CallIndVF ||
               edge->getEdgeKind() == RetIndVF ||
               edge->getEdgeKind() == TheadMHPIndirectVF;
    }
    static inline bool classof(const GenericVFGEdgeTy *edge)
    {
        return edge->getEdgeKind() == IntraIndirectVF  ||
               edge->getEdgeKind() == CallIndVF ||
               edge->getEdgeKind() == RetIndVF ||
               edge->getEdgeKind() == TheadMHPIndirectVF;
    }
    //@}

    virtual const std::string toString() const;
};

/*!
 * Intra SVFG edge representing indirect intra-procedural value-flows
 */
class IntraIndSVFGEdge : public IndirectSVFGEdge
{

public:
    IntraIndSVFGEdge(VFGNode* s, VFGNode* d): IndirectSVFGEdge(s,d,IntraIndirectVF)
    {
    }
    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const IntraIndSVFGEdge*)
    {
        return true;
    }
    static inline bool classof(const IndirectSVFGEdge *edge)
    {
        return edge->getEdgeKind() == IntraIndirectVF;
    }
    static inline bool classof(const VFGEdge *edge)
    {
        return edge->getEdgeKind() == IntraIndirectVF;
    }
    static inline bool classof(const GenericVFGEdgeTy *edge)
    {
        return edge->getEdgeKind() == IntraIndirectVF;
    }
    //@}

    virtual const std::string toString() const;
};

/*!
 * SVFG call edge representing indirect value-flows from a caller to its callee at a callsite
 */
class CallIndSVFGEdge : public IndirectSVFGEdge
{

private:
    CallSiteID csId;
public:
    CallIndSVFGEdge(VFGNode* s, VFGNode* d, CallSiteID id):
        IndirectSVFGEdge(s,d,makeEdgeFlagWithInvokeID(CallIndVF,id)),csId(id)
    {
    }
    inline CallSiteID getCallSiteId() const
    {
        return csId;
    }
    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const CallIndSVFGEdge *)
    {
        return true;
    }
    static inline bool classof(const IndirectSVFGEdge *edge)
    {
        return edge->getEdgeKind() == CallIndVF;
    }
    static inline bool classof(const VFGEdge *edge)
    {
        return edge->getEdgeKind() == CallIndVF ;
    }
    static inline bool classof(const GenericVFGEdgeTy *edge)
    {
        return edge->getEdgeKind() == CallIndVF ;
    }
    //@}

    virtual const std::string toString() const;
};

/*!
 * SVFG return edge representing direct value-flows from a callee to its caller at a callsite
 */
class RetIndSVFGEdge : public IndirectSVFGEdge
{

private:
    CallSiteID csId;
public:
    RetIndSVFGEdge(VFGNode* s, VFGNode* d, CallSiteID id):
        IndirectSVFGEdge(s,d,makeEdgeFlagWithInvokeID(RetIndVF,id)),csId(id)
    {
    }
    inline CallSiteID getCallSiteId() const
    {
        return csId;
    }
    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const RetIndSVFGEdge *)
    {
        return true;
    }
    static inline bool classof(const IndirectSVFGEdge *edge)
    {
        return edge->getEdgeKind() == RetIndVF;
    }
    static inline bool classof(const VFGEdge *edge)
    {
        return edge->getEdgeKind() == RetIndVF;
    }
    static inline bool classof(const GenericVFGEdgeTy *edge)
    {
        return edge->getEdgeKind() == RetIndVF;
    }
    //@}

    virtual const std::string toString() const;
};


/*!
 * MHP SVFG edge representing indirect value-flows between
 * two memory access may-happen-in-parallel in multithreaded program
 */
class ThreadMHPIndSVFGEdge : public IndirectSVFGEdge
{

public:
    ThreadMHPIndSVFGEdge(VFGNode* s, VFGNode* d): IndirectSVFGEdge(s,d,TheadMHPIndirectVF)
    {
    }
    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const ThreadMHPIndSVFGEdge*)
    {
        return true;
    }
    static inline bool classof(const IndirectSVFGEdge *edge)
    {
        return edge->getEdgeKind() == TheadMHPIndirectVF;
    }
    static inline bool classof(const VFGEdge *edge)
    {
        return edge->getEdgeKind() == TheadMHPIndirectVF;
    }
    static inline bool classof(const GenericVFGEdgeTy *edge)
    {
        return edge->getEdgeKind() == TheadMHPIndirectVF;
    }
    //@}

    virtual const std::string toString() const;
};

} // End namespace SVF

#endif /* INCLUDE_MSSA_SVFGEDGE_H_ */
