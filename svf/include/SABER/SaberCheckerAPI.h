
#ifndef SABERCHECKERAPI_H_
#define SABERCHECKERAPI_H_

#include "Util/SVFUtil.h"
#include "Graphs/ICFGNode.h"
#include "SVFIR/SVFVariables.h" // add header

namespace SVF
{

/*
 * Saber Checker API class contains interfaces for various bug checking
 * memory leak detection e.g., alloc free
 * incorrect file operation detection, e.g., fopen, fclose
 */
class SaberCheckerAPI
{

public:
    enum CHECKER_TYPE
    {
        CK_DUMMY = 0,		/// dummy type
        CK_ALLOC,		/// memory allocation
        CK_FREE,      /// memory deallocation
        CK_FOPEN,		/// File open
        CK_FCLOSE		/// File close
    };

    typedef Map<std::string, CHECKER_TYPE> TDAPIMap;

private:
    /// API map, from a string to threadAPI type
    TDAPIMap tdAPIMap;

    /// Constructor
    SaberCheckerAPI ()
    {
        init();
    }

    /// Initialize the map
    void init();

    /// Static reference
    static SaberCheckerAPI* ckAPI;

    /// Get the function type of a function
    inline CHECKER_TYPE getType(const FunObjVar* F) const
    {
        if(F)
        {
            TDAPIMap::const_iterator it= tdAPIMap.find(F->getName());
            if(it != tdAPIMap.end())
                return it->second;
        }
        return CK_DUMMY;
    }

public:
    /// Return a static reference
    static SaberCheckerAPI* getCheckerAPI()
    {
        if(ckAPI == nullptr)
        {
            ckAPI = new SaberCheckerAPI();
        }
        return ckAPI;
    }

    /// Return true if this call is a memory allocation
    //@{
    inline bool isMemAlloc(const FunObjVar* fun) const
    {
        return getType(fun) == CK_ALLOC;
    }
    inline bool isMemAlloc(const CallICFGNode* cs) const
    {
        return isMemAlloc(cs->getCalledFunction());
    }
    //@}

    /// Return true if this call is a memory deallocation
    //@{
    inline bool isMemDealloc(const FunObjVar* fun) const
    {
        return getType(fun) == CK_FREE;
    }
    inline bool isMemDealloc(const CallICFGNode* cs) const
    {
        return isMemDealloc(cs->getCalledFunction());
    }
    //@}

    /// Return true if this call is a file open
    //@{
    inline bool isFOpen(const FunObjVar* fun) const
    {
        return getType(fun) == CK_FOPEN;
    }
    inline bool isFOpen(const CallICFGNode* cs) const
    {
        return isFOpen(cs->getCalledFunction());
    }
    //@}

    /// Return true if this call is a file close
    //@{
    inline bool isFClose(const FunObjVar* fun) const
    {
        return getType(fun) == CK_FCLOSE;
    }
    inline bool isFClose(const CallICFGNode* cs) const
    {
        return isFClose(cs->getCalledFunction());
    }
    //@}

};

} // End namespace SVF

#endif /* SABERCHECKERAPI_H_ */
