
#ifndef UTIL_ITERATOR_RANGE_H
#define UTIL_ITERATOR_RANGE_H

#include <utility>

namespace SVF
{

/// A range adaptor for a pair of iterators.
///
/// This just wraps two iterators into a range-compatible interface. Nothing
/// fancy at all.
template <typename IteratorT>
class iter_range
{
    IteratorT begin_iterator, end_iterator;

public:
    //TODO: Add SFINAE to test that the Container's iterators match the range's
    //      iterators.
    template <typename Container>
    iter_range(Container &&c)
    //TODO: Consider ADL/non-member begin/end calls.
        : begin_iterator(c.begin()), end_iterator(c.end()) {}
    iter_range(IteratorT begin_iterator, IteratorT end_iterator)
        : begin_iterator(std::move(begin_iterator)),
          end_iterator(std::move(end_iterator)) {}

    IteratorT begin() const
    {
        return begin_iterator;
    }
    IteratorT end() const
    {
        return end_iterator;
    }
    bool empty() const
    {
        return begin_iterator == end_iterator;
    }
};

/// Convenience function for iterating over sub-ranges.
///
/// This provides a bit of syntactic sugar to make using sub-ranges
/// in for loops a bit easier. Analogous to std::make_pair().
template <class T> iter_range<T> make_range(T x, T y)
{
    return iter_range<T>(std::move(x), std::move(y));
}

template <typename T> iter_range<T> make_range(std::pair<T, T> p)
{
    return iter_range<T>(std::move(p.first), std::move(p.second));
}

}

#endif
