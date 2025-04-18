

#include "Util/SVFUtil.h"
#include "WPA/WPAStat.h"
#include "WPA/VersionedFlowSensitive.h"
#include "MemoryModel/PointsTo.h"

using namespace SVF;
using namespace SVFUtil;

void VersionedFlowSensitiveStat::clearStat()
{
    _NumVersions         = 0;
    _MaxVersions         = 0;
    _NumNonEmptyVersions = 0;
    _NumSingleVersion    = 0;
    _NumUsedVersions     = 0;
    _NumEmptyVersions    = 0;
    _MaxPtsSize          = 0;
    _MaxTopLvlPtsSize    = 0;
    _MaxVersionPtsSize   = 0;
    _TotalPtsSize        = 0;
    _AvgPtsSize          = 0.0;
    _AvgTopLvlPtsSize    = 0.0;
    _AvgVersionPtsSize   = 0.0;
}

void VersionedFlowSensitiveStat::performStat()
{
    // Largely based on that in FlowSensitiveStat. Would be better to split the FSStat version
    // and reuse code rather than copy.
    assert(SVFUtil::isa<VersionedFlowSensitive>(vfspta) && "VFSStat::performStat: not given VFSPTA.");
    endClk();

    clearStat();

    SVFIR *pag = vfspta->getPAG();

    versionStat();
    ptsSizeStat();

    u32_t fiObjNumber = 0;
    u32_t fsObjNumber = 0;
    Set<NodeID> nodeSet;
    for (SVFIR::const_iterator it = pag->begin(); it != pag->end(); ++it)
    {
        NodeID nodeId = it->first;
        PAGNode* pagNode = it->second;
        if (SVFUtil::isa<ObjVar>(pagNode))
        {
            const BaseObjVar* baseObj = pag->getBaseObject(nodeId);
            NodeID baseId = baseObj->getId();
            if (nodeSet.insert(baseId).second)
            {
                if (baseObj->isFieldInsensitive()) fiObjNumber++;
                else fsObjNumber++;
            }
        }
    }

    PTNumStatMap["FIObjNum"] = fiObjNumber;
    PTNumStatMap["FSObjNum"] = fsObjNumber;

    unsigned numOfCopy = 0;
    unsigned numOfStore = 0;
    for (SVFG::iterator it = vfspta->svfg->begin(); it != vfspta->svfg->end(); ++it)
    {
        SVFGNode* svfgNode = it->second;
        if (SVFUtil::isa<CopySVFGNode>(svfgNode)) numOfCopy++;
        else if (SVFUtil::isa<StoreSVFGNode>(svfgNode)) numOfStore++;
    }

    PTAStat::performStat();

    timeStatMap["TotalTime"]    = (endTime - startTime)/TIMEINTERVAL;
    timeStatMap["SolveTime"]          = vfspta->solveTime;
    timeStatMap["SCCTime"]            = vfspta->sccTime;
    timeStatMap["ProcessTime"]        = vfspta->processTime;
    timeStatMap["PropagationTime"]    = vfspta->propagationTime;
    timeStatMap["DirectPropaTime"]    = vfspta->directPropaTime;
    timeStatMap["IndirectPropaTime"]  = vfspta->indirectPropaTime;
    timeStatMap["Strong/WeakUpdTime"] = vfspta->updateTime;
    timeStatMap["AddrTime"]           = vfspta->addrTime;
    timeStatMap["CopyTime"]           = vfspta->copyTime;
    timeStatMap["GepTime"]            = vfspta->gepTime;
    timeStatMap["LoadTime"]           = vfspta->loadTime;
    timeStatMap["StoreTime"]          = vfspta->storeTime;
    timeStatMap["UpdateCGTime"]       = vfspta->updateCallGraphTime;
    timeStatMap["PhiTime"]            = vfspta->phiTime;
    timeStatMap["meldLabelingTime"]   = vfspta->meldLabelingTime;
    timeStatMap["PrelabelingTime"]    = vfspta->prelabelingTime;
    timeStatMap["VersionPropTime"]    = vfspta->versionPropTime;

    PTNumStatMap["TotalPointers"]  = pag->getValueNodeNum();
    PTNumStatMap["TotalObjects"]   = pag->getObjectNodeNum();

    PTNumStatMap["Pointers"]         = pag->getValueNodeNum();
    PTNumStatMap["MemObjects"]       = pag->getObjectNodeNum();
    PTNumStatMap["DummyFieldPtrs"] = pag->getFieldValNodeNum();
    PTNumStatMap["FieldObjs"]  = pag->getFieldObjNodeNum();

    PTNumStatMap["TotalVersions"]     = _NumVersions;
    PTNumStatMap["MaxVersionsForObj"] = _MaxVersions;
    PTNumStatMap["TotalNonEmptyVPts"] = _NumNonEmptyVersions;
    PTNumStatMap["TotalEmptyVPts"]    = _NumEmptyVersions;
    PTNumStatMap["TotalExistingVPts"] = _NumUsedVersions;
    PTNumStatMap["TotalSingleVObjs"]  = _NumSingleVersion;

    PTNumStatMap["CopysNum"]  = numOfCopy;
    PTNumStatMap["StoresNum"] = numOfStore;

    PTNumStatMap["SolveIterations"] = vfspta->numOfIteration;

    PTNumStatMap["IndEdgeSolved"] = vfspta->getNumOfResolvedIndCallEdge();

    PTNumStatMap["StrongUpdates"] = vfspta->svfgHasSU.count();

    PTNumStatMap["MaxPtsSize"]        = _MaxPtsSize;
    PTNumStatMap["MaxTopLvlPtsSize"]  = _MaxTopLvlPtsSize;
    PTNumStatMap["MaxVersionPtsSize"] = _MaxVersionPtsSize;

    timeStatMap["AvgPtsSize"]        = _AvgPtsSize;
    timeStatMap["AvgTopLvlPtsSize"]  = _AvgTopLvlPtsSize;
    timeStatMap["AvgVersionPtsSize"] = _AvgVersionPtsSize;

    PTNumStatMap["ProcessedAddr"]     = vfspta->numOfProcessedAddr;
    PTNumStatMap["ProcessedCopy"]     = vfspta->numOfProcessedCopy;
    PTNumStatMap["ProcessedGep"]      = vfspta->numOfProcessedGep;
    PTNumStatMap["ProcessedLoad"]     = vfspta->numOfProcessedLoad;
    PTNumStatMap["ProcessedStore"]    = vfspta->numOfProcessedStore;
    PTNumStatMap["ProcessedPhi"]      = vfspta->numOfProcessedPhi;
    PTNumStatMap["ProcessedAParam"]   = vfspta->numOfProcessedActualParam;
    PTNumStatMap["ProcessedFRet"]     = vfspta->numOfProcessedFormalRet;
    PTNumStatMap["ProcessedMSSANode"] = vfspta->numOfProcessedMSSANode;

    PTNumStatMap["NumOfNodesInSCC"] = vfspta->numOfNodesInSCC;
    PTNumStatMap["MaxSCCSize"]      = vfspta->maxSCCSize;
    PTNumStatMap["NumOfSCC"]        = vfspta->numOfSCC;
    timeStatMap["AverageSCCSize"]   = (vfspta->numOfSCC == 0) ? 0 :
                                      ((double)vfspta->numOfNodesInSCC / vfspta->numOfSCC);

    PTAStat::printStat("Versioned Flow-Sensitive Pointer Analysis Statistics");
}

