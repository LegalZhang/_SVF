
#ifndef WORKLIST_H_
#define WORKLIST_H_

#include "SVFIR/SVFValue.h"

#include <assert.h>
#include <cstdlib>
#include <vector>
#include <deque>
#include <set>

namespace SVF
{

/**
 * Worklist with "first come first go" order.
 * New nodes pushed at back and popped from front.
 * Elements in the list are unique as they're recorded by Set.
 */
template<class Data>
class List
{
    class ListNode
    {
    public:
        ListNode(const Data &d)
        {
            data = d;
            next = nullptr;
        }

        ~ListNode() {}

        Data data;
        ListNode *next;
    };

    typedef Set<Data> DataSet;
    typedef ListNode Node;

public:
    List()
    {
        head = nullptr;
        tail = nullptr;
    }

    ~List() {}

    inline bool empty() const
    {
        return (head == nullptr);
    }

    inline bool find(const Data &data) const
    {
        return nodeSet.find(data) != nodeSet.end();
    }

    void push(const Data &data)
    {
        if (nodeSet.find(data) == nodeSet.end())
        {
            Node *new_node = new Node(data);
            if (head == nullptr)
                head = new_node;// the list is empty
            else
                tail->next = new_node;
            tail = new_node;
        }
    }

    Data pop()
    {
        assert(head != nullptr && "list is empty");
        /// get node from list head
        Node *head_node = head;

        /// change list head to the next node
        head = head->next;
        if (head == nullptr)
            tail = nullptr;    /// the last node is popped.

        Data data = head_node->data;
        nodeSet.erase(data);
        delete head_node;
        return data;
    }

private:
    DataSet nodeSet;
    Node *head;
    Node *tail;
};

/**
 * Worklist with "first in first out" order.
 * New nodes will be pushed at back and popped from front.
 * Elements in the list are unique as they're recorded by Set.
 */
template<class Data>
class FIFOWorkList
{
    typedef Set<Data> DataSet;
    typedef std::deque<Data> DataDeque;
public:
    FIFOWorkList() {}

    ~FIFOWorkList() {}

    inline bool empty() const
    {
        return data_list.empty();
    }

    inline u32_t size() const
    {
        assert(data_list.size() == data_set.size() && "list and set must be the same size!");
        return data_list.size();
    }

    inline bool find(const Data &data) const
    {
        return data_set.find(data) != data_set.end();
    }

    /**
     * Push a data into the work list.
     */
    inline bool push(const Data &data)
    {
        if (!find(data))
        {
            data_list.push_back(data);
            data_set.insert(data);
            return true;
        }
        else
            return false;
    }

    /**
     * Remove a data from the END of work list, no return value
     */
    inline void removeFront()
    {
        assert(!empty() && "work list is empty");
        data_set.erase(front());
        data_list.pop_front();
    }

    /**
     * Get reference of top data from the END of work list.
     */
    inline Data &front()
    {
        assert(!empty() && "work list is empty");
        Data &data = data_list.front();
        return data;
    }

    /**
     * Pop a data from the END of work list.
     */
    inline Data pop()
    {
        assert(!empty() && "work list is empty");
        Data data = data_list.front();
        data_list.pop_front();
        data_set.erase(data);
        return data;
    }

    /*!
     * Clear all the data
     */
    inline void clear()
    {
        data_list.clear();
        data_set.clear();
    }

private:
    DataSet data_set;    ///< store all data in the work list.
    DataDeque data_list;    ///< work list using std::vector.
};

/**
 * Worklist with "first in last out" order.
 * New nodes will be pushed at back and popped from back.
 * Elements in the list are unique as they're recorded by Set.
 */
template<class Data>
class FILOWorkList
{
    typedef Set<Data> DataSet;
    typedef std::vector<Data> DataVector;
public:
    FILOWorkList() {}

    ~FILOWorkList() {}

    inline bool empty() const
    {
        return data_list.empty();
    }

    inline u32_t size() const
    {
        assert(data_list.size() == data_set.size() && "list and set must be the same size!");
        return data_list.size();
    }

    inline bool find(const Data &data) const
    {
        return data_set.find(data) != data_set.end();;
    }

    /**
     * Push a data into the work list.
     */
    inline bool push(const Data &data)
    {
        if (!find(data))
        {
            data_list.push_back(data);
            data_set.insert(data);
            return true;
        }
        else
            return false;
    }

    /**
     * Pop a data from the END of work list.
     */
    inline Data pop()
    {
        assert(!empty() && "work list is empty");
        Data data = data_list.back();
        data_list.pop_back();
        data_set.erase(data);
        return data;
    }

    /**
     * Remove a data from the END of work list, no return value
     */
    inline void removeBack()
    {
        assert(!empty() && "work list is empty");
        data_set.erase(back());
        data_list.pop_back();
    }

    /**
     * Get reference of top data from the END of work list.
     */
    inline Data &back()
    {
        assert(!empty() && "work list is empty");
        Data &data = data_list.back();
        return data;
    }

    /*!
     * Clear all the data
     */
    inline void clear()
    {
        data_list.clear();
        data_set.clear();
    }

private:
    DataSet data_set;    ///< store all data in the work list.
    DataVector data_list;    ///< work list using std::vector.
};

} // End namespace SVF

#endif /* WORKLIST_H_ */
