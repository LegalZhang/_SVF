

#include "Graphs/CHG.h"
#include "Util/SVFUtil.h"
#include "Graphs/ICFG.h"
#include "SVFIR/SVFIR.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;

static bool hasEdge(const CHNode *src, const CHNode *dst,
                    CHEdge::CHEDGETYPE et)
{
    for (CHEdge::CHEdgeSetTy::const_iterator it = src->getOutEdges().begin(),
            eit = src->getOutEdges().end(); it != eit; ++it)
    {
        CHNode *node = (*it)->getDstNode();
        CHEdge::CHEDGETYPE edgeType = (*it)->getEdgeType();
        if (node == dst && edgeType == et)
            return true;
    }
    return false;
}

static bool checkArgTypes(const CallICFGNode* cs, const FunObjVar* fn)
{

    // here we skip the first argument (i.e., this pointer)
    u32_t arg_size = (fn->arg_size() > cs->arg_size()) ? cs->arg_size(): fn->arg_size();
    if(arg_size > 1)
    {
        for (unsigned i = 1; i < arg_size; i++)
        {
            auto cs_arg = cs->getArgument(i);
            auto fn_arg = fn->getArg(i);
            if (cs_arg->getType() != fn_arg->getType())
            {
                return false;
            }
        }
    }

    return true;
}

bool CHGraph::csHasVtblsBasedonCHA(const CallICFGNode* cs)
{
    CallNodeToVTableSetMap::const_iterator it = callNodeToCHAVtblsMap.find(cs);
    return it != callNodeToCHAVtblsMap.end();
}
bool CHGraph::csHasVFnsBasedonCHA(const CallICFGNode* cs)
{
    CallNodeToVFunSetMap::const_iterator it = callNodeToCHAVFnsMap.find(cs);
    return it != callNodeToCHAVFnsMap.end();
}
const VTableSet& CHGraph::getCSVtblsBasedonCHA(const CallICFGNode* cs)
{
    CallNodeToVTableSetMap::const_iterator it = callNodeToCHAVtblsMap.find(cs);
    assert(it != callNodeToCHAVtblsMap.end() && "cs does not have vtabls based on CHA.");
    return it->second;
}
const VFunSet& CHGraph::getCSVFsBasedonCHA(const CallICFGNode* cs)
{
    CallNodeToVFunSetMap::const_iterator it = callNodeToCHAVFnsMap.find(cs);
    assert(it != callNodeToCHAVFnsMap.end() && "cs does not have vfns based on CHA.");
    return it->second;
}

void CHGraph::addEdge(const string className, const string baseClassName,
                      CHEdge::CHEDGETYPE edgeType)
{
    CHNode *srcNode = getNode(className);
    CHNode *dstNode = getNode(baseClassName);
    assert(srcNode && dstNode && "node not found?");

    if (!hasEdge(srcNode, dstNode, edgeType))
    {
        CHEdge *edge = new CHEdge(srcNode, dstNode, edgeType);
        srcNode->addOutgoingEdge(edge);
        dstNode->addIncomingEdge(edge);
    }
}

CHNode *CHGraph::getNode(const string name) const
{
    auto chNode = classNameToNodeMap.find(name);
    if (chNode != classNameToNodeMap.end()) return chNode->second;
    else return nullptr;
}


/*
 * Get virtual functions for callsite "cs" based on vtbls (calculated
 * based on pointsto set)
 */
