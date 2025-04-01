
#include "AE/Core/IntervalValue.h"
#include "AE/Core/AddressValue.h"
#include "Util/SVFUtil.h"

namespace SVF
{

class AbstractValue
{

public:
    IntervalValue interval;
    AddressValue addrs;

    AbstractValue()
    {
        interval = IntervalValue::bottom();
        addrs = AddressValue();
    }

    AbstractValue(const AbstractValue& other)
    {
        interval = other.interval;
        addrs = other.addrs;
    }

    inline bool isInterval() const
    {
        return !interval.isBottom();
    }
    inline bool isAddr() const
    {
        return !addrs.isBottom();
    }

    AbstractValue(AbstractValue &&other)
    {
        interval = SVFUtil::move(other.interval);
        addrs = SVFUtil::move(other.addrs);
    }

    // operator overload, supporting both interval and address
    AbstractValue& operator=(const AbstractValue& other)
    {
        interval = other.interval;
        addrs = other.addrs;
        return *this;
    }

    AbstractValue& operator=(const AbstractValue&& other)
    {
        interval = SVFUtil::move(other.interval);
        addrs = SVFUtil::move(other.addrs);
        return *this;
    }

    AbstractValue& operator=(const IntervalValue& other)
    {
        interval = other;
        addrs = AddressValue();
        return *this;
    }

    AbstractValue& operator=(const AddressValue& other)
    {
        addrs = other;
        interval = IntervalValue::bottom();
        return *this;
    }

    AbstractValue(const IntervalValue& ival) : interval(ival), addrs(AddressValue()) {}

    AbstractValue(const AddressValue& addr) : interval(IntervalValue::bottom()), addrs(addr) {}

    IntervalValue& getInterval()
    {
        return interval;
    }

    const IntervalValue getInterval() const
    {
        return interval;
    }

    AddressValue& getAddrs()
    {
        return addrs;
    }

    const AddressValue getAddrs() const
    {
        return addrs;
    }

    ~AbstractValue() {};

    bool equals(const AbstractValue &rhs) const
    {
        return interval.equals(rhs.interval) && addrs.equals(rhs.addrs);
    }

    void join_with(const AbstractValue &other)
    {
        interval.join_with(other.interval);
        addrs.join_with(other.addrs);
    }

    void meet_with(const AbstractValue &other)
    {
        interval.meet_with(other.interval);
        addrs.meet_with(other.addrs);
    }

    void widen_with(const AbstractValue &other)
    {
        interval.widen_with(other.interval);
        // TODO: widen Addrs
    }

    void narrow_with(const AbstractValue &other)
    {
        interval.narrow_with(other.interval);
        // TODO: narrow Addrs
    }

    std::string toString() const
    {
        return "<" + interval.toString() + ", " + addrs.toString() + ">";
    }
};
}