
#ifndef Z3_EXAMPLE_RELEXESTATE_H
#define Z3_EXAMPLE_RELEXESTATE_H

#include "AE/Core/AddressValue.h"
#include "Util/Z3Expr.h"

namespace SVF
{

class RelExeState
{
    friend class SVFIR2AbsState;

public:
    typedef Map<u32_t, Z3Expr> VarToValMap;
    typedef VarToValMap AddrToValMap;

protected:
    VarToValMap _varToVal;
    AddrToValMap _addrToVal;

public:
    RelExeState() = default;

    RelExeState(VarToValMap &varToVal, AddrToValMap&locToVal) : _varToVal(varToVal), _addrToVal(locToVal) {}

    RelExeState(const RelExeState &rhs) : _varToVal(rhs.getVarToVal()), _addrToVal(rhs.getLocToVal())
    {

    }

    virtual ~RelExeState() = default;

    RelExeState &operator=(const RelExeState &rhs);

    RelExeState(RelExeState &&rhs) noexcept: _varToVal(std::move(rhs._varToVal)),
        _addrToVal(std::move(rhs._addrToVal))
    {

    }

    RelExeState &operator=(RelExeState &&rhs) noexcept
    {
        if (&rhs != this)
        {
            _varToVal = std::move(rhs._varToVal);
            _addrToVal = std::move(rhs._addrToVal);
        }
        return *this;
    }

    /// Overloading Operator==
    bool operator==(const RelExeState &rhs) const;

    /// Overloading Operator!=
    inline bool operator!=(const RelExeState &rhs) const
    {
        return !(*this == rhs);
    }

    /// Overloading Operator==
    bool operator<(const RelExeState &rhs) const;


    static z3::context &getContext()
    {
        return Z3Expr::getContext();
    }

    const VarToValMap &getVarToVal() const
    {
        return _varToVal;
    }

    const AddrToValMap&getLocToVal() const
    {
        return _addrToVal;
    }

    inline Z3Expr &operator[](u32_t varId)
    {
        return getZ3Expr(varId);
    }

    u32_t hash() const
    {
        size_t h = getVarToVal().size() * 2;
        SVF::Hash<SVF::u32_t> hf;
        for (const auto &t: getVarToVal())
        {
            h ^= hf(t.first) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hf(t.second.id()) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }

        size_t h2 = getVarToVal().size() * 2;

        for (const auto &t: getLocToVal())
        {
            h2 ^= hf(t.first) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
            h2 ^= hf(t.second.id()) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
        }
        SVF::Hash<std::pair<SVF::u32_t, SVF::u32_t>> pairH;

        return pairH(std::make_pair(h, h2));
    }

    /// Return true if map has varId
    inline bool existsVar(u32_t varId) const
    {
        return _varToVal.count(varId);
    }

    /// Return Z3 expression eagerly based on SVFVar ID
    virtual inline Z3Expr &getZ3Expr(u32_t varId)
    {
        return _varToVal[varId];
    }

    /// Return Z3 expression lazily based on SVFVar ID
    virtual inline Z3Expr toZ3Expr(u32_t varId) const
    {
        return getContext().int_const(std::to_string(varId).c_str());
    }

    /// Extract sub SVFVar IDs of a Z3Expr
    void extractSubVars(const Z3Expr &expr, Set<u32_t> &res);

    /// Extract all related SVFVar IDs based on compare expr
    void extractCmpVars(const Z3Expr &expr, Set<u32_t> &res);

    /// Build relational Z3Expr
    Z3Expr buildRelZ3Expr(u32_t cmp, s32_t succ, Set<u32_t> &vars, Set<u32_t> &initVars);

    /// Store value to location
    void store(const Z3Expr &loc, const Z3Expr &value);

    /// Load value at location
    Z3Expr &load(const Z3Expr &loc);

    /// The physical address starts with 0x7f...... + idx
    static inline u32_t getVirtualMemAddress(u32_t idx)
    {
        return AddressValue::getVirtualMemAddress(idx);
    }

    /// Check bit value of val start with 0x7F000000, filter by 0xFF000000
    static inline bool isVirtualMemAddress(u32_t val)
    {
        if (val == 0)
            assert(false && "val cannot be 0");
        return AddressValue::isVirtualMemAddress(val);
    }

    /// Return the internal index if idx is an address otherwise return the value of idx
    static inline u32_t getInternalID(u32_t idx)
    {
        return AddressValue::getInternalID(idx);
    }

    /// Return int value from an expression if it is a numeral, otherwise return an approximate value
    static inline s32_t z3Expr2NumValue(const Z3Expr &e)
    {
        assert(e.is_numeral() && "not numeral?");
        return e.get_numeral_int64();
    }

    /// Print values of all expressions
    void printExprValues();

private:
    bool eqVarToValMap(const VarToValMap &lhs, const VarToValMap &rhs) const;

    bool lessThanVarToValMap(const VarToValMap &lhs, const VarToValMap &rhs) const;

protected:
    inline void store(u32_t objId, const Z3Expr &z3Expr)
    {
        _addrToVal[objId] = z3Expr.simplify();
    }

    inline Z3Expr &load(u32_t objId)
    {
        return _addrToVal[objId];
    }
}; // end class RelExeState
} // end namespace SVF

template<>
struct std::hash<SVF::RelExeState>
{
    size_t operator()(const SVF::RelExeState &exeState) const
    {
        return exeState.hash();
    }
};

#endif //Z3_EXAMPLE_RELEXESTATE_H
