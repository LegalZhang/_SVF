
#ifndef SVF_CFLSVFGBUILDER_H
#define SVF_CFLSVFGBUILDER_H

#include "SABER/SaberSVFGBuilder.h"

namespace SVF
{

class CFLSVFGBuilder: public SaberSVFGBuilder
{
public:
    typedef Set<const SVFGNode*> SVFGNodeSet;
    typedef Map<NodeID, PointsTo> NodeToPTSSMap;
    typedef FIFOWorkList<NodeID> WorkList;

    /// Constructor
    CFLSVFGBuilder() = default;

    /// Destructor
    virtual ~CFLSVFGBuilder() = default;

protected:
    /// Re-write create SVFG method
    virtual void buildSVFG();

    /// Remove Incoming Edge for strong-update (SU) store instruction
    /// Because the SU node does not receive indirect value
    virtual void rmIncomingEdgeForSUStore(BVDataPTAImpl* pta);
};

}
#endif //SVF_CFLSVFGBUILDER_H
