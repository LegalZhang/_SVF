

#ifndef DISNCTMRGENERATOR_H_
#define DISNCTMRGENERATOR_H_

#include "MSSA/MemRegion.h"

namespace SVF
{

/*!
 * Distinct memory region generator.
 */
class DistinctMRG : public MRGenerator
{
public:
    DistinctMRG(BVDataPTAImpl* p, bool ptrOnly) : MRGenerator(p, ptrOnly)
    {}

    ~DistinctMRG() {}

protected:
    /// Partition regions
    virtual void partitionMRs();

    /// Get memory region at a load
    virtual void getMRsForLoad(MRSet& aliasMRs, const NodeBS& cpts, const FunObjVar* fun);

    /// Get memory regions to be inserted at a load statement.
    virtual void getMRsForCallSiteRef(MRSet& aliasMRs, const NodeBS& cpts, const FunObjVar* fun);
private:
    /// Create memory regions for each points-to target.
    void createDistinctMR(const FunObjVar* func, const NodeBS& cpts);

};

/*!
 * Create memory regions which don't have intersections with each other in the same function scope.
 */
class IntraDisjointMRG : public MRGenerator
{
public:
    typedef OrderedMap<NodeBS, PointsToList> PtsToSubPtsMap;
    typedef Map<const FunObjVar*, PtsToSubPtsMap> FunToPtsMap;
    typedef Map<const FunObjVar*, PointsToList> FunToInterMap;

    IntraDisjointMRG(BVDataPTAImpl* p, bool ptrOnly) : MRGenerator(p, ptrOnly)
    {}

    ~IntraDisjointMRG() {}

protected:

    /// Partition regions
    virtual void partitionMRs();

    /**
     * Get memory regions to be inserted at a load statement.
     * @param cpts The conditional points-to set of load statement.
     * @param fun The function being analyzed.
     * @param mrs Memory region set contains all possible target memory regions.
     */
    virtual inline void getMRsForLoad(MRSet& aliasMRs, const NodeBS& cpts,
                                      const FunObjVar* fun)
    {
        const PointsToList& inters = getIntersList(fun);
        getMRsForLoadFromInterList(aliasMRs, cpts, inters);
    }

    void getMRsForLoadFromInterList(MRSet& mrs, const NodeBS& cpts, const PointsToList& inters);

    /// Get memory regions to be inserted at a load statement.
    virtual void getMRsForCallSiteRef(MRSet& aliasMRs, const NodeBS& cpts, const FunObjVar* fun);

    /// Create disjoint memory region
    void createDisjointMR(const FunObjVar* func, const NodeBS& cpts);

    /// Compute intersections between cpts and computed cpts intersections before.
    void computeIntersections(const NodeBS& cpts, PointsToList& inters);

private:
    inline PtsToSubPtsMap& getPtsSubSetMap(const FunObjVar* func)
    {
        return funcToPtsMap[func];
    }

    inline PointsToList& getIntersList(const FunObjVar* func)
    {
        return funcToInterMap[func];
    }

    inline const PtsToSubPtsMap& getPtsSubSetMap(const FunObjVar* func) const
    {
        FunToPtsMap::const_iterator it = funcToPtsMap.find(func);
        assert(it != funcToPtsMap.end() && "can not find pts map for specified function");
        return it->second;
    }

    FunToPtsMap funcToPtsMap;
    FunToInterMap funcToInterMap;
};

/*!
 * Create memory regions which don't have intersections with each other in the whole program scope.
 */
class InterDisjointMRG : public IntraDisjointMRG
{
public:
    InterDisjointMRG(BVDataPTAImpl* p, bool ptrOnly) : IntraDisjointMRG(p, ptrOnly)
    {}

    ~InterDisjointMRG() {}

protected:
    /// Partition regions
    virtual void partitionMRs();

    /**
     * Get memory regions to be inserted at a load statement.
     * @param cpts The conditional points-to set of load statement.
     * @param fun The function being analyzed.
     * @param mrs Memory region set contains all possible target memory regions.
     */
    virtual inline void getMRsForLoad(MRSet& aliasMRs, const NodeBS& cpts,
                                      const FunObjVar*)
    {
        getMRsForLoadFromInterList(aliasMRs, cpts, inters);
    }

private:
    PointsToList inters;
};

} // End namespace SVF

#endif /* DISNCTMRGENERATOR_H_ */