void VersionedFlowSensitiveStat::versionStat(void)
{
    // TODO! Need to merge yield/consume.
    _NumSingleVersion = 0;
    _MaxVersions = 0;

    u32_t totalVersionPtsSize = 0;
    for (const VersionedFlowSensitive::LocVersionMap *lvm :
            {
                &vfspta->consume, &vfspta->yield
            })
    {
        for (const VersionedFlowSensitive::ObjToVersionMap  &lov : *lvm)
        {
            for (const VersionedFlowSensitive::ObjToVersionMap::value_type &ov : lov)
            {
                const NodeID o = ov.first;
                const Version v = ov.second;

                ++_NumVersions;

                // If the version was just over-approximate and never accessed, ignore.
                // TODO: with vPtD changed there is no interface to check if the PTS
                //       exists; an emptiness check is *not* an existence check.
                if (vfspta->vPtD->getPts(vfspta->atKey(o, v)).empty()) continue;

                const PointsTo &ovPts = vfspta->vPtD->getPts(vfspta->atKey(o, v));
                if (!ovPts.empty()) ++_NumNonEmptyVersions;
                else ++_NumEmptyVersions;

                _TotalPtsSize += ovPts.count();
                totalVersionPtsSize += ovPts.count();
                if (ovPts.count() > _MaxVersionPtsSize) _MaxVersionPtsSize = ovPts.count();
            }
        }
    }

    _NumUsedVersions = _NumNonEmptyVersions + _NumEmptyVersions;

    if (_NumNonEmptyVersions != 0) _AvgVersionPtsSize = (double)totalVersionPtsSize / (double)_NumUsedVersions;

    _TotalPtsSize += totalVersionPtsSize;
}

void VersionedFlowSensitiveStat::ptsSizeStat()
{
    u32_t totalValidTopLvlPointers = 0;
    u32_t totalTopLvlPtsSize = 0;
    for (SVFIR::iterator it = vfspta->getPAG()->begin(); it != vfspta->getPAG()->end(); ++it)
    {
        if (!vfspta->getPAG()->isValidTopLevelPtr(it->second)) continue;

        NodeID p = it->first;

        totalValidTopLvlPointers++;

        u32_t size = vfspta->getPts(p).count();
        totalTopLvlPtsSize += size;
        if (size > _MaxTopLvlPtsSize) _MaxTopLvlPtsSize = size;
    }

    if (totalValidTopLvlPointers != 0) _AvgTopLvlPtsSize = (double)totalTopLvlPtsSize / (double)totalValidTopLvlPointers;

    _TotalPtsSize += totalTopLvlPtsSize;

    if (_NumNonEmptyVersions + totalValidTopLvlPointers != 0)
    {
        _AvgPtsSize = (double)_TotalPtsSize / (double)(_NumNonEmptyVersions + totalValidTopLvlPointers);
    }

    _MaxPtsSize = _MaxVersionPtsSize > _MaxTopLvlPtsSize ? _MaxVersionPtsSize : _MaxTopLvlPtsSize;
}
