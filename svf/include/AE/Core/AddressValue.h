
#ifndef Z3_EXAMPLE_ADDRESSVALUE_H
#define Z3_EXAMPLE_ADDRESSVALUE_H

#define AddressMask 0x7f000000
#define FlippedAddressMask (AddressMask^0xffffffff)
// the address of the black hole, getVirtualMemAddress(2);
#define BlackHoleAddr 0x7f000000 + 2

#include "Util/GeneralType.h"
#include <sstream>

namespace SVF
{
class AddressValue
{
public:
    typedef Set<u32_t> AddrSet;
private:
    AddrSet _addrs;
public:
    /// Default constructor
    AddressValue() {}

    /// Constructor
    AddressValue(const Set<u32_t> &addrs) :  _addrs(addrs) {}

    AddressValue(u32_t addr) :  _addrs({addr}) {}

    /// Default destructor
    ~AddressValue() = default;

    /// Copy constructor
    AddressValue(const AddressValue &other) : _addrs(other._addrs) {}

    /// Move constructor
    AddressValue(AddressValue &&other) noexcept: _addrs(std::move(other._addrs)) {}

    /// Copy operator=
    AddressValue &operator=(const AddressValue &other)
    {
        if (!this->equals(other))
        {
            _addrs = other._addrs;
        }
        return *this;
    }

    /// Move operator=
    AddressValue &operator=(AddressValue &&other) noexcept
    {
        if (this != &other)
        {
            _addrs = std::move(other._addrs);
        }
        return *this;
    }

    bool equals(const AddressValue &rhs) const
    {
        return _addrs == rhs._addrs;
    }

    AddrSet::const_iterator begin() const
    {
        return _addrs.cbegin();
    }

    AddrSet::const_iterator end() const
    {
        return _addrs.cend();
    }

    bool empty() const
    {
        return _addrs.empty();
    }

    u32_t size() const
    {
        return _addrs.size();
    }

    std::pair<AddressValue::AddrSet::iterator, bool> insert(u32_t id)
    {
        return _addrs.insert(id);
    }

    const AddrSet &getVals() const
    {
        return _addrs;
    }

    void setVals(const AddrSet &vals)
    {
        _addrs = vals;
    }

    /// Current AddressValue joins with another AddressValue
    bool join_with(const AddressValue &other)
    {
        bool changed = false;
        for (const auto &addr: other)
        {
            if (!_addrs.count(addr))
            {
                if (insert(addr).second)
                    changed = true;
            }
        }
        return changed;
    }

    /// Return a intersected AddressValue
    bool meet_with(const AddressValue &other)
    {
        AddrSet s;
        for (const auto &id: other._addrs)
        {
            if (_addrs.find(id) != _addrs.end())
            {
                s.insert(id);
            }
        }
        bool changed = (_addrs != s);
        _addrs = std::move(s);
        return changed;
    }

    /// Return true if the AddressValue contains n
    bool contains(u32_t id) const
    {
        return _addrs.count(id);
    }

    bool hasIntersect(const AddressValue &other)
    {
        AddressValue v = *this;
        v.meet_with(other);
        return !v.empty();
    }

    inline bool isTop() const
    {
        return *this->begin() == BlackHoleAddr;
    }

    inline bool isBottom() const
    {
        return empty();
    }

    inline void setTop()
    {
        *this = AddressValue(BlackHoleAddr);
    }

    inline void setBottom()
    {
        _addrs.clear();
    }

    const std::string toString() const
    {
        std::string str;
        std::stringstream rawStr(str);
        if (this->isBottom())
        {
            rawStr << "⊥";
        }
        else
        {
            rawStr << "[";
            for (auto it = _addrs.begin(), eit = _addrs.end(); it!= eit; ++it)
            {
                rawStr << *it << ", ";
            }
            rawStr << "]";
        }
        return rawStr.str();
    }

    /// The physical address starts with 0x7f...... + idx
    static inline u32_t getVirtualMemAddress(u32_t idx)
    {
        // 0 is the null address, should not be used as a virtual address
        assert(idx != 0 && "idx can’t be 0 because it represents a nullptr");
        return AddressMask + idx;
    }

    /// Check bit value of val start with 0x7F000000, filter by 0xFF000000
    static inline bool isVirtualMemAddress(u32_t val)
    {
        return (val & 0xff000000) == AddressMask && val != AddressMask + 0;
    }

    /// Return the internal index if idx is an address otherwise return the value of idx
    static inline u32_t getInternalID(u32_t idx)
    {
        return (idx & FlippedAddressMask);
    }
};
} // end namespace SVF
#endif //Z3_EXAMPLE_ADDRESSVALUE_H
