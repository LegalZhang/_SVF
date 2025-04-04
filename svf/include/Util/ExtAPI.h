
#ifndef __ExtAPI_H
#define __ExtAPI_H

#include "SVFIR/SVFValue.h"
#include <Util/config.h>
#include <string>
#include <vector>
#include <map>

/// For a more detailed explanation of how External APIs are handled in SVF, please refer to the SVF Wiki: https://github.com/SVF-tools/SVF/wiki/Handling-External-APIs-with-extapi.c

namespace SVF
{

class FunObjVar;

class ExtAPI
{
    friend class LLVMModuleSet;
    friend class SVFIRBuilder;

private:

    static ExtAPI *extOp;

    // Map SVFFunction to its annotations
    Map<const FunObjVar*, std::vector<std::string>> funObjVar2Annotations;

    // extapi.bc file path
    static std::string extBcPath;

    ExtAPI() = default;

public:

    static ExtAPI *getExtAPI();

    static void destory();

    // Set extapi.bc file path
    static bool setExtBcPath(const std::string& path);

    // Get extapi.bc file path
    std::string getExtBcPath();

    // Get the annotation of (F)
    std::string getExtFuncAnnotation(const FunObjVar* fun, const std::string&funcAnnotation);

    const std::vector<std::string>& getExtFuncAnnotations(const FunObjVar*fun);

    // Does (F) have some annotation?
    bool hasExtFuncAnnotation(const FunObjVar* fun, const std::string&funcAnnotation);

public:

    // Does (F) have a static var X (unavailable to us) that its return points to?
    bool has_static(const FunObjVar *F);

    // Does (F) have a memcpy_like operation?
    bool is_memcpy(const FunObjVar *F);

    // Does (F) have a memset_like operation?
    bool is_memset(const FunObjVar *F);

    // Does (F) allocate a new object and return it?
    bool is_alloc(const FunObjVar *F);

    // Does (F) allocate a new object and assign it to one of its arguments?
    bool is_arg_alloc(const FunObjVar *F);

    // Does (F) allocate a new stack object and return it?
    bool is_alloc_stack_ret(const FunObjVar *F);

    // Get the position of argument which holds the new object
    s32_t get_alloc_arg_pos(const FunObjVar *F);

    // Does (F) reallocate a new object?
    bool is_realloc(const FunObjVar *F);

    bool is_ext(const FunObjVar* funObjVar);

private:
    // Set the annotation of (F)
    void setExtFuncAnnotations(const FunObjVar* fun, const std::vector<std::string>&funcAnnotations);
};
} // End namespace SVF

#endif