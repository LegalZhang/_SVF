
#include "Graphs/CHG.h"
#include "SVF-LLVM/BasicTypes.h"

namespace SVF
{

class LLVMModuleSet;

class CHGBuilder
{

private:
    CHGraph* chg;

public:
    typedef CHGraph::CHNodeSetTy CHNodeSetTy;
    typedef CHGraph::WorkList WorkList;

    CHGBuilder(CHGraph* c): chg(c)
    {

    }
    void buildCHG();
    void buildCHGNodes(const GlobalValue *V);
    void buildCHGNodes(const Function* F);
    void buildCHGEdges(const Function* F);
    void buildInternalMaps();
    void readInheritanceMetadataFromModule(const Module &M);

    CHNode *createNode(const std::string& name);

    void connectInheritEdgeViaCall(const Function* caller, const CallBase* cs);
    void connectInheritEdgeViaStore(const Function* caller, const StoreInst* store);

    void buildClassNameToAncestorsDescendantsMap();
    const CHGraph::CHNodeSetTy& getInstancesAndDescendants(const std::string& className);

    void analyzeVTables(const Module &M);
    void buildVirtualFunctionToIDMap();
    void buildCSToCHAVtblsAndVfnsMap();
    const CHNodeSetTy& getCSClasses(const CallBase* cs);
    void addFuncToFuncVector(CHNode::FuncVector &v, const Function *f);

private:
    LLVMModuleSet* llvmModuleSet();
};

} // End namespace SVF

