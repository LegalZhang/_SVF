
#pragma once
#include "AE/Core/AbstractState.h"
#include "AE/Core/ICFGWTO.h"
#include "AE/Svfexe/AEDetector.h"
#include "AE/Svfexe/AbsExtAPI.h"
#include "Util/SVFBugReport.h"
#include "WPA/Andersen.h"

namespace SVF
{

// Forward declaration of AbstractInterpretation class
class AbstractInterpretation;

/**
 * @class AbsExtAPI
 * @brief Handles external API calls and manages abstract states.
 */
class AbsExtAPI
{
public:
    /**
     * @enum ExtAPIType
     * @brief Enumeration of external API types.
     */
    enum ExtAPIType { UNCLASSIFIED, MEMCPY, MEMSET, STRCPY, STRCAT };

    /**
     * @brief Constructor for AbsExtAPI.
     * @param abstractTrace Reference to a map of ICFG nodes to abstract states.
     */
    AbsExtAPI(Map<const ICFGNode*, AbstractState>& traces);

    /**
     * @brief Initializes the external function map.
     */
    void initExtFunMap();

    /**
     * @brief Reads a string from the abstract state.
     * @param as Reference to the abstract state.
     * @param rhs Pointer to the SVF variable representing the string.
     * @return The string value.
     */
    std::string strRead(AbstractState& as, const SVFVar* rhs);

    /**
     * @brief Handles an external API call.
     * @param call Pointer to the call ICFG node.
     */
    void handleExtAPI(const CallICFGNode *call);

    /**
     * @brief Handles the strcpy API call.
     * @param call Pointer to the call ICFG node.
     */
    void handleStrcpy(const CallICFGNode *call);

    /**
     * @brief Calculates the length of a string.
     * @param as Reference to the abstract state.
     * @param strValue Pointer to the SVF variable representing the string.
     * @return The interval value representing the string length.
     */
    IntervalValue getStrlen(AbstractState& as, const SVF::SVFVar *strValue);

    /**
     * @brief Handles the strcat API call.
     * @param call Pointer to the call ICFG node.
     */
    void handleStrcat(const SVF::CallICFGNode *call);

    /**
     * @brief Handles the memcpy API call.
     * @param as Reference to the abstract state.
     * @param dst Pointer to the destination SVF variable.
     * @param src Pointer to the source SVF variable.
     * @param len The interval value representing the length to copy.
     * @param start_idx The starting index for copying.
     */
    void handleMemcpy(AbstractState& as, const SVF::SVFVar *dst, const SVF::SVFVar *src, IntervalValue len, u32_t start_idx);

    /**
     * @brief Handles the memset API call.
     * @param as Reference to the abstract state.
     * @param dst Pointer to the destination SVF variable.
     * @param elem The interval value representing the element to set.
     * @param len The interval value representing the length to set.
     */
    void handleMemset(AbstractState& as, const SVFVar* dst, IntervalValue elem, IntervalValue len);

    /**
     * @brief Gets the range limit from a type.
     * @param type Pointer to the SVF type.
     * @return The interval value representing the range limit.
     */
    IntervalValue getRangeLimitFromType(const SVFType* type);

    /**
     * @brief Retrieves the abstract state from the trace for a given ICFG node.
     * @param node Pointer to the ICFG node.
     * @return Reference to the abstract state.
     * @throws Assertion if no trace exists for the node.
     */
    AbstractState& getAbsStateFromTrace(const ICFGNode* node);

protected:
    SVFIR* svfir; ///< Pointer to the SVF intermediate representation.
    ICFG* icfg; ///< Pointer to the interprocedural control flow graph.
    Map<const ICFGNode*, AbstractState>& abstractTrace; ///< Map of ICFG nodes to abstract states.
    Map<std::string, std::function<void(const CallICFGNode*)>> func_map; ///< Map of function names to handlers.
};

} // namespace SVF