void CHGraph::getVFnsFromVtbls(const CallICFGNode* callsite, const VTableSet &vtbls, VFunSet &virtualFunctions)
{
    /// get target virtual functions
    size_t idx = callsite->getFunIdxInVtable();
    /// get the function name of the virtual callsite
    string funName = callsite->getFunNameOfVirtualCall();
    for (const GlobalObjVar *vt : vtbls)
    {
        const CHNode *child = getNode(vt->getName());
        if (child == nullptr)
            continue;
        CHNode::FuncVector vfns;
        child->getVirtualFunctions(idx, vfns);
        for (CHNode::FuncVector::const_iterator fit = vfns.begin(),
                feit = vfns.end(); fit != feit; ++fit)
        {
            const FunObjVar* callee = *fit;
            if (callsite->arg_size() == callee->arg_size() ||
                    (callsite->isVarArg() && callee->isVarArg()))
            {

                // if argument types do not match
                // skip this one
                if (!checkArgTypes(callsite, callee))
                    continue;

                string calleeName = callee->getName();

                /*
                 * The compiler will add some special suffix (e.g.,
                 * "[abi:cxx11]") to the end of some virtual function:
                 * In dealII
                 * function: FE_Q<3>::get_name
                 * will be mangled as: _ZNK4FE_QILi3EE8get_nameB5cxx11Ev
                 * after demangling: FE_Q<3>::get_name[abi:cxx11]
                 * The special suffix ("[abi:cxx11]") needs to be removed
                 */
                const std::string suffix("[abi:cxx11]");
                size_t suffix_pos = calleeName.rfind(suffix);
                if (suffix_pos != string::npos)
                    calleeName.erase(suffix_pos, suffix.size());

                /*
                 * if we can't get the function name of a virtual callsite, all virtual
                 * functions calculated by idx will be valid
                 */
                if (funName.size() == 0)
                {
                    virtualFunctions.insert(callee);
                }
                else if (funName[0] == '~')
                {
                    /*
                     * if the virtual callsite is calling a destructor, then all
                     * destructors in the ch will be valid
                     * class A { virtual ~A(){} };
                     * class B: public A { virtual ~B(){} };
                     * int main() {
                     *   A *a = new B;
                     *   delete a;  /// the function name of this virtual callsite is ~A()
                     * }
                     */
                    if (calleeName[0] == '~')
                    {
                        virtualFunctions.insert(callee);
                    }
                }
                else
                {
                    /*
                     * for other virtual function calls, the function name of the callsite
                     * and the function name of the target callee should match exactly
                     */
                    if (funName.compare(calleeName) == 0)
                    {
                        virtualFunctions.insert(callee);
                    }
                }
            }
        }
    }
}


void CHNode::getVirtualFunctions(u32_t idx, FuncVector &virtualFunctions) const
{
    for (vector<FuncVector>::const_iterator it = virtualFunctionVectors.begin(),
            eit = virtualFunctionVectors.end(); it != eit; ++it)
    {
        if ((*it).size() > idx)
            virtualFunctions.push_back((*it)[idx]);
    }
}

void CHGraph::printCH()
{
    for (CHGraph::const_iterator it = this->begin(), eit = this->end();
            it != eit; ++it)
    {
        const CHNode *node = it->second;
        outs() << "class: " << node->getName() << "\n";
        for (CHEdge::CHEdgeSetTy::const_iterator it = node->OutEdgeBegin();
                it != node->OutEdgeEnd(); ++it)
        {
            if ((*it)->getEdgeType() == CHEdge::INHERITANCE)
                outs() << (*it)->getDstNode()->getName() << " --inheritance--> "
                       << (*it)->getSrcNode()->getName() << "\n";
            else
                outs() << (*it)->getSrcNode()->getName() << " --instance--> "
                       << (*it)->getDstNode()->getName() << "\n";
        }
    }
    outs() << '\n';
}

/*!
 * Dump call graph into dot file
 */
void CHGraph::dump(const std::string& filename)
{
    GraphPrinter::WriteGraphToFile(outs(), filename, this);
    printCH();
}

void CHGraph::view()
{
    SVF::ViewGraph(this, "Class Hierarchy Graph");
}

namespace SVF
{

/*!
 * Write value flow graph into dot file for debugging
 */
template<>
struct DOTGraphTraits<CHGraph*> : public DefaultDOTGraphTraits
{

    typedef CHNode NodeType;
    DOTGraphTraits(bool isSimple = false) :
        DefaultDOTGraphTraits(isSimple)
    {
    }

    /// Return name of the graph
    static std::string getGraphName(CHGraph*)
    {
        return "Class Hierarchy Graph";
    }
    /// Return function name;
    static std::string getNodeLabel(CHNode *node, CHGraph*)
    {
        return node->getName();
    }

    static std::string getNodeAttributes(CHNode *node, CHGraph*)
    {
        if (node->isPureAbstract())
        {
            return "shape=tab";
        }
        else
            return "shape=box";
    }

    template<class EdgeIter>
    static std::string getEdgeAttributes(CHNode*, EdgeIter EI, CHGraph*)
    {

        CHEdge* edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");
        if (edge->getEdgeType() == CHEdge::INHERITANCE)
        {
            return "style=solid";
        }
        else
        {
            return "style=dashed";
        }
    }
};
} // End namespace llvm
