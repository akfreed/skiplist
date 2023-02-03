// ===========================================================================
// Copyright (c) 2014-2018 Alexander Freed. ALL RIGHTS RESERVED.
//
// ISO C++11
//
// The skiplist has an insert, remove, and retrieve efficiency of Log2(n) when balanced.
// The height of the skiplist depends on how many elements it contains [ log2(n) ]
// The skiplist will automatically balance itself when performing
// functions that iterate through all the elements.
// 
// ===========================================================================
#pragma once

#include <utility>
#include <type_traits>
#include <iterator>
#include <functional>
#include <initializer_list>
#include <limits>
#include <algorithm>
#include <random>
#include <chrono>
#include <memory>
#include <cmath>
#include <cassert>
#include <stdexcept>

#include "SkiplistPair.hpp"


namespace fsl {


// ------------------------------------------------------------------
// forward declarations

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
class Skiplist;

template <typename CONTAINER>
class SkiplistNonBalancingIterator;

template <typename CONTAINER>
class SkiplistBalancingIterator;


// ------------------------------------------------------------------
// Prototypes

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void swap(Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right) noexcept;

template <typename CONTAINER>
void swap(SkiplistNonBalancingIterator<CONTAINER>& left, SkiplistNonBalancingIterator<CONTAINER>& right) noexcept;

template <typename CONTAINER>
void swap(SkiplistBalancingIterator<CONTAINER>& left, SkiplistBalancingIterator<CONTAINER>& right) noexcept;

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator==(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right);

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator!=(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right);

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator<(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right);

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator>(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right);

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator<=(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right);

template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator>=(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right);


// ==================================================================
// Skiplist Interface

/** Skiplist definition
It is assumed that two keys are equivalent if (!compare(A, B) && !compare(B, A)) is true.
Both KEY and VAL must implement the copy constructor.
MULTIMAP if false, keys must be unique. If true a key may be used more than once. The returned
    value is unspecified.
A compare function may be specified. It should be a binary operator that takes KEY types and returns bool
*/
template <typename KEY, typename VAL, bool MULTIMAP=false, typename COMPARE=std::less<KEY>>
class Skiplist
{
public:
    // Helper Typedefs
    using Meta             = Skiplist<KEY, VAL, MULTIMAP, COMPARE>;
    using Pair             = SLPair<KEY, VAL>;
    using PairConst        = SLPairConst<KEY, VAL>;
    using const_value_type = PairConst;

    // Standard Typedefs
    using size_type        = std::size_t;
    using difference_type  = std::ptrdiff_t;
    using key_type         = KEY;
    using mapped_type      = VAL;
    using value_type       = Pair;
    using pointer          = value_type*;
    using reference        = value_type&;
    using const_pointer    = const const_value_type*;
    using const_reference  = const const_value_type&;
    using iterator         = SkiplistBalancingIterator<Skiplist<KEY, VAL, MULTIMAP, COMPARE>>;
    using const_iterator   = SkiplistNonBalancingIterator<Skiplist<KEY, VAL, MULTIMAP, COMPARE>>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using key_compare      = COMPARE;

    // Create value_compare here, nested in Skiplist
    /** The value_compare callable compares the same as key_compare (it compares the keys).
    The difference is that it operates on value_type instead of key_type.
    */
    class value_compare
    {
        friend class Skiplist;
        key_compare m_comp;  // data member
        explicit value_compare(key_compare comp) : m_comp(std::move(comp)) { }  // constructor
    public:
        bool operator()(const_reference left, const_reference right) const { return m_comp(left.first, right.first); }
    };

    // Dependent Helper Typedefs
    using insert_return    = typename std::conditional<MULTIMAP, iterator, std::pair<iterator, bool>>::type;  // note: this is NOT "insert_return_type" as prescribed by C++17

    // Friend Classes
    friend class SkiplistNonBalancingIterator<Skiplist<KEY, VAL, MULTIMAP, COMPARE>>;
    friend class SkiplistBalancingIterator<Skiplist<KEY, VAL, MULTIMAP, COMPARE>>;
    friend class SkiplistDebug;

    // Friend Functions
    friend void swap<KEY, VAL, MULTIMAP, COMPARE>(Skiplist& left, Skiplist& right) noexcept;

    // Public Static Data
    static const bool s_MULTIMAP = MULTIMAP;

    // Public Static Functions
    static bool is_multimap();

    // Public Functions
    Skiplist();
    Skiplist(const std::initializer_list<std::pair<const key_type, mapped_type>>& initList);
    Skiplist(const Skiplist& other);
    Skiplist(Skiplist&& other) noexcept;
    Skiplist& operator=(const Skiplist& other);
    Skiplist& operator=(Skiplist&& other) noexcept;
    Skiplist& operator=(const std::initializer_list<std::pair<const key_type, mapped_type>>& initList);
    ~Skiplist();

    void swap(Skiplist& right) noexcept;

    iterator        begin();
    const_iterator  begin() const;
    const_iterator cbegin() const;
    iterator        end();
    const_iterator  end() const;
    const_iterator cend() const;
    reverse_iterator        rbegin();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator crbegin() const;
    reverse_iterator        rend();
    const_reverse_iterator  rend() const;
    const_reverse_iterator crend() const;

    bool            empty() const;
    size_type       size() const;
    size_type       max_size() const;
    bool            is_balanced() const;
    key_compare     key_comp() const;
    value_compare   value_comp() const;

    VAL&            operator[](const KEY& key);
    VAL&            operator[](KEY&& key);

    insert_return   insert(const value_type& keyValPair);
    insert_return   insert(value_type&& keyValPair);
    iterator        insert(const_iterator hint, const value_type& keyValPair);
    iterator        insert(const_iterator hint, value_type&& keyValPair);
    template <typename InputIt>
    void            insert(InputIt first, InputIt last);
    void            insert(std::initializer_list<std::pair<const key_type, mapped_type>> initList);
    template <typename ...ARGS>
    insert_return   emplace(ARGS&&... constructionArgs);
    template <typename ...ARGS>
    iterator        emplace_hint(const_iterator hint, ARGS&&... constructionArgs);
    template <typename ...ARGS>
    std::pair<iterator, bool> try_emplace(const KEY& key, ARGS&&... valConstructionArgs);
    template <typename ...ARGS>
    std::pair<iterator, bool> try_emplace(KEY&& key, ARGS&&... valConstructionArgs);
    template <typename ...ARGS>
    iterator        try_emplace(const_iterator hint, const KEY& key, ARGS&&... valConstructionArgs);
    template <typename ...ARGS>
    iterator        try_emplace(const_iterator hint, KEY&& key, ARGS&&... valConstructionArgs);
    iterator        find(const KEY& key);
    const_iterator  find(const KEY& key) const;
    bool            find(const KEY& key, VAL& out_val) const;
    VAL&            at(const KEY& key);
    const VAL&      at(const KEY& key) const;
    bool            contains(const KEY& key) const;
    size_type       count(const KEY& key) const;
    iterator        lower_bound(const KEY& key);
    const_iterator  lower_bound(const KEY& key) const;
    iterator        upper_bound(const KEY& key);
    const_iterator  upper_bound(const KEY& key) const;
    std::pair<iterator, iterator>             equal_range(const KEY& key);
    std::pair<const_iterator, const_iterator> equal_range(const KEY& key) const;
    size_type       erase(const KEY& key);
    iterator        erase(const_iterator pos);
    iterator        erase(const_iterator first, const_iterator last);
    void            clear();

    reference       front();
    const_reference front() const;
    reference       back();
    const_reference back() const;
    void            pop_front();
    void            pop_back();

    void            balance();
    template <typename Functor> void for_each(Functor&& functor);
    template <typename Functor> void for_each(Functor&& functor) const;
    template <typename Functor> void for_each_no_balance(Functor&& functor);
    template <typename Functor> void for_each_no_balance(Functor&& functor) const;

private:
    // The Node for the skiplist
    struct Node
    {
        Pair* KeyValPair = nullptr;
        Node* Next       = nullptr;
        Node* Prev       = nullptr;
        Node* Down       = nullptr;
        Node* Up         = nullptr;
    };

    // Private Functions
    void            updateMinMax();
    int             chooseLevel();
    void            addLevel();
    void            removeLevel();

    template <typename COMP, typename FUNCTOR>
    std::pair<Node*, bool> insertTopDown(const KEY& key, COMP&& compare, FUNCTOR&& getAllocatedPair);
    template <typename COMP>
    bool            insertTopDownRecursive(Node* const head, const KEY& key, COMP&& compare, const int currentLevel, const int targetLevel, Node*& out_node);
    template <typename FUNCTOR>
    iterator        insertWithHintDispatch(const typename Skiplist<KEY, VAL, false, COMPARE>::const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair);
    template <typename FUNCTOR>
    iterator        insertWithHintDispatch(const typename Skiplist<KEY, VAL, true, COMPARE>::const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair);
    template <typename FUNCTOR>
    iterator        insertWithHintMap     (const const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair);
    template <typename FUNCTOR>
    iterator        insertWithHintMultimap(const const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair);
    iterator        insertBottomUp(Node* previous, pointer const pKeyValPair);
    insert_return   insertReturnConvert(Node* node, const bool success);
    Node*           findRecursive(Node* const head, const KEY& key);
    const Node*     findRecursive(const Node* const head, const KEY& key) const;
    template <typename COMP>
    Node*           findRecursive(Node* const head, const KEY& key, COMP&& compare);
    template <typename COMP>
    const Node*     findRecursive(const Node* const head, const KEY& key, COMP&& compare) const;
    size_type       eraseKeyRecursive(Node* toRemove);
    int             calcBalancedLevel(size_type nodeIndex) const;
    template <typename FUNCTOR>
    void            balanceWorker(FUNCTOR&& functor);
    void            insertAbove(Node* current, const int maxDepth, Node* const lowerNode);
    void            eraseAbove(const Node* current);

    // Private Data
    int             m_levelCount = 0;
    size_type       m_count = 0;
    Node*           m_head = nullptr;
    Node*           m_begin = nullptr;
    Node*           m_tail = nullptr;
    size_type       m_countMin = 0;
    size_type       m_countMax = 0;
    bool            m_balanced = true;
    key_compare     m_keyCompare;
    value_compare   m_pairCompare;
    std::unique_ptr<std::mt19937_64>       m_rng;
    std::uniform_real_distribution<double> m_distribution;
};


// ==================================================================
// Skiplist Iterator Interfaces

/** A non-balancing bi-directional iterator.
This iterator will not balance the skiplist as it progresses.
The iterator is invalidated if the element it points to is removed from the
skiplist.
*/
template <typename CONTAINER>
class SkiplistNonBalancingIterator
{
private:
    // Private Typedefs
    using Node = typename CONTAINER::Node;

    // Private Functions
    SkiplistNonBalancingIterator(const CONTAINER& container, const Node* const startNode);

    // Private Data
    const CONTAINER* m_pContainer  = nullptr;
    const Node*      m_currentNode = nullptr;

public:
    // Standard Typedefs
    using value_type        = typename CONTAINER::const_value_type;
    using pointer           = typename CONTAINER::const_pointer;
    using reference         = typename CONTAINER::const_reference;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    // Friends Classes
    friend class CONTAINER::Meta;
    friend class CONTAINER::iterator;

    // Public Functions
    SkiplistNonBalancingIterator() = default;
    SkiplistNonBalancingIterator(const SkiplistNonBalancingIterator&) = default;
    SkiplistNonBalancingIterator& operator=(const SkiplistNonBalancingIterator&) = default;
    SkiplistNonBalancingIterator(const SkiplistBalancingIterator<CONTAINER>& iter);

    // friend swap. Not member function.
    friend void swap<CONTAINER>(SkiplistNonBalancingIterator& left, SkiplistNonBalancingIterator& right) noexcept;

    reference operator*() const;
    pointer   operator->() const;
    bool      operator==(const SkiplistNonBalancingIterator& right) const;
    bool      operator==(const SkiplistBalancingIterator<CONTAINER>& right) const;
    bool      operator!=(const SkiplistNonBalancingIterator& right) const;
    bool      operator!=(const SkiplistBalancingIterator<CONTAINER>& right) const;
    SkiplistNonBalancingIterator& operator++();
    SkiplistNonBalancingIterator  operator++(int);
    SkiplistNonBalancingIterator& operator--();
    SkiplistNonBalancingIterator  operator--(int);
};


// ------------------------------------------------------------------

/** A bi-directional iterator that automatically balances the 
skiplist as it iterates. The iterator will be invalidated if any element
of the skiplist is added or removed.
*/
template <typename CONTAINER>
class SkiplistBalancingIterator
{
private:
    // Private Typedefs
    using Node      = typename CONTAINER::Node;
    using size_type = typename CONTAINER::size_type;  // doesn't have to be private, but this is a convenient spot, and it's currently not used outside the class

    // Private Enums
    enum class StartLocation
    {
        BEGINNING,
        END,
        UNKNOWN
    };

    // Private Functions
    SkiplistBalancingIterator(CONTAINER& container, Node* const startNode, const size_type startNodeIndex);
    SkiplistBalancingIterator(CONTAINER& container, Node* const startNode);
    void balance();

    // Private Data
    CONTAINER*    m_pContainer = nullptr;
    Node*         m_currentNode = nullptr;
    size_type     m_index = 0;
    bool          m_dontBalance = false;
    StartLocation m_startLocation = StartLocation::UNKNOWN;

public:
    // Standard Typedefs
    using value_type        = typename CONTAINER::value_type;
    using pointer           = typename CONTAINER::pointer;
    using reference         = typename CONTAINER::reference;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    // Friend Classes
    friend class CONTAINER::Meta;
    friend class CONTAINER::const_iterator;

    // Public Functions
    SkiplistBalancingIterator() = default;
    SkiplistBalancingIterator(const SkiplistBalancingIterator&) = default;
    SkiplistBalancingIterator& operator=(const SkiplistBalancingIterator&) = default;

    // friend swap. Not member function.
    friend void swap<CONTAINER>(SkiplistBalancingIterator& left, SkiplistBalancingIterator& right) noexcept;

    reference operator*() const;
    pointer   operator->() const;
    bool      operator==(const SkiplistBalancingIterator& right) const;
    bool      operator==(const SkiplistNonBalancingIterator<CONTAINER>& right) const;
    bool      operator!=(const SkiplistBalancingIterator& right) const;
    bool      operator!=(const SkiplistNonBalancingIterator<CONTAINER>& right) const;
    SkiplistBalancingIterator& operator++();
    SkiplistBalancingIterator  operator++(int);
    SkiplistBalancingIterator& operator--();
    SkiplistBalancingIterator  operator--(int);
};


// ==================================================================
// Skiplist Function Definitions


/** Default Constructor
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Skiplist()
    : m_pairCompare(m_keyCompare)
    , m_rng(new std::mt19937_64(std::chrono::steady_clock::now().time_since_epoch().count()))
{ }


/** Initialization List Constructor
Constructs the list from pairs of key/val.
Copies from the values in the initialization list.
@param[in] initList A brace initilizer list consisting of std::pair<const key_type, mapped_type>.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Skiplist(const std::initializer_list<std::pair<const key_type, mapped_type>>& initList)
    : Skiplist()
{
    insert(initList.begin(), initList.end());
}


/** Copy Constructor
Will copy all the elements of the other list, however, the structure may be different. 
In the process of copying, this skiplist will be created balanced.
@param[in] other the Skiplist to copy.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Skiplist(const Skiplist& other)
    : Skiplist()  // default construct before copy
{
    if (!other.m_head)
        return;

    const_iterator otherIter = other.cbegin();
    const_iterator const otherEnd = other.cend();
    // first, copy the entire bottom list, as it contains all the elements

    m_head = new Node;
    Node* current = m_head;
    m_begin = current;  // point begin to the dummy
    // now iterate through that list, copying into ours
    while (otherIter != otherEnd)
    {
        current->Next = new Node;
        current->Next->Prev = current;
        current = current->Next;
        current->KeyValPair = new Pair(*otherIter++);  // copy key and val
        ++m_count;
    }
    // point tail to last item in lowest list
    m_tail = current;
    // begin was pointed to the dummy. Advance it to the first real node
    m_begin = m_begin->Next;

    // figure out how many levels we need
    m_levelCount = static_cast<int>((log(m_count) / log(2)) + 1); 
    // recalculate min/max
    updateMinMax(); 

    // if we have only 1 element, we are done
    if (m_count == 1)
        return;

    // otherwise, we need to fill out the levels

    // Create the heads. In the process we will create an array of tail
    // pointers for all the lists except the lowest one
    Node** tails = new Node*[m_levelCount - 1];
    // connect them vertically
    tails[0] = new Node;  // the head of the 2nd-to-bottom list
    // connect to bottom list
    tails[0]->Down = m_head;
    tails[0]->Down->Up = tails[0];
    // work the way up the levels, allocating the head and linking
    for (int i = 1; i < m_levelCount - 1; ++i)
    {
        tails[i] = new Node;
        tails[i]->Down = tails[i - 1];
        tails[i]->Down->Up = tails[i];
    }

    // point current to the head of the lowest list
    current = m_head; 
    // point head to the head of the top list
    m_head = tails[m_levelCount - 2]; 

    // iterate through the lowest list, adding to the other lists as decided by
    // the balancing function

    // advance to the first real node
    current = current->Next;  

    size_type nodeCount = 1;  // count the head as the first node. The heads already
    int level;                // exist in all the lists, we don't need the 1st element to as well
    Node* toAdd;
    Node dummy;
    while (current)
    {
        // figure out how many levels to add to
        level = calcBalancedLevel(nodeCount++); 
        // add to those levels
        // start with the dummy node
        toAdd = &dummy;
        // start at the highest list and work down to 2nd to bottom
        for (int i = level - 2; i >= 0; --i)
        {
            toAdd->Down = new Node;
            if (toAdd != &dummy)
                toAdd->Down->Up = toAdd;
            toAdd = toAdd->Down;
            toAdd->KeyValPair = current->KeyValPair;
            toAdd->Prev = tails[i];
            tails[i]->Next = toAdd;
            // point the tail to the new tail
            tails[i] = toAdd;  
        }
        // finally, set the last tail's down to the element in the bottom list
        toAdd->Down = current;
        if (toAdd != &dummy)
            toAdd->Down->Up = toAdd;
        current = current->Next;
    }
    delete[] tails;
}


/** Move Constructor
@param[in,out] other The skiplist to move. Will be in a default-constructed state when done.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Skiplist(Skiplist&& other) noexcept
    : Skiplist()  // default construct before swap
{
    swap(other);
}


/** Copy Assignment
Will copy all the elements of the other list, however, the structure may be different.
In the process of copying, this skiplist will be balanced.
@param[in] other The skiplist to copy
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>& Skiplist<KEY, VAL, MULTIMAP, COMPARE>::operator=(const Skiplist& other)
{
    // copy-construct a temp from other. 
    Skiplist temp(other);
    // swap with the temp. When temp goes out of scope, its destructor will deallocate this' old state.
    swap(temp);
    return *this;
}


/** Move Assignment
@param[in,out] other The skiplist to move. Will be in a default-constructed state when done.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>& Skiplist<KEY, VAL, MULTIMAP, COMPARE>::operator=(Skiplist&& other) noexcept
{
    // move-construct a temp from other. 
    Skiplist temp(std::move(other));
    // swap with the temp. When temp goes out of scope, its destructor will deallocate this' old state.
    swap(temp);
    return *this;
}


/** Initialization List Assignment
This function isn't strictly necessary, but it's here for completeness. 
Infers the list from pairs of key/val.
Copies from the values in the initialization list.
@param[in] initList A brace initilizer list consisting of std::pair<const key_type, mapped_type>.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>& Skiplist<KEY, VAL, MULTIMAP, COMPARE>::operator=(const std::initializer_list<std::pair<const key_type, mapped_type>>& initList)
{
    // construct a temp from the initializer list.
    Skiplist temp(initList);
    // swap with the temp. When temp goes out of scope, its destructor will deallocate this' old state.
    swap(temp);
    return *this;
}


/** Member-wise swap of two collections.
friend function of Skiplist, not a member function.
@param[in] left  The Skiplist to swap with right.
@param[in] right The Skiplist to swap with left.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void swap(Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right) noexcept
{
    using std::swap;
    swap(left.m_levelCount,   right.m_levelCount);
    swap(left.m_count,        right.m_count);
    swap(left.m_head,         right.m_head);
    swap(left.m_begin,        right.m_begin);
    swap(left.m_tail,         right.m_tail);
    swap(left.m_countMin,     right.m_countMin);
    swap(left.m_countMax,     right.m_countMax);
    swap(left.m_balanced,     right.m_balanced);
    swap(left.m_keyCompare,   right.m_keyCompare);
    swap(left.m_pairCompare,  right.m_pairCompare);
    swap(left.m_rng,          right.m_rng);
    swap(left.m_distribution, right.m_distribution);
}


/** Member-wise swap of two collections.
member function of Skiplist.
@param[in] right The Skiplist to swap with this one.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::swap(Skiplist& right) noexcept
{
    fsl::swap(*this, right);
}


/** Destructor
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>::~Skiplist()
{
    Node* down;
    Node* next;
    Node* temp;
    while (m_head)
    {
        temp = m_head->Next;
        while (temp)
        {
            next = temp->Next;
            // only delete key and value when on the bottom list
            if (!temp->Down)
                delete temp->KeyValPair;

            delete temp;
            temp = next;
        }
        down = m_head->Down;
        delete m_head;
        m_head = down;
    }
}


// ------------------------------------------------------------------
// Skiplist iterator functions

/** 
@return an iterator to the first element in the skiplist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::begin()
{
    if (!m_head)
        return end();
    return iterator(*this, m_begin, 0);
}


/**
@return a const_iterator to the first element in the skiplist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::begin() const
{
    return cbegin();
}


/**
@return a const_iterator to the first element in the skiplist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::cbegin() const
{
    if (!m_head)
        return cend();
    return const_iterator(*this, m_begin);
}


/**
@return an iterator to the element following the last element in the skiplist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::end()
{
    return iterator(*this, nullptr, m_count);
}


/**
@return a const_iterator to the element following the last element in the skiplist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::end() const
{
    return cend();
}


/**
@return a const_iterator to the element following the last element in the skiplist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::cend() const
{
    return const_iterator(*this, nullptr);
}


/**
@return a reverse_iterator to the first element in the reversed skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::reverse_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::rbegin()
{
    return reverse_iterator(end());
}


/**
@return a const_reverse_iterator to the first element in the reversed skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_reverse_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::rbegin() const
{
    return crbegin();
}


/**
@return a const_reverse_iterator to the first element in the reversed skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_reverse_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::crbegin() const
{
    return const_reverse_iterator(cend());
}


/**
@return a reverse_iterator to the element past the last element in the reversed skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::reverse_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::rend()
{
    return reverse_iterator(begin());
}


/**
@return a const_reverse_iterator to the element past the last element in the reversed skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_reverse_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::rend() const
{
    return crend();
}


/**
@return a const_reverse_iterator to the element past the last element in the reversed skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_reverse_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::crend() const
{
    return const_reverse_iterator(cbegin());
}


// ------------------------------------------------------------------
// Properties

/** Return true if the skiplist is empty.
This doesn't count the items; it has constant time complexity
@return true if the skiplist has no items.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool Skiplist<KEY, VAL, MULTIMAP, COMPARE>::empty() const
{
    return m_count == 0;
}


/** Get the number of items in the skiplist.
This doesn't count the items; it has constant time complexity
@return The number of items in the skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::size_type Skiplist<KEY, VAL, MULTIMAP, COMPARE>::size() const
{
    return m_count;
}


/** Returns the theoretical maximum number of items, but realistically, it's just UINT_MAX.
@return UINT_MAX
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::size_type Skiplist<KEY, VAL, MULTIMAP, COMPARE>::max_size() const
{
    return std::numeric_limits<size_type>::max();
}


/** This will return false if the skiplist has been changed since 
the last time it was balanced.
Constant time complexity.
@return true, if the skiplist is balanced.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool Skiplist<KEY, VAL, MULTIMAP, COMPARE>::is_balanced() const
{
    return m_balanced;
}


/** Returns a copy of the callable used to compare keys.
@return A copy of the callable used to compare keys.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::key_compare Skiplist<KEY, VAL, MULTIMAP, COMPARE>::key_comp() const
{
    return m_keyCompare;
}


/** Returns a copy of the callable used to compare values.
The value_compare callable compares the same as key_compare (it compares the keys).
The difference is that it operates on value_type instead of key_type.
@return A copy of the callable used to compare value_type.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::value_compare Skiplist<KEY, VAL, MULTIMAP, COMPARE>::value_comp() const
{
    return value_compare(m_keyCompare);
}


/** Returns true if the class specialization allows duplicate keys.
static function
@return true, if the class specialization allows duplicate keys.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool Skiplist<KEY, VAL, MULTIMAP, COMPARE>::is_multimap()
{
    return MULTIMAP;
}


// ------------------------------------------------------------------
// Item manipulation functions

/** Given the number of levels of the skip list, calculate the number of items it should hold.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::updateMinMax()
{
    // int overflow (m_levelCount==32) can't happen, as it would mean there are 2^4,000,000,000 elements
    m_countMin = static_cast<size_type>(pow(2.0, m_levelCount - 1));
    m_countMax = static_cast<size_type>(pow(2.0, m_levelCount) - 1);
}


/** Randomly chooses how many levels a node should be in.
Each increased level has half the chance of being selected as the level before it.
@return A number from 1 to the maximum level (inclusive)
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
int Skiplist<KEY, VAL, MULTIMAP, COMPARE>::chooseLevel()
{
    double random = m_distribution(*m_rng);

    // special case for log(0), which will crash
    if (random == 0)  
        return m_levelCount;  // return the maximum possible

    // convert to log base 1/2
    int level = static_cast<int>((log(random) / log(0.5)) + 1);

    // can't be higher than the max level
    if (level > m_levelCount)
        level = m_levelCount;

    return level;
}


/** Add a new level to the top of the skiplist and adjust m_countMin and m_countMax for the new level count.
To keep a good statistically balanced skiplist, this function should be called 
when (and only when) about to add a new new node and m_count == m_countMax
If multiple elements will be added, this function should be called repeatedly until m_countMin <= newCount <= m_countMax
@pre  (number of nodes) == m_countMax
@post m_countMin and m_countMax are increased so m_countMin == (number of nodes)
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::addLevel()
{
    ++m_levelCount;
    // recalculate min/max
    updateMinMax();
    // add a level to head
    Node* const temp = new Node;
    temp->Down = m_head;
    if (m_head)
        m_head->Up = temp;
    m_head = temp;
}


/** Remove the top level of the skiplist and adjust m_countMin and m_countMax for the new level count
To keep a good statistically balanced skiplist, this function should be called 
after (and only after) removing a node and m_count is now less than m_countMin.
If multiple elements have been removed, this function should be called repeatedly until m_countMin <= m_count <= m_countMax
@pre  m_count < m_countMin
@post m_countMin is halved. m_countMax is approximately halved (m_countMax = 2*m_countMin-1). 
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::removeLevel()
{
    assert(m_count < m_countMin);

    --m_levelCount;
    // recalculate min/max
    updateMinMax();
    // deallocate the top list
    Node* topList = m_head;
    m_head = m_head->Down;

    Node* temp;
    while (topList)
    {
        if (topList->Down)
            topList->Down->Up = nullptr;
        temp = topList->Next;
        delete topList;
        topList = temp;
    }
}


/** Remove all items from the skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::clear()
{
    // replace this instance with a default-initialized one
    Skiplist newSkiplist;
    using std::swap;
    // keep rng and distribution
    swap(m_rng, newSkiplist.m_rng);
    swap(m_distribution, newSkiplist.m_distribution);
    // swap the rest
    swap(*this, newSkiplist);
}


/** Add an item.
@param[in] keyValPair The key/value in SLPair to be inserted. An L-value reference to value_type.
                      Will be copied, but only if insertion was successful.
@return If MULTIMAP is false, return a pair consisting of an iterator to the inserted element (or the element that 
            prevented insertion) and a bool indicating if insertion occured. (insertion will not occur if MULTIMAP 
            is false and the key is already in the skiplist.
        If MULTIMAP is true, return an iterator to the inserted element.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert_return Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert(const value_type& keyValPair)
{
    // the allocating lambda copy-constructs the pair only if the insertion is successful
    std::pair<Node*, bool> result = insertTopDown(keyValPair.first, m_keyCompare, [&keyValPair] { return new value_type(keyValPair); });
    // convert result to type insert_return (different for multimaps)
    return insertReturnConvert(result.first, result.second);
}


/** Add an item.
@param[in] keyValPair The key/value in SLPair to be inserted. An R-value reference to value_type. 
                      Will be moved, but only if insertion was successful.
@return If MULTIMAP is false, return a pair consisting of an iterator to the inserted element (or the element that 
            prevented insertion) and a bool indicating if insertion occured. (insertion will not occur if MULTIMAP 
            is false and the key is already in the skiplist.
        If MULTIMAP is true, return an iterator to the inserted element.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert_return Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert(value_type&& keyValPair)
{
    // the allocating lambda move-constructs the pair only if the insertion is successful
    std::pair<Node*, bool> result = insertTopDown(keyValPair.first, m_keyCompare, [&keyValPair]{ return new value_type(std::move(keyValPair)); });
    // convert result to type insert_return (different for multimaps)
    return insertReturnConvert(result.first, result.second);
}


/** Add an item.
This version of insert returns only an iterator, even if MULTIMAP is false (duplicate keys not allowed).
    This is to maintain compatibility with std::insert_iterator.
One can tell if the item was inserted by checking the size of the container before and after the operation.
The given key/value pair will be copied, but only if insertion was successful.
If the hint is correct, insertion occurs immediately and complexity is amortized O(1). If the hint 
    is wrong, a normal searching insert must be done and the complexity is O(log N) (for a balanced 
    skiplist) where N is the number of elements).

Additional notes:
If the hint is wrong, the new element will be inserted as close as possible to it. For a map, this
has no meaning. For a multimap, it means that if the hint is wrong, and compares greater than the new 
item's key, the new item will be inserted at the upper bound of the equal range of keys. If a hint is 
wrong, and compares less than the new item's key, the new item will be inserted at the lower bound of
the equal range of keys. This is the GCC behavior for hinted std::multimap::insert. MSVC has different 
behavior.

@param[in] hint       A const_iterator to the element just after the one to be inserted.
@param[in] keyValPair The key/value in SLPair to be inserted. An L-value reference to value_type.
@return an iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert(const_iterator hint, const value_type& keyValPair)
{
    // the allocating lambda copy-constructs the pair only if the insertion is successful
    return insertWithHintDispatch(hint, keyValPair.first, [&keyValPair] { return new value_type(keyValPair); });
}


/** Add an item.
This version of insert returns only an iterator, even if MULTIMAP is false (duplicate keys not allowed).
    This is to maintain compatibility with std::insert_iterator.
One can tell if the item was inserted by checking the size of the container before and after the operation.
The given key/value pair will be moved, but only if insertion was successful.
If the hint is correct, insertion occurs immediately and complexity is amortized O(1). If the hint
    is wrong, a normal searching insert must be done and the complexity is O(log N) (for a balanced
    skiplist) where N is the number of elements).

Additional notes:
If the hint is wrong, the new element will be inserted as close as possible to it. For a map, this
has no meaning. For a multimap, it means that if the hint is wrong, and compares greater than the new 
item's key, the new item will be inserted at the upper bound of the equal range of keys. If a hint is 
wrong, and compares less than the new item's key, the new item will be inserted at the lower bound of
the equal range of keys. This is the GCC behavior for hinted std::multimap::insert. MSVC has different 
behavior.

@param[in] hint       A const_iterator to the element just after the one to be inserted.
@param[in] keyValPair The key/value in SLPair to be inserted. An R-value reference to value_type. 
@return an iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert(const_iterator hint, value_type&& keyValPair)
{
    // the allocating lambda move-constructs the pair only if the insertion is successful
    return insertWithHintDispatch(hint, keyValPair.first, [&keyValPair] { return new value_type(std::move(keyValPair)); });
}


/** range insert
Insert value_type in range [first, last)
If MULTIMAP is false and multiple items in the range have the same key, only the first is inserted.
InputIt::reference will be passed to the value_type constructor.
@param[in] first An input iterator to the first element to add
@param[in] last  An input iterator to the element after the last one to add
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename InputIt>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert(InputIt first, InputIt last)
{
    const_iterator const hint = cend();
    while (first != last)
        emplace_hint(hint, *first++);
}


/** initializer_list insert
If MULTIMAP is false and multiple items in the range have the same key, only the first is inserted.
@param[in] initList A brace initilizer list consisting of std::pair<const key_type, mapped_type>.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert(std::initializer_list<std::pair<const key_type, mapped_type>> initList)
{
    insert(initList.begin(), initList.end());
}


/** Add an item, constructing it in-place.
Any moved arguments passed in will be moved regardless of the success or failure of insertion.
Similarly, any other arguments will be copied.
If the insertion fails, the created object is destroyed.
@param[in] constructionArgs The arguments to forward to the pair constructor.
@return If MULTIMAP is false, return a pair consisting of an iterator to the inserted element (or the element that 
            prevented insertion) and a bool indicating if insertion occured. (insertion will not occur if MULTIMAP 
            is false and the key is already in the skiplist.
        If MULTIMAP is true, return an iterator to the inserted element.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename ...ARGS>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert_return Skiplist<KEY, VAL, MULTIMAP, COMPARE>::emplace(ARGS&&... constructionArgs)
{
    // construct immediately, forwarding the arguments
    value_type* const pNewPair = new value_type(std::forward<ARGS>(constructionArgs)...);
    // insert
    std::pair<Node*, bool> result = insertTopDown(pNewPair->first, m_keyCompare, [pNewPair]{ return pNewPair; });
    // if insertion was not successful, deallocate the pair
    if (!result.second)
        delete pNewPair;
    // convert result to type insert_return (different for multimaps)
    return insertReturnConvert(result.first, result.second);
}


/** Add an item, constructing it in-place.
Just like the hinted insert overloads, this version of emplace returns only an iterator, even 
    if MULTIMAP is false (duplicate keys not allowed).
One can tell if the item was inserted by checking the size of the container before and after the operation.
Any moved arguments passed in will be moved regardless of the success or failure of insertion.
    Similarly, any other arguments will be copied.
    If the insertion fails, the created object is destroyed.
If the hint is correct, insertion occurs immediately and complexity is amortized O(1). If the hint
    is wrong, a normal searching insert must be done and the complexity is O(log N) (for a balanced
    skiplist) where N is the number of elements).

Additional notes:
If the hint is wrong, the new element will be inserted as close as possible to it. For a map, this
has no meaning. For a multimap, it means that if the hint is wrong, and compares greater than the new 
item's key, the new item will be inserted at the upper bound of the equal range of keys. If a hint is 
wrong, and compares less than the new item's key, the new item will be inserted at the lower bound of
the equal range of keys. This is the GCC behavior for hinted std::multimap::insert. MSVC has different 
behavior.

@param[in] hint             A const_iterator to the element just after the one to be inserted.
@param[in] constructionArgs The arguments to forward to the pair constructor.
@return an iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename ...ARGS>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::emplace_hint(const_iterator hint, ARGS&&... constructionArgs)
{
    // construct immediately, forwarding the arguments
    value_type* const pNewPair = new value_type(std::forward<ARGS>(constructionArgs)...);
    // insert
    const size_type count = m_count;
    iterator retval = insertWithHintDispatch(hint, pNewPair->first, [pNewPair]{ return pNewPair; });
    // if insertion was not successful, deallocate the pair
    if (m_count == count)
        delete pNewPair;
    return retval;
}


namespace fsl_utility {

    /** This functor is used by try_emplace to allocate a new value_type.
    The types should be tuples whose internal types are references.
    They will be move-passed as the 2nd and 3rd arguments to the value_type's piecewise constructor.
    A lambda is normally used, but in C++11, lambda capture doesn't support perfect-forwarding.
    */
    template <typename CONTAINER, typename Tuple1, typename Tuple2>
    struct TryEmplaceConstructor
    {
        /** constructor
        @param[in] t1 An R-value reference to a tuple that will be passed as the 2nd argument to the container's value_type piecewise constructor.
        @param[in] t2 An R-value reference to a tuple that will be passed as the 3rd argument to the container's value_type piecewise constructor.
        */
        TryEmplaceConstructor(Tuple1&& t1, Tuple2&& t2)
            : m_tuple1(std::move(t1))
            , m_tuple2(std::move(t2))
        { }

        /** () overload
        Calling this function may invalidate (by moving) the internal tuples, so it should probably only be called once.
        @return A newly allocated pointer-to-value_type. Its piecewise_constructor will have 
                been called by passing in the tuples given to the functor's constructor.
        */
        typename CONTAINER::pointer operator()()
        {
            return new typename CONTAINER::value_type(std::piecewise_construct, std::move(m_tuple1), std::move(m_tuple2));
        }

    private:
        Tuple1 m_tuple1;
        Tuple2 m_tuple2;
    };


}  // namespace fsl_utility


/** Add an item, constructing it in-place.
If this function fails due to a duplicate key, the arguments passed in by R-value reference
will not be moved and the other arguments will not be copied. (This behavior is unlike the 
normal emplace function, which will move its R-value arguments and copy its other arguments,
even if insertion fails due to a duplicate key, in which case the newly created object will 
be destroyed before returning.)

On success, the new pair will have been constructed as if by a call to
new value_type(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));

This function is available to multimaps, but it never makes sense to use it for a multimap
over the regular emplace function, because multimap insertion does not fail due to duplicate
keys. Therefore, this function always returns an iterator/bool pair.

@param[in] key                 The key for the new element. An L-value reference to key_type. 
                               This will be forwarded (via std::forward_as_tuple) as the 2nd 
                               argument to the pair's piecewise constructor. 
@param[in] valConstructionArgs The arguments forward to the constructor of the mapped_type. 
                               These will be forwarded (via std::forward_as_tuple) as the 3rd
                               argument to the pair's piecewise constructor
@return A pair consisting of an iterator to the inserted element (or the element that 
            prevented insertion) and a bool indicating if insertion occured. 
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename ...ARGS>
std::pair<typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator, bool> Skiplist<KEY, VAL, MULTIMAP, COMPARE>::try_emplace(const KEY& key, ARGS&&... valConstructionArgs)
{
    // instantiate a value_type-allocating functor
    fsl_utility::TryEmplaceConstructor<Meta, std::tuple<const KEY&>, std::tuple<ARGS&&...>> tec(
        std::forward_as_tuple(key), 
        std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));
    // insert
    // the allocating functor piecewise-constructs the pair only if the insertion is successful
    std::pair<Node*, bool> result = insertTopDown(key, m_keyCompare, std::move(tec));

    return std::pair<iterator, bool>(iterator(*this, result.first), result.second);
}


/** Add an item, constructing it in-place.
If this function fails due to a duplicate key, the arguments passed in by R-value reference
will not be moved and the other arguments will not be copied. (This behavior is unlike the 
normal emplace function, which will move its R-value arguments and copy its other arguments,
even if insertion fails due to a duplicate key, in which case the newly created object will 
be destroyed before returning.)

On success, the new pair will have been constructed as if by a call to
new value_type(std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));

This function is available to multimaps, but it never makes sense to use it for a multimap
over the regular emplace function, because multimap insertion does not fail due to duplicate
keys. Therefore, this function always returns an iterator/bool pair.

@param[in] key                 The key for the new element. An R-value reference to key_type. 
                               This will be forwarded (via std::forward_as_tuple) as the 2nd 
                               argument to the pair's piecewise constructor. 
@param[in] valConstructionArgs The arguments forward to the constructor of the mapped_type. 
                               These will be forwarded (via std::forward_as_tuple) as the 3rd
                               argument to the pair's piecewise constructor
@return A pair consisting of an iterator to the inserted element (or the element that 
            prevented insertion) and a bool indicating if insertion occured. 
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename ...ARGS>
std::pair<typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator, bool> Skiplist<KEY, VAL, MULTIMAP, COMPARE>::try_emplace(KEY&& key, ARGS&&... valConstructionArgs)
{
    // instantiate a value_type-allocating functor
    fsl_utility::TryEmplaceConstructor<Meta, std::tuple<KEY&&>, std::tuple<ARGS&&...>> tec(
        std::forward_as_tuple(std::move(key)), 
        std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));
    // insert
    // the allocating functor piecewise-constructs the pair only if the insertion is successful
    std::pair<Node*, bool> result = insertTopDown(key, m_keyCompare, std::move(tec));

    return std::pair<iterator, bool>(iterator(*this, result.first), result.second);
}


/** Add an item, constructing it in-place.
- Just like the hinted insert overloads, this version of emplace returns only an iterator.
- One can tell if the item was inserted by checking the size of the container before and after the operation.
- If this function fails due to a duplicate key, the arguments passed in by R-value reference
      will not be moved and the other arguments will not be copied. (This behavior is unlike the 
      normal emplace function, which will move its R-value arguments and copy its other arguments,
      even if insertion fails due to a duplicate key, in which case the newly created object will 
      be destroyed before returning.)
- On success, the new pair will have been constructed as if by a call to:
      new value_type(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));
- This function is available to multimaps, but it never makes sense to use it for a multimap over
      the regular emplace_hint function, because multimap insertion does not fail due to duplicate keys.
- If the hint is correct, insertion occurs immediately and complexity is amortized O(1). If the hint
      is wrong, a normal searching insert must be done and the complexity is O(log N) (for a balanced
      skiplist) where N is the number of elements).
@param[in] hint                A const_iterator to the element just after the one to be inserted.
@param[in] key                 The key for the new element. An L-value reference to key_type.
                               This will be forwarded (via std::forward_as_tuple) as the 2nd 
                               argument to the pair's piecewise constructor. 
@param[in] valConstructionArgs The arguments forward to the constructor of the mapped_type. 
                               These will be forwarded (via std::forward_as_tuple) as the 3rd
                               argument to the pair's piecewise constructor
@return An iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename ...ARGS>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::try_emplace(const_iterator hint, const KEY& key, ARGS&&... valConstructionArgs)
{
    // instantiate a value_type-allocating functor
    fsl_utility::TryEmplaceConstructor<Meta, std::tuple<const KEY&>, std::tuple<ARGS&&...>> tec(
        std::forward_as_tuple(key), 
        std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));
    // insert
    // the allocating functor piecewise-constructs the pair only if the insertion is successful
    return insertWithHintDispatch(hint, key, std::move(tec));
}


/** Add an item, constructing it in-place.
- Just like the hinted insert overloads, this version of emplace returns only an iterator.
- One can tell if the item was inserted by checking the size of the container before and after the operation.
- If this function fails due to a duplicate key, the arguments passed in by R-value reference
      will not be moved and the other arguments will not be copied. (This behavior is unlike the 
      normal emplace function, which will move its R-value arguments and copy its other arguments,
      even if insertion fails due to a duplicate key, in which case the newly created object will 
      be destroyed before returning.)
- On success, the new pair will have been constructed as if by a call to:
      new value_type(std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));
- This function is available to multimaps, but it never makes sense to use it for a multimap over
      the regular emplace_hint function, because multimap insertion does not fail due to duplicate keys.
- If the hint is correct, insertion occurs immediately and complexity is amortized O(1). If the hint
      is wrong, a normal searching insert must be done and the complexity is O(log N) (for a balanced
      skiplist) where N is the number of elements).
@param[in] hint                A const_iterator to the element just after the one to be inserted.
@param[in] key                 The key for the new element. An R-value reference to key_type.
                               This will be forwarded (via std::forward_as_tuple) as the 2nd 
                               argument to the pair's piecewise constructor. 
@param[in] valConstructionArgs The arguments forward to the constructor of the mapped_type. 
                               These will be forwarded (via std::forward_as_tuple) as the 3rd
                               argument to the pair's piecewise constructor
@return An iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename ...ARGS>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::try_emplace(const_iterator hint, KEY&& key, ARGS&&... valConstructionArgs)
{
    // instantiate a value_type-allocating functor
    fsl_utility::TryEmplaceConstructor<Meta, std::tuple<KEY&&>, std::tuple<ARGS&&...>> tec(
        std::forward_as_tuple(std::move(key)), 
        std::forward_as_tuple(std::forward<ARGS>(valConstructionArgs)...));
    // insert
    // the allocating functor piecewise-constructs the pair only if the insertion is successful
    return insertWithHintDispatch(hint, key, std::move(tec));
}


/** Add an element
Starts at the top node and works its way to the bottom, adding nodes as it goes.
@param[in] key              The key at which the element will be added. This parameter may be 
                            invalidated by calling the getAllocatedPair parameter.
@param[in] compare          A key comparator function. The signature is: (const key_type& toFind, const key_type& currentNode) -> bool
                            The regular key_compare function will insert at the upper bound in a multimap.
                            The hint-insert functions may need to insert at the lower bound. 
@param[in] getAllocatedPair A no-argument callable that returns a pointer to value_type. The returned 
                            pointer shall be a new pointer to value_type, which the new Node will
                            assume ownership of. This callable is only called if insertion is not 
                            blocked by an existing element, and the callable is never called more than 
                            once. Calling this callable may invalidate the key parameter.
@return A pair consisting of a pointer to the inserted Node (or the Node that 
            prevented insertion) and a bool indicating if insertion occured. (insertion 
            will not occur if MULTIMAP is false and the key is already in the skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename COMP, typename FUNCTOR>
std::pair<typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Node*, bool> Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertTopDown(const KEY& key, COMP&& compare, FUNCTOR&& getAllocatedPair)
{
    // figure out how many levels to add to
    int level = chooseLevel();

    // if we are above the maximum size, add a new level
    if (m_count + 1 > m_countMax)
    {
        addLevel();
        // when a new level is created, the node being added automatically goes into the highest level.
        level = m_levelCount;
    }

    // Now add to the levels
    // call recursive worker function
    Node* out_node = nullptr;
    if (insertTopDownRecursive(m_head, key, std::forward<COMP>(compare), m_levelCount, level, out_node))
    {
        assert(out_node);

        // allocate the new element
        Pair* pNewPair = getAllocatedPair();
        // adjust begin
        if (out_node->Next == m_begin)
            m_begin = out_node;
        // adjust tail
        if (!out_node->Next)
            m_tail = out_node;
        // create the return value
        std::pair<Node*, bool> retval(out_node, true);

        // "zip" the nodes.
        while (out_node)
        {
            out_node->Prev->Next = out_node;
            if (out_node->Next)
                out_node->Next->Prev = out_node;
            out_node->KeyValPair = pNewPair;

            out_node = out_node->Up;
        }

        ++m_count;
        m_balanced = false;

        // return
        return retval;
    }


    // We are at this point only if the insertRecursive was unsuccessful because duplicate keys were not allowed.
    // do we need to remove a level?
    if (m_count < m_countMin)
        removeLevel();
    return std::pair<Node*, bool>(out_node, false);
}


/** The recursive worker function that does the insert
If returns true, a chain of Nodes will have been allocated and the caller is responsible for
linking the nodes into the graph. The Nodes are linked vertically. Their Prev and Next pointers
point to the correct nodes, but those nodes do not point back. The caller must use Prev and
Next to link the node into the graph. The caller is also responsible for setting the KeyValPair
pointer.

If this function returns false, the nodes are destroyed by this function.

@param[in]     head         The starting node.
@param[in]     key          The key for the new element.
@param[in]     compare      A key comparator function. The signature is: (const key_type& toFind, const key_type& currentNode) -> bool
@param[in]     currentLevel The current level in the skiplist.
@param[in]     targetLevel  The highest level to insert the new item in. The item will be added to all lists below this as well.
@param[in,out] out_node     A pointer to the new node from the level above. After return, the value changes.
                            After return of true, it will point to the new node in bottom list.
                            After return of false, it will point to the node in the bottom list that blocked it.
@return If true, the node column was inserted and out_node points to the bottom of the node column.
        If false, the node column was not inserted and out_node points to the node that blocked insertion.
        (insertion will not occur if MULTIMAP is false and the key is already in the skiplist.)
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename COMP>
bool Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertTopDownRecursive(Node* const head, const KEY& key, COMP&& compare, const int currentLevel, const int targetLevel, Node*& out_node)
{
    Node* const next = head->Next;
    // if no next or next key is greater than this key, this is the place to insert
    if (!next || compare(key, next->KeyValPair->first))
    {
        // insert a node if we are under or at the first level we are supposed to insert
        if (currentLevel <= targetLevel)
        {
            // allocate node and link vertically
            Node* oldNode = out_node;
            out_node = new Node;
            out_node->Up = oldNode;
            if (oldNode)
                oldNode->Down = out_node;

            // We will not actually insert the node. But its own pointers will point to the correct place.
            out_node->Prev = head;
            out_node->Next = next;
        }

        // must work down to lowest level
        if (head->Down)
            return insertTopDownRecursive(head->Down, key, std::forward<COMP>(compare), currentLevel - 1, targetLevel, out_node);

        // multimap insert is always a success (duplicate keys are allowed)
        if (MULTIMAP)
            return true;

        // If we are here, duplicate keys are not allowed. check equivalency with previous node
        if (!head->Prev || compare(head->KeyValPair->first, key))  // prev node is dummy or is not equivalent. Good to go!
            return true;

        // If we are here, the previous node had an equivalent key and we cannot insert.
        // deallocate the nodes we created
        Node* temp;
        while (out_node)
        {
            temp = out_node;
            out_node = out_node->Up;
            delete temp;
        }
        // point out_node to the blocking node
        out_node = head;
        return false;
    }

    // traverse to next
    return insertTopDownRecursive(head->Next, key, std::forward<COMP>(compare), currentLevel, targetLevel, out_node);
}


/** Helper function calls the correct hint-insert function for either maps or multimaps.
This overload calls the map insertion function.
@param[in] hint             A const_iterator to the element just after the one to be inserted.
@param[in] key              The key at which the element will be added. This parameter may be 
                            invalidated by calling the getAllocatedPair parameter.
@param[in] getAllocatedPair A no-argument callable that returns a pointer to value_type. Calling 
                            this callable may invalidate the key parameter.
@return an iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename FUNCTOR>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertWithHintDispatch(const typename Skiplist<KEY, VAL, false, COMPARE>::const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair)
{
    return insertWithHintMap(hint, key, std::forward<FUNCTOR>(getAllocatedPair));
}


/** Helper function calls the correct hint-insert function for either maps or multimaps.
This overload calls the multimap insertion function.
@param[in] hint             A const_iterator to the element just after the one to be inserted.
@param[in] key              The key at which the element will be added. This parameter may be 
                            invalidated by calling the getAllocatedPair parameter.
@param[in] getAllocatedPair A no-argument callable that returns a pointer to value_type. Calling 
                            this callable may invalidate the key parameter.
@return an iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename FUNCTOR>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertWithHintDispatch(const typename Skiplist<KEY, VAL, true, COMPARE>::const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair)
{
    return insertWithHintMultimap(hint, key, std::forward<FUNCTOR>(getAllocatedPair));
}


/** Worker function performs the insert with hint operation for a map (duplicate keys not allowed).
This function returns only an iterator. No success indicator is given.
One can tell if the item was inserted by checking the size of the container before and after the operation.
The given callable (getAllocatedPair) is only called if the insertion is successful.
If the hint is correct, insertion occurs immediately and complexity is amortized O(1). If the hint
    is wrong, a normal searching insert must be performed and the complexity is O(log N) (for a balanced
    skiplist) where N is the number of elements).
@param[in] hint             A const_iterator to the element just after the one to be inserted.
@param[in] key              The key at which the element will be added. This parameter may be 
                            invalidated by calling the getAllocatedPair parameter.
@param[in] getAllocatedPair A no-argument callable that returns a pointer to value_type. The returned 
                            pointer shall be a new pointer to value_type, which the new Node will
                            assume ownership of. This callable is only called if insertion is not 
                            blocked by an existing element, and the callable is never called more 
                            than once. Calling this callable may invalidate the key parameter.
@return An iterator to the inserted element or the element that prevented insertion.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename FUNCTOR>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertWithHintMap(const const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair)
{
    Node* const b = const_cast<Node*>(hint.m_currentNode);  // const_iterator::m_currentNode always points to a mutable Node. The const is just for const correctness. If the const_iterator is for a const object, the insert function may not be called, as it is not a const function. Therefore, this const_cast is safe.
    // a should be the node immediately preceeding b. If b is past-the-end, then a is the tail.
    Node* const a = b ? b->Prev : m_tail;

    // sanity checks for structure of skiplist:
    // if b is not nullptr, a must not be either
    assert(!b || a);
    // In dummy nodes, KeyValPair and Prev are nullptr. In real data nodes, they may not be nullptr.
    if (a && (a->KeyValPair || a->Prev))
        assert(a->KeyValPair && a->Prev);
    // if b is not nullptr, it must not be a dummy node.
    assert(!b || (b->KeyValPair && b->Prev));

    // if x < b ...
    if (!b || m_keyCompare(key, b->KeyValPair->first))
    {
        // if a < x && x < b, good hint!
        if (!a || !a->KeyValPair || m_keyCompare(a->KeyValPair->first, key))
        {
            // insert here
            return insertBottomUp(a, getAllocatedPair());
        }
        // if x < a, bad hint!
        if (m_keyCompare(key, a->KeyValPair->first))
        {
            // recursive search
            // insertTopDown returns a pair. Our return value doesn't care if the insertion was successful or not, so take just the first value.
            Node* const node = insertTopDown(key, m_keyCompare, std::forward<FUNCTOR>(getAllocatedPair)).first;
            return iterator(*this, node);
        }
        // x == a, blocked by a!
        return iterator(*this, a);
    }
    // if b < x, bad hint!
    if (m_keyCompare(b->KeyValPair->first, key))
    {
        // recursive search
        // insertTopDown returns a pair. Our return value doesn't care if the insertion was successful or not, so take just the first value.
        Node* const node = insertTopDown(key, m_keyCompare, std::forward<FUNCTOR>(getAllocatedPair)).first;
        return iterator(*this, node);
    }
    // b == x, blocked by b!
    return iterator(*this, b);
}


/** Worker function performs the insert with hint operation for a multimap (duplicate keys allowed).
If the hint is correct, insertion occurs immediately and complexity is amortized O(1). If the hint
    is wrong, a normal searching insert must be performed and the complexity is O(log N) (for a balanced
    skiplist) where N is the number of elements).
@param[in] hint             A const_iterator to the element just after the one to be inserted.
@param[in] key              The key at which the element will be added. This parameter may be 
                            invalidated by calling the getAllocatedPair parameter.
@param[in] getAllocatedPair A no-argument callable that returns a pointer to value_type. The returned 
                            pointer shall be a new pointer to value_type, which the new Node will
                            assume ownership of. This callable is never called more than once. 
                            Calling this callable may invalidate the key parameter.
@return An iterator to the inserted element. Multimap insertion does not fail.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename FUNCTOR>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertWithHintMultimap(const const_iterator& hint, const KEY& key, FUNCTOR&& getAllocatedPair)
{
    const Node* const b = hint.m_currentNode;
    // a should be the node immediately preceeding b. If b is past-the-end, then a is the tail.
    Node* const a = b ? b->Prev : m_tail;
    
    // sanity checks for structure of skiplist:
    // if b is not nullptr, a must not be either
    assert(!b || a);
    // In dummy nodes, KeyValPair and Prev are nullptr. In real data nodes, they may not be nullptr.
    if (a && (a->KeyValPair || a->Prev))
        assert(a->KeyValPair && a->Prev);
    // if b is not nullptr, it must not be a dummy node.
    assert(!b || (b->KeyValPair && b->Prev));

    // if x < a or b < x, bad hint! Do recursive search
    if (b && m_keyCompare(b->KeyValPair->first, key))
    {
        // if b < x, insert at the lower bound (negate the compare function).
        const auto compare = [this](const KEY& searchKey, const KEY& nodeKey) { return !m_keyCompare(nodeKey, searchKey); };
        // insertTopDown returns a pair. Multimap insertions never fail, so we can take just the first element.
        Node* const node = insertTopDown(key, compare, std::forward<FUNCTOR>(getAllocatedPair)).first;
        return iterator(*this, node);
    }
    if (a && a->KeyValPair && m_keyCompare(key, a->KeyValPair->first))
    {
        // if x < a, insert at the upper bound (regular compare function).
        // insertTopDown returns a pair. Multimap insertions never fail, so we can take just the first element.
        Node* const node = insertTopDown(key, m_keyCompare, std::forward<FUNCTOR>(getAllocatedPair)).first;
        return iterator(*this, node);
    }
    // a <= x <= b, good hint! Insert here
    return insertBottomUp(a, getAllocatedPair());
}


/** A different insertion algorithm that starts in the bottom list and adds 
to the levels above instead of starting at the top list and working down.
@param[in] previous    A pointer to the node whose Next will be the new node. May be 
                       nullptr or a pointer to a dummy node. In either case, the
                       new node will be the first real node in the skiplist.
@param[in] pKeyValPair A new pointer to value_type that the new nodes will assume
                       ownership of.
@return
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertBottomUp(Node* previous, pointer const pKeyValPair)
{
    ++m_count;
    m_balanced = false;

    // figure out how many levels to add to
    int level = chooseLevel();

    // if we are above the maximum size, add a new level
    if (m_count > m_countMax)
    {
        addLevel();
        // when a new level is created, the node being added automatically goes into the highest level.
        level = m_levelCount;
    }

    // special case: this is the first node
    if (!previous)
        previous = m_head;

    // Now add to the levels
    // allocate and link first level
    Node* const pNewNode = new Node;
    pNewNode->Next = previous->Next;
    pNewNode->Prev = previous;
    if (pNewNode->Next)
        pNewNode->Next->Prev = pNewNode;
    else
        m_tail = pNewNode;
    previous->Next = pNewNode;
    pNewNode->KeyValPair = pKeyValPair;
    if (!previous->Prev)
        m_begin = pNewNode;

    // insert into upper levels
    if (level > 1) 
        insertAbove(previous, level - 1, pNewNode);

    return iterator(*this, pNewNode);
}


/** returns an std::pair<iterator, bool> or iterator depending on if the container is a map or multimap.
@param[in] node    A node from which to construct an iterator. Must be from the bottom list.
@param[in] success Indicates if the insert operation was successful or not. Ignored for multimaps.
@return If MULTIMAP is false, return an std::pair<iterator, bool>, where the iterator 
            is an iterator to the inserted element (in which case the bool is true) or 
            to the element that prevented insertion (in which case the bool is false).
        If MULTIMAP is true, return an iterator to the inserted item.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insert_return Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertReturnConvert(Node* const node, const bool success)
{
    // functor for returning an iterator
    struct MultimapConverter 
    { 
        iterator operator()(Meta& container, Node* const node, const bool) const { return iterator(container, node); } 
    };
    // functor for returning an std::pair<iterator, bool>
    struct MapConverter
    { 
        std::pair<iterator, bool> operator()(Meta& container, Node* const node, const bool success) const { return std::pair<iterator, bool>(iterator(container, node), success); }
    };
    // construct a converter functor of the appropriate type, depending on if MULTIMAP is true or false.
    typename std::conditional<MULTIMAP, MultimapConverter, MapConverter>::type converter;
    // return the appropriate type
    return converter(*this, node, success);
}


/** Subcript Operator
This function is equivalent to `return try_emplace(key).first->second`
See the try_emplace comment header for details.
@param[in] key The key to search for.
@return A reference to the mapped value at the given key. If no element had the 
        given key, a new one is inserted (default-constructed).
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
VAL& Skiplist<KEY, VAL, MULTIMAP, COMPARE>::operator[](const KEY& key)
{
    const_iterator const hint = lower_bound(key);
    return try_emplace(hint, key)->second;
}


/** Subcript Operator
This function is equivalent to `return try_emplace(std::move(key)).first->second`
See the try_emplace comment header for details.
@param[in] key The key to search for.
@return A reference to the mapped value at the given key. If no element had the 
        given key, a new one is inserted (default-constructed).
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
VAL& Skiplist<KEY, VAL, MULTIMAP, COMPARE>::operator[](KEY&& key)
{
    const_iterator const hint = lower_bound(key);
    return try_emplace(hint, std::move(key))->second;
}


/** Retrieve a const_iterator for a key/value pair from the skiplist
If there are several elements with the same key, any one of them may be returned.
Wrapper for recursive search
@param[in] key The key to search for.
@return A const_iterator for the key/value pair with the given key. If the key is not in the skiplist, cend() is returned.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::find(const KEY& key) const
{
    const Node* node = findRecursive(m_head, key);
    if (!node)
        return cend();
    // return an iterator to the node in the lowest list
    while (node->Down)
        node = node->Down;
    return const_iterator(*this, node);
}
/** Retrieve an iterator for a key/value pair from the skiplist
If there are several elements with the same key, any one of them may be returned.
Wrapper for recursive search
@param[in] key The key to search for.
@return An iterator for the key/value pair with the given key. If the key is not in the skiplist, end() is returned.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::find(const KEY& key)
{
    Node* node = findRecursive(m_head, key);
    if (!node)
        return end();
    // return an iterator to the node in the lowest list
    while (node->Down)
        node = node->Down;
    return iterator(*this, node);
}


/** Retrieve a copy of a value from the skiplist
If there are several elements with the same key, any one of them may be returned.
Wrapper for recursive search
@param[in]  key     The key to search for.
@param[out] out_val A variable in which to copy the value with the given key. Must implement operator=
@return true if the item was found. false if not found.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool Skiplist<KEY, VAL, MULTIMAP, COMPARE>::find(const KEY& key, VAL& out_val) const
{
    const Node* const node = findRecursive(m_head, key);
    if (!node)
        return false;
    out_val = node->KeyValPair->second;
    return true;
}


/** recursive worker function that does the search (const version)
If there are several elements with the same key, any one of them may be returned.
This function returns the first matching node it matches (or nullptr).
@param[in] head The current node.
@param[in] key  The key to search for.
@return A pointer (to const) to the highest node with the given key. returns nullptr if the key was not found.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
const typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Node* Skiplist<KEY, VAL, MULTIMAP, COMPARE>::findRecursive(const Node* const head, const KEY& key) const
{
    if (!head)
        return nullptr;
    const Node* const next = head->Next;
    if (!next || m_keyCompare(key, next->KeyValPair->first))  // went too far on this level
        return findRecursive(head->Down, key);  // go down
    // It is assumed that two keys are equivalent if (!compare(A, B) && !compare(B, A)) is true.
    if (!m_keyCompare(next->KeyValPair->first, key))  // match found
        return next;
    return findRecursive(head->Next, key);
}
/** recursive worker function that does the search (non-const version)
If there are several elements with the same key, any one of them may be returned.
This function returns the first matching node it matches (or nullptr).
@param[in] head The current node. Not modified by this function.
@param[in] key  The key to search for.
@return A pointer to the highest node with the given key. returns nullptr if the key was not found.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Node* Skiplist<KEY, VAL, MULTIMAP, COMPARE>::findRecursive(Node* const head, const KEY& key)
{
    if (!head)
        return nullptr;
    Node* const next = head->Next;
    if (!next || m_keyCompare(key, next->KeyValPair->first))  // went too far on this level
        return findRecursive(head->Down, key);  // go down
    // It is assumed that two keys are equivalent if (!compare(A, B) && !compare(B, A)) is true.
    if (!m_keyCompare(next->KeyValPair->first, key))  // match found
        return next;
    return findRecursive(head->Next, key);
}


/** recursive worker function that does the search (const version)
This overload is used for both lower_bound and upper_bound.
This function always returns a node in the bottom list (or nullptr).
@param[in] head    The current node.
@param[in] key     The key to search for.
@param[in] compare A key comparator function. The signature is: (const key_type& toFind, const key_type& currentNode) -> bool
@return A pointer (to const) to the a node in the bottom list that is the first node for which the 
        compare function returns true. returns nullptr if the compare function never returned true.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename COMP>
const typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Node* Skiplist<KEY, VAL, MULTIMAP, COMPARE>::findRecursive(const Node* const head, const KEY& key, COMP&& compare) const
{
    if (!head)
        return nullptr;
    const Node* const next = head->Next;
    if (!next || compare(key, next->KeyValPair->first))
    {
        // if not in the bottom list, go down
        if (head->Down)
            return findRecursive(head->Down, key, std::forward<COMP>(compare));  // go down
        // if in the bottom list, next is the correct node.
        return next;
    }
    return findRecursive(head->Next, key, std::forward<COMP>(compare));  // go next
}
/** recursive worker function that does the search (non-const version)
This overload is used for both lower_bound and upper_bound.
This function always returns a node in the bottom list (or nullptr).
@param[in] head    The current node.
@param[in] key     The key to search for.
@param[in] compare A key comparator function. The signature is: (const key_type& toFind, const key_type& currentNode) -> bool
@return A pointer to the a node in the bottom list that is the first node for which the
        compare function returns true. returns nullptr if the compare function never returned true.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename COMP>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::Node* Skiplist<KEY, VAL, MULTIMAP, COMPARE>::findRecursive(Node* const head, const KEY& key, COMP&& compare)
{
    if (!head)
        return nullptr;
    Node* const next = head->Next;
    if (!next || compare(key, next->KeyValPair->first))
    {
        // if not in the bottom list, go down
        if (head->Down)
            return findRecursive(head->Down, key, std::forward<COMP>(compare));  // go down
        // if in the bottom list, next is the correct node.
        return next;
    }
    return findRecursive(head->Next, key, std::forward<COMP>(compare));  // go next
}


/** (const version) Get a reference to the value at the given key.
If the key does not exist, throw std::out_of_range
@param[in] key the key at which to find the value
@return a reference to the value at the given key
@throw std::out_of_range if the key does not exist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
const VAL& Skiplist<KEY, VAL, MULTIMAP, COMPARE>::at(const KEY& key) const
{
    const Node* const node = findRecursive(m_head, key);
    if (!node)
        throw std::out_of_range("Invalid Skiplist<KEY, VAL> key.");
    return node->KeyValPair->second;
}
/** (non-const version) Get a reference to the value at the given key.
If the key does not exist, throw std::out_of_range
@param[in] key the key at which to find the value
@return a reference to the value at the given key
@throw std::out_of_range if the key does not exist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
VAL& Skiplist<KEY, VAL, MULTIMAP, COMPARE>::at(const KEY& key)
{
    Node* const node = findRecursive(m_head, key);
    if (!node)
        throw std::out_of_range("Invalid Skiplist<KEY, VAL> key.");
    return node->KeyValPair->second;
}


/** Returns true if the key exists in the skiplist
@param[in] key The key to search for.
@return true if an item that matches the key is in the skiplist
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool Skiplist<KEY, VAL, MULTIMAP, COMPARE>::contains(const KEY& key) const
{
    return (findRecursive(m_head, key) != nullptr);
}


/** Returns the number of items matching the given key.
The count is 0 or 1 for a non-multimap.
@param[in] key The key to search for.
@return The number of items in the skiplist that match the key
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::size_type Skiplist<KEY, VAL, MULTIMAP, COMPARE>::count(const KEY& key) const
{
    // get the lower bound node
    const Node* current = lower_bound(key).m_currentNode;
    // start counting
    size_type count = 0;
    while (current && !m_keyCompare(key, current->KeyValPair->first))
    {
        ++count;
        current = current->Next;
    }
    return count;
}


/** (const version) Returns a const_iterator to the first element not less than key
aka >= greater than or equal to (assuming std::less is the comparator)
@param[in] key The key to search for.
@return a const_iterator to the first element not less than key, or end() if no element is not less than key.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::lower_bound(const KEY& key) const
{
    // negate the normal key compare
    const auto compare = [this](const KEY& searchKey, const KEY& nodeKey) { return !m_keyCompare(nodeKey, searchKey); };
    const Node* const node = findRecursive(m_head, key, compare);
    return const_iterator(*this, node);
}
/** (non-const version) Returns an iterator to the first element not less than key
aka >= greater than or equal to (assuming std::less is the comparator)
@param[in] key The key to search for.
@return An iterator to the first element not less than key, or end() if no element is not less than key.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::lower_bound(const KEY& key)
{
    // negate the normal key compare
    const auto compare = [this](const KEY& searchKey, const KEY& nodeKey) { return !m_keyCompare(nodeKey, searchKey); };
    Node* const node = findRecursive(m_head, key, compare);
    return iterator(*this, node);
}


/** (const version) Returns a const_iterator to the first element greater than key
> greater than (assuming std::less is the comparator)
@param[in] key The key to search for.
@return a const_iterator to the first element greater than key, or end() if no element is greater than key.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::upper_bound(const KEY& key) const
{
    const Node* const node = findRecursive(m_head, key, m_keyCompare);
    return const_iterator(*this, node);
}
/** (non-const version) Returns an iterator to the first element greater than key
> greater than (assuming std::less is the comparator)
@param[in] key The key to search for.
@return an iterator to the first element greater than key, or end() if no element is greater than key.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::upper_bound(const KEY& key)
{
    Node* const node = findRecursive(m_head, key, m_keyCompare);
    return iterator(*this, node);
}


/** (const version) Returns a range of all elements with the given key.
@param[in] key The key to search for.
@return A std::pair of iterators.
            The first iterator is the first element not less than key (i.e. element >= key)
            The second iterator is the first element greater than key (i.e. element >  key)
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
std::pair<typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator, typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator>
Skiplist<KEY, VAL, MULTIMAP, COMPARE>::equal_range(const KEY& key) const
{
    return std::make_pair(lower_bound(key), upper_bound(key));
}
/** (non-const version) Returns a range of all elements with the given key.
@param[in] key The key to search for.
@return A std::pair of const_iterators.
            The first const_iterator is the first element not less than key (i.e. element >= key)
            The second const_iterator is the first element greater than key (i.e. element >  key)
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
std::pair<typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator, typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator>
    Skiplist<KEY, VAL, MULTIMAP, COMPARE>::equal_range(const KEY& key)
{
    return std::make_pair(lower_bound(key), upper_bound(key));
}


/** Removes all elements that match the given key.
Wrapper for recursive removal
@param[in] key The key to search for.
@return The number of items removed.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::size_type Skiplist<KEY, VAL, MULTIMAP, COMPARE>::erase(const KEY& key)
{
    // get the node with the matching key
    Node* const toRemove = findRecursive(m_head, key);
    if (!toRemove)
        return 0;

    // call the recursive worker function
    const size_type numRemoved = eraseKeyRecursive(toRemove);
    // If the remove function returns 0, it could mean data corruption.
    // The key was found, but nothing was erased.
    assert(numRemoved > 0);

    assert(numRemoved <= m_count);
    m_count -= numRemoved;

    // special case: set tail to null if no items
    if (m_count == 0)
    {
        assert(!m_tail->Next && !m_tail->Down && !m_tail->Prev);
        assert(!m_begin);
        m_tail = nullptr;
    }

    m_balanced = false;
    // do we need to remove a level (or more)?
    while (m_count < m_countMin)
        removeLevel();

    return numRemoved;
}


/** function does the actual removal
Removes all nodes that match the key of the given node.
Should be given a pointer to the highest node with the same key.
If multiple nodes tie for highest, either will do. (Traverses left and right.)
@param[in] toRemove The node to remove plus all nodes with equivalent keys previous, next, and in the levels below.
@return the number of items removed
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::size_type Skiplist<KEY, VAL, MULTIMAP, COMPARE>::eraseKeyRecursive(Node* toRemove)
{
    if (!toRemove)
        return 0;

    size_type numRemoved = 0;
    const Pair& kvPair = *toRemove->KeyValPair;
    Node* const down = toRemove->Down;

    // deallocate nodes to the left of toRemove with equivalent keys
    const Node* temp;
    assert(toRemove->Prev);
    while (toRemove->Prev->Prev && !m_pairCompare(*toRemove->Prev->KeyValPair, kvPair))
    {
        // unlink
        temp = toRemove->Prev;
        toRemove->Prev = toRemove->Prev->Prev;
        // if we are in the bottom list, deallocate the key and value
        // m_begin/m_tail will be updated later
        if (!down)
        {
            delete temp->KeyValPair;
            ++numRemoved;
        }
        delete temp;
    }

    // deallocate nodes to the right of toRemove with equivalent keys
    while (toRemove->Next && !m_pairCompare(kvPair, *toRemove->Next->KeyValPair))
    {
        // unlink
        temp = toRemove->Next;
        toRemove->Next = toRemove->Next->Next;
        // if we are in the bottom list, deallocate the key and value
        // m_begin/m_tail will be updated later
        if (!down)
        {
            delete temp->KeyValPair;
            ++numRemoved;
        }
        delete temp;
    }

    // deallocate toRemove
    toRemove->Prev->Next = toRemove->Next;
    if (toRemove->Next)
        toRemove->Next->Prev = toRemove->Prev;
    if (!down)
    {
        delete toRemove->KeyValPair;
        ++numRemoved;
        // update begin
        if (!toRemove->Prev->Prev)
            m_begin = toRemove->Next;
        // update tail
        if (!toRemove->Next)
            m_tail = toRemove->Prev;
    }
    delete toRemove;

    // recursive call to erase nodes in lower level
    return down ? eraseKeyRecursive(down) : numRemoved;
}


/** Removes the specified element from the skiplist.
The iterator must be valid and dereferenceable. Thus, the end iterator is not allowed.
This operation invalidates the iterator.
@param[in] pos A valid and dereferenceable iterator to an element in the skiplist to remove.
@return the iterator following the removed element.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::erase(const_iterator pos)
{
    const Node* const current = pos.m_currentNode;
    // pos must be valid and dereferenceable. The end iterator is not valid. It must be from this collection.
    if (!current || pos.m_pContainer != this)
    {
        assert(false);
        return end();
    }

    // the return value is the next node after the erased one
    const iterator retVal(*this, current->Next);

    assert(current->Prev);
    // erase nodes above
    eraseAbove(current);
    // unlink current node
    current->Prev->Next = current->Next;
    if (current->Next)
        current->Next->Prev = current->Prev;
    else
        m_tail = current->Prev;
    if (current == m_begin)
        m_begin = current->Next;
    // deallocate
    delete current->KeyValPair;
    delete current;

    --m_count;

    // special case: set tail to null if no items
    if (m_count == 0)
    {
        assert(!m_tail->Next && !m_tail->Down);
        assert(!m_begin);
        m_tail = nullptr;
    }

    m_balanced = false;
    // do we need to remove a level?
    if (m_count < m_countMin)
        removeLevel();

    return retVal;
}


/** range erase
Must be a valid range in the container.
Iterators to removed elements are invalidated.
@param[in] first An iterator to the first element in the skiplist to remove
@param[in] last  An iterator to the element after the last one to be removed
@return the iterator following the last removed element
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::iterator Skiplist<KEY, VAL, MULTIMAP, COMPARE>::erase(const_iterator first, const_iterator last)
{
    // sanity check
    if (first.m_pContainer != this || last.m_pContainer != this)
    {
        assert(false);
        return end();
    }

    // erase
    while (first != last)
        erase(first++);
    
    // return iterator
    Node* const node = const_cast<Node*>(first.m_currentNode);  // const_iterator::m_currentNode always points to a mutable Node. The const is just for const correctness. If the const_iterator is for a const object, the insert function may not be called, as it is not a const function. Therefore, this const_cast is safe.
    return iterator(*this, node);
}


/** Return the first item in the skiplist. (const version)
The skiplist must not be empty.
@return The first item in the skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_reference Skiplist<KEY, VAL, MULTIMAP, COMPARE>::front() const
{
    assert(m_head);
    return *cbegin();
}
/** Return the first item in the skiplist. (non-const version)
The skiplist must not be empty.
@return The first item in the skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::reference Skiplist<KEY, VAL, MULTIMAP, COMPARE>::front()
{
    assert(m_head);
    return *begin();
}


/** Return the last item in the skiplist. (const version)
The skiplist must not be empty.
@return The last item in the skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_reference Skiplist<KEY, VAL, MULTIMAP, COMPARE>::back() const
{
    assert(m_tail && m_tail->KeyValPair);
    return *m_tail->KeyValPair;
}
/** Return the last item in the skiplist. (non-const version)
The skiplist must not be empty.
@return The last item in the skiplist.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::reference Skiplist<KEY, VAL, MULTIMAP, COMPARE>::back()
{
    assert(m_tail && m_tail->KeyValPair);
    return *m_tail->KeyValPair;
}


/** Remove first item in the skiplist.
The skiplist must not be empty.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::pop_front()
{
    if (!m_head)
    {
        assert(false);
        return;
    }
    erase(cbegin());
}


/** Remove first item in the skiplist.
The skiplist must not be empty.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::pop_back()
{
    if (!m_tail)
    {
        assert(false);
        return;
    }
    assert(m_tail->KeyValPair);

    const_iterator iter(*this, m_tail);
    erase(iter);
}


// ------------------------------------------------------------------
// balancing functions

/** Calculate the appropriate level for the i-th item of a skiplist that is balanced.
@param[in] nodeIndex The index of a hypothetical item in the skiplist.
@return The highest level list the item should occupy. Range is from 1 to the highest level (inclusive)
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
int Skiplist<KEY, VAL, MULTIMAP, COMPARE>::calcBalancedLevel(size_type nodeIndex) const
{
    assert(nodeIndex >= 0);
    // the 0th node technically would be infinite height, so return the highest level
    if (nodeIndex == 0)
        return m_levelCount;
    // start counting at 1, since we special-cased 0
    int level = 1;
    // find how many times nodeIndex can be divided by 2 before the result is an odd number
    while (nodeIndex % 2 == 0)  // while node_index is even
    {
        ++level;
        nodeIndex /= 2;
    }
    return level;
}


/** Balance the skip list levels to even out the performance.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::balance()
{
    // call balanceWorker with a functor that does nothing
    balanceWorker([](value_type&) {});
}


/** Balance the skip list so that:
      level 1 contains every node,
      level 2 contains every other node,
      level 3 contains every 4th node,
      level 4 contains every 8th node, etc.

Since this function iterates through all the nodes, it also allows for a
functor to be passed to it. It will call the call the functor for
every value in the list, passing a value_type to the function.
@param[in] functor A callable that accepts a single argument of type value_type
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename FUNCTOR>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::balanceWorker(FUNCTOR&& functor)
{
    if (m_balanced)
        return;
    m_balanced = true;

    if (!m_head)
        return;

    // create an array of tail pointers for all the lists, except the lowest one
    Node** const tails = new Node*[m_levelCount - 1];

    Node* levelHead = m_head;
    Node* current;
    Node* temp;
    int i = m_levelCount - 2;

    // traverse from the top list to the 2nd-to-bottom list
    // erase all nodes from every level except lowest (keep the heads, though)
    // also build the array of tail pointers
    while (levelHead->Down) 
    {
        current = levelHead->Next;
        // deallocate this level
        while (current)
        {
            temp = current->Next;
            delete current;
            current = temp;
        }
        // set the head as the tail
        tails[i--] = levelHead;
        // descend a level
        levelHead = levelHead->Down;
    }
    // levelHead is now pointing to the lowest list
    // advance to the first real node
    current = levelHead->Next; 

    // Iterate through the lowest list, adding to the other lists as decided by the balancing function.
    int level;
    Node* toAdd;
    Node dummy;
    // Start from index 1. The heads (index 0) exist in all the lists. The 1st element (index 1) shall exist only in the bottom list.
    size_type nodeCount = 1;
    
    while (current)
    {
        // figure out how many levels to add this item to
        level = calcBalancedLevel(nodeCount++); 
        // add to those levels, starting with the highest and working down to 2nd-to-bottom.
        // start with the dummy node.
        toAdd = &dummy;
        for (i = level - 2; i >= 0; --i)  
        {
            toAdd->Down = new Node;
            if (toAdd != &dummy)
                toAdd->Down->Up = toAdd;
            toAdd = toAdd->Down;
            toAdd->KeyValPair = current->KeyValPair;
            toAdd->Prev = tails[i];
            tails[i]->Next = toAdd;
            // point the tail to the new tail
            tails[i] = toAdd;
        }
        // finally, set the last tail's down to the element in the bottom list
        toAdd->Down = current;
        if (toAdd != &dummy)
            current->Up = toAdd;
        else
            current->Up = nullptr;

        functor(*current->KeyValPair);

        current = current->Next;
    }
    delete[] tails;
}


/** Add N new nodes above the given node that link to the same key/val pair.
@param[in]     current   Must be one of the following:
                             - The first node preceding lowerNode that has a non-NULL Up pointer
                             - lowerNode itself
                             - any node between.
@param[in]     maxDepth  The number of nodes above this one to add.
@param[in,out] lowerNode The node that will be "below" the soon-to-be-added nodes
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::insertAbove(Node* current, const int maxDepth, Node* const lowerNode)
{
    if (maxDepth <= 0)
        return;
    if (!current || !lowerNode)
    {
        assert(false);
        return;
    }

    // special case. Node already has an up! Just do recursive call.
    // This extra feature isn't used by the iterator balance function.
    if (current == lowerNode && lowerNode->Up)
    {
        insertAbove(lowerNode->Up, maxDepth - 1, lowerNode->Up);
        return;
    }

    // traverse left until a node can take us up
    while (current && !current->Up)
        current = current->Prev;
    if (!current)  // reached the very top of the skiplist
        return;

    // rise a level
    current = current->Up;
    // The appropriate place to insert is after current. We know this because of how the skiplist works.

    // allocate
    Node* const pNewNode = new Node;
    // link
    pNewNode->KeyValPair = lowerNode->KeyValPair;
    pNewNode->Next = current->Next;
    pNewNode->Prev = current;
    current->Next = pNewNode;
    if (pNewNode->Next)
        pNewNode->Next->Prev = pNewNode;
    pNewNode->Down = lowerNode;
    lowerNode->Up = pNewNode;

    // recursive call
    insertAbove(current, maxDepth - 1, pNewNode);
}


/** Call to remove (in the column) all the nodes above this one.
@param[in] current The soon-to-be highest node in the column.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::eraseAbove(const Node* current)
{
    if (!current || !current->Up)
        return;

    // rise to upper list.
    current = current->Up;
    // do recursive call first
    eraseAbove(current);

    // unlink
    current->Prev->Next = current->Next;
    if (current->Next)
        current->Next->Prev = current->Prev;
    // deallocate
    delete current;
}


/** (const version) Iterate through the list and invoke the functor for
each element in the list in order, passing a const_reference to it.
This function will not balance the list.
@param[in] functor A callable that accepts a single argument of type const_reference
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename Functor>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::for_each_no_balance(Functor&& functor) const
{
    const_iterator iter = cbegin();
    const_iterator const stop = cend();
    while (iter != stop)
        functor(*iter++);
}


/** (non-const version) Iterate through the list and invoke the functor for
each element in the list in order, passing a value_type to it.
This function will not balance the list.
@param[in] functor A callable that accepts a single argument of type value_type
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename Functor>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::for_each_no_balance(Functor&& functor)
{
    iterator iter = begin();
    iterator const stop = end();

    // don't want the iterator's increment function to automatically balance
    iter.m_dontBalance = true;

    while (iter != stop)
        functor(*iter++);
}


/** (const version) Iterate through the list and activate the functor for
each element in the list in order, passing a const_reference to it.
This function will not balance the skiplist.
@param[in] functor A callable that accepts a single argument of type const_reference
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename Functor>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::for_each(Functor&& functor) const
{
    for_each_no_balance(functor);
}


/** (non-const version) Iterate through the list and activate the functor for
each element in the list in order, passing a value_type to it.
This function will balance the list if it is not balanced
@param[in] functor A callable that accepts a single argument of type value_type
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
template <typename Functor>
void Skiplist<KEY, VAL, MULTIMAP, COMPARE>::for_each(Functor&& functor)
{
    if (m_balanced)
        for_each_no_balance(functor);
    else
        balanceWorker(functor);
}


// ------------------------------------------------------------------
// comparison overloads

/** Compare two skiplists for equality
Does not compare structure.
KEY and VAL must overload== or specialize std::equal_to
@param[in] left  The Skiplist to compare with right
@param[in] right The Skiplist to compare with left
@param true if the skiplists have the same contents
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator==(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right)
{
    if (left.size() != right.size())
        return false;
    // The C++11 compliant std::equal triggers a C4996 warning (needs -D_SCL_SECURE_NO_WARNINGS) in MSVC
    // because it can overrun the 2nd container. To keep compatibility with C++11 for GCC, can't use std::equal.

    // instantiate comparator
    std::equal_to<typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_value_type> compare;

    // iterate
    using const_iterator = typename Skiplist<KEY, VAL, MULTIMAP, COMPARE>::const_iterator;
    const_iterator leftIter      = left.cbegin();
    const_iterator const leftEnd = left.cend();
    const_iterator rightIter     = right.cbegin();
    while (leftIter != leftEnd)
        if (!compare(*leftIter++, *rightIter++))
            return false;
    return true;
}


/** Compare two skiplists for inequality
Does not compare structure.
KEY and VAL must overload== or specialize std::equal_to
@param[in] left  The Skiplist to compare with right
@param[in] right The Skiplist to compare with left
@param true if the skiplists have different contents.
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator!=(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right)
{
    return !(left == right);
}


/** Lexographically compare two skiplists.
Does not compare structure.
KEY and VAL must overload operator< or specialize std::less
@param[in] left  The Skiplist to compare with right
@param[in] right The Skiplist to compare with left
@param true if left is lexographically less than right
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator<(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right)
{
    return std::lexicographical_compare(left.cbegin(), left.cend(), right.cbegin(), right.cend());
}


/** Lexographically compare two skiplists.
Does not compare structure.
KEY and VAL must overload operator< or specialize std::less
@param[in] left  The Skiplist to compare with right
@param[in] right The Skiplist to compare with left
@param true if left is lexographically greater than right
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator>(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right)
{
    return (right < left);
}


/** Lexographically compare two skiplists.
Does not compare structure.
KEY and VAL must overload operator< or specialize std::less
@param[in] left  The Skiplist to compare with right
@param[in] right The Skiplist to compare with left
@param true if left is lexographically less than or equal to right
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator<=(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right)
{
    return !(right < left);
}


/** Lexographically compare two skiplists.
Does not compare structure.
KEY and VAL must overload operator< or specialize std::less
@param[in] left  The Skiplist to compare with right
@param[in] right The Skiplist to compare with left
@param true if left is lexographically greater than or equal to right
*/
template <typename KEY, typename VAL, bool MULTIMAP, typename COMPARE>
bool operator>=(const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& left, const Skiplist<KEY, VAL, MULTIMAP, COMPARE>& right)
{
    return !(left < right);
}


// ==================================================================
// Skiplist Iterator Function Definitions

// ------------------------------------------------------------------
// Non-balancing Iterator

/** conversion constructor
Converts a SkiplistBalancingIterator into a SkiplistNonBalancingIterator
@param[in] iter A SkiplistBalancingIterator
*/
template <typename CONTAINER>
SkiplistNonBalancingIterator<CONTAINER>::SkiplistNonBalancingIterator(const SkiplistBalancingIterator<CONTAINER>& iter)
    : m_pContainer(iter.m_pContainer)
    , m_currentNode(iter.m_currentNode)
{ }


/** argument constructor
@param[in] container A reference to the skiplist.
@param[in] startNode The skiplist node for the iterator to start with.
*/
template <typename CONTAINER>
SkiplistNonBalancingIterator<CONTAINER>::SkiplistNonBalancingIterator(const CONTAINER& container, const Node* const startNode)
    : m_pContainer(&container)
    , m_currentNode(startNode)
{ 
    assert(!startNode || (startNode->Prev && !startNode->Down));  // may be nullptr, but must not be the dummy node and must be in the lowest list
}


/** friend swap. Not member function.
@param[in,out] left  The iterator to swap with right.
@param[in,out] right The iterator to swap with left.
*/
template <typename CONTAINER>
void swap(SkiplistNonBalancingIterator<CONTAINER>& left, SkiplistNonBalancingIterator<CONTAINER>& right) noexcept
{
    using std::swap;
    swap(left.m_pContainer,  right.m_pContainer);
    swap(left.m_currentNode, right.m_currentNode);
}


/** dereference overload
@return A reference to the current key/val pair.
*/
template <typename CONTAINER>
typename SkiplistNonBalancingIterator<CONTAINER>::reference SkiplistNonBalancingIterator<CONTAINER>::operator*() const
{
    assert(m_currentNode && m_currentNode->Prev);  // not the end iterator and not the dummy node
    return *m_currentNode->KeyValPair;
}


/** arrow overload
@return A pointer to the current key/val pair.
*/
template <typename CONTAINER>
typename SkiplistNonBalancingIterator<CONTAINER>::pointer SkiplistNonBalancingIterator<CONTAINER>::operator->() const
{
    assert(m_currentNode && m_currentNode->Prev);  // not the end iterator and not the dummy node
    return m_currentNode->KeyValPair;
}


/** equality overload
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to the skiplist element
*/
template <typename CONTAINER>
bool SkiplistNonBalancingIterator<CONTAINER>::operator==(const SkiplistNonBalancingIterator& right) const
{
    if (m_pContainer != right.m_pContainer)
    {
        assert(false);
        return false;
    }
    return m_currentNode == right.m_currentNode;
}


/** equality overload
special overload that takes a balancing iterator
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to the skiplist element
*/
template <typename CONTAINER>
bool SkiplistNonBalancingIterator<CONTAINER>::operator==(const SkiplistBalancingIterator<CONTAINER>& right) const
{
    if (m_pContainer != right.m_pContainer)
    {
        assert(false);
        return false;
    }
    return m_currentNode == right.m_currentNode;
}


/** inequality overload
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to different skiplist elements
*/
template <typename CONTAINER>
bool SkiplistNonBalancingIterator<CONTAINER>::operator!=(const SkiplistNonBalancingIterator& right) const
{
    return !(*this == right);
}


/** inequality overload
special overload that takes a balancing iterator
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to different skiplist elements
*/
template <typename CONTAINER>
bool SkiplistNonBalancingIterator<CONTAINER>::operator!=(const SkiplistBalancingIterator<CONTAINER>& right) const
{
    return !(*this == right);
}


/** prefix increment
Points the iterator to the next element in the sequence.
@return this (after increment)
*/
template <typename CONTAINER>
SkiplistNonBalancingIterator<CONTAINER>& SkiplistNonBalancingIterator<CONTAINER>::operator++()
{
    assert(m_currentNode && m_currentNode->Prev);  // not the end iterator and not the dummy node
    m_currentNode = m_currentNode->Next;
    return *this;
}


/** postfix increment
Points the iterator to the next element in the sequence.
@return A copy of this before increment
*/
template <typename CONTAINER>
SkiplistNonBalancingIterator<CONTAINER> SkiplistNonBalancingIterator<CONTAINER>::operator++(int)
{
    SkiplistNonBalancingIterator save(*this);
    ++(*this);
    return save;
}


/** prefix decrement
Points the iterator to the previous element in the sequence.
It is valid to decremend the end iterator.
Undefined behavior if --container.begin() is evaluated.
@return this (after decrement)
*/
template <typename CONTAINER>
SkiplistNonBalancingIterator<CONTAINER>& SkiplistNonBalancingIterator<CONTAINER>::operator--()
{
    // it is valid to decremend the end iterator.
    if (!m_currentNode)
    {
        // this is end pointer. Get tail.
        assert(m_pContainer);
        m_currentNode = m_pContainer->m_tail;
    }
    else
    {
        // Not the end iterator.
        // undefined behavior if --container.begin() is evaluated
        if (m_currentNode->Prev && m_currentNode->Prev->Prev)
            m_currentNode = m_currentNode->Prev;
        else
            assert(false);  // may not be the dummy node or the first actual node
    }
    return *this;
}


/** postfix decrement
Points the iterator to the previous element in the sequence.
It is valid to decremend the end iterator.
Undefined behavior if --container.begin() is evaluated.
@return A copy of this before decrement
*/
template <typename CONTAINER>
SkiplistNonBalancingIterator<CONTAINER> SkiplistNonBalancingIterator<CONTAINER>::operator--(int)
{
    SkiplistNonBalancingIterator save(*this);
    --(*this);
    return save;
}

// ------------------------------------------------------------------
// Balancing Iterator

/** argument constructor
@param[in] container      A reference to the skiplist.
@param[in] startNode      The skiplist node for the iterator to start with.
@param[in] startNodeIndex The index (in the skiplist) of the given startNode.
*/
template <typename CONTAINER>
SkiplistBalancingIterator<CONTAINER>::SkiplistBalancingIterator(CONTAINER& container, Node* const startNode, const size_type startNodeIndex)
    : m_pContainer(&container)
    , m_currentNode(startNode)
    , m_index(startNodeIndex)
    , m_dontBalance(container.is_balanced())
{ 
    if (startNode == nullptr)
        m_startLocation = StartLocation::END;
    else if (!startNode->Prev || startNode->Down)
    {
        // If no previous, the start node is a dummy node. This iterator should never be initialized with that dummy!
        // If there is a down, we are not in the lowest list. We must be in the lowest list!
        assert(false);
        m_startLocation = StartLocation::UNKNOWN;
    }
    else if (!startNode->Prev->Prev)
        m_startLocation = StartLocation::BEGINNING;
    else
        m_startLocation = StartLocation::UNKNOWN;
}


/** argument constructor, non-balancing
Use this constructor if you don't know the index of the node in the skiplist.
The iterator cannot balance if it doesn't know what index it is.

If you use this constructor, the iterator will not balance unless it so happens
that the starting node is the first or last element.

@param[in] container A reference to the skiplist.
@param[in] startNode The skiplist node for the iterator to start with.
*/
template <typename CONTAINER>
SkiplistBalancingIterator<CONTAINER>::SkiplistBalancingIterator(CONTAINER& container, Node* const startNode)
    : m_pContainer(&container)
    , m_currentNode(startNode)
    , m_dontBalance(true)
{ 
    if (startNode == nullptr)
    {
        m_startLocation = StartLocation::END;
        m_index = container.size();
        m_dontBalance = false;
    }
    else if (!startNode->Prev || startNode->Down)
    {
        // If no previous, the start node is a dummy node. This iterator should never be initialized with that dummy!
        // If there is a down, we are not in the lowest list. We must be in the lowest list!
        assert(false);  
        m_startLocation = StartLocation::UNKNOWN;
    }
    else if (!startNode->Prev->Prev)
    {
        m_startLocation = StartLocation::BEGINNING;
        m_index = 0;
        m_dontBalance = false;
    }
    else
        m_startLocation = StartLocation::UNKNOWN;
}


/** friend swap. Not member function.
calls base class swap.
@param[in,out] left  The iterator to swap with right.
@param[in,out] right The iterator to swap with left.
*/
template <typename CONTAINER>
void swap(SkiplistBalancingIterator<CONTAINER>& left, SkiplistBalancingIterator<CONTAINER>& right) noexcept
{
    using std::swap;
    swap(left.m_pContainer,    right.m_pContainer);
    swap(left.m_currentNode,   right.m_currentNode);
    swap(left.m_index,         right.m_index);
    swap(left.m_dontBalance,   right.m_dontBalance);
    swap(left.m_startLocation, right.m_startLocation);
}


/** dereference overload
@return A reference to the current key/val pair.
*/
template <typename CONTAINER>
typename SkiplistBalancingIterator<CONTAINER>::reference SkiplistBalancingIterator<CONTAINER>::operator*() const
{
    assert(m_currentNode && m_currentNode->Prev);  // not the end iterator and not the dummy node
    return *m_currentNode->KeyValPair;
}


/** arrow overload
@return A pointer to the current key/val pair.
*/
template <typename CONTAINER>
typename SkiplistBalancingIterator<CONTAINER>::pointer SkiplistBalancingIterator<CONTAINER>::operator->() const
{
    assert(m_currentNode && m_currentNode->Prev);  // not the end iterator and not the dummy node
    return m_currentNode->KeyValPair;
}


/** equality overload
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to the skiplist element
*/
template <typename CONTAINER>
bool SkiplistBalancingIterator<CONTAINER>::operator==(const SkiplistBalancingIterator& right) const
{
    if (m_pContainer != right.m_pContainer)
    {
        assert(false);
        return false;
    }
    return m_currentNode == right.m_currentNode;
}


/** equality overload
special overload that takes a non-balancing iterator
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to the skiplist element
*/
template <typename CONTAINER>
bool SkiplistBalancingIterator<CONTAINER>::operator==(const SkiplistNonBalancingIterator<CONTAINER>& right) const
{
    if (m_pContainer != right.m_pContainer)
    {
        assert(false);
        return false;
    }
    return m_currentNode == right.m_currentNode;
}


/** inequality overload
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to different skiplist elements
*/
template <typename CONTAINER>
bool SkiplistBalancingIterator<CONTAINER>::operator!=(const SkiplistBalancingIterator& right) const
{
    return !(*this == right);
}


/** inequality overload
special overload that takes a non-balancing iterator
Both iterators must from the same container.
@param[in] right The iterator to compare to.
@return true, if both iterators point to different skiplist elements
*/
template <typename CONTAINER>
bool SkiplistBalancingIterator<CONTAINER>::operator!=(const SkiplistNonBalancingIterator<CONTAINER>& right) const
{
    return !(*this == right);
}


/** Balances the current node's column. 
The current value will be placed in the appropriate levels based
on its index in the skiplist.
*/
template <typename CONTAINER>
void SkiplistBalancingIterator<CONTAINER>::balance()
{
    if (!m_currentNode)
        return;

    // calculate how many levels this node should be in based on its index
    const int desiredHeight = m_pContainer->calcBalancedLevel(m_index + 1);

    // count how many levels the node is in
    Node* current = m_currentNode;
    int height = 1;
    while (current->Up)
    {
        assert(height <= desiredHeight);
        if (height == desiredHeight)
        {
            // Node is in too many levels. Remove upper levels.
            m_pContainer->eraseAbove(current);
            current->Up = nullptr;
            break;
        }

        current = current->Up;
        ++height;
    }
    assert(height <= desiredHeight);
    if (height < desiredHeight)
    {
        // Node needs to be added to upper levels.
        m_pContainer->insertAbove(current, desiredHeight - height, current);
    }
}


/** prefix increment
Points the iterator to the next element in the sequence.
@return this (after increment)
*/
template <typename CONTAINER>
SkiplistBalancingIterator<CONTAINER>& SkiplistBalancingIterator<CONTAINER>::operator++()
{
    assert(m_currentNode && m_currentNode->Prev);  // not the end iterator and not the dummy node
    if (!m_dontBalance)
        balance();
    m_currentNode = m_currentNode->Next;
    ++m_index;
    // have we finished iterating over the entire collection?
    if (!m_currentNode && !m_dontBalance && m_startLocation == StartLocation::BEGINNING)
    {
        m_dontBalance = true;
        m_pContainer->m_balanced = true;
    }
    return *this;
}


/** postfix increment
Points the iterator to the next element in the sequence.
@return A copy of this before increment
*/
template <typename CONTAINER>
SkiplistBalancingIterator<CONTAINER> SkiplistBalancingIterator<CONTAINER>::operator++(int)
{
    SkiplistBalancingIterator save(*this);
    ++(*this);
    return save;
}


/** prefix decrement
Points the iterator to the previous element in the sequence.
It is valid to decremend the end iterator.
Undefined behavior if --container.begin() is evaluated.
@return this (after decrement)
*/
template <typename CONTAINER>
SkiplistBalancingIterator<CONTAINER>& SkiplistBalancingIterator<CONTAINER>::operator--()
{
    // it is valid to decremend the end iterator.
    if (!m_currentNode)
    {
        // this is end pointer. Get tail.
        // must not decrement in the case of an empty container. 
        // Undefined behavior if --container.begin() is evaluated
        // The tail must not point to the dummy node.
        assert(m_pContainer && m_pContainer->m_tail && m_pContainer->m_tail->Prev);
        m_currentNode = m_pContainer->m_tail;
        --m_index;
    }
    else
    {
        // Not the end iterator.
        // undefined behavior if --container.begin() is evaluated
        if (m_currentNode->Prev && m_currentNode->Prev->Prev)
        {
            m_currentNode = m_currentNode->Prev;
            --m_index;
        }
        else
            assert(false);  // may not be the dummy node or the first actual node
    }

    if (!m_dontBalance)
    {
        balance();
        // have we finished iterating over the entire collection?
        if (!m_currentNode->Prev->Prev && m_startLocation == StartLocation::END)
        {
            m_dontBalance = true;
            m_pContainer->m_balanced = true;
        }
    }
    return *this;
}


/** postfix decrement
Points the iterator to the previous element in the sequence.
It is valid to decremend the end iterator.
Undefined behavior if --container.begin() is evaluated.
@return A copy of this before decrement
*/
template <typename CONTAINER>
SkiplistBalancingIterator<CONTAINER> SkiplistBalancingIterator<CONTAINER>::operator--(int)
{
    SkiplistBalancingIterator save(*this);
    --(*this);
    return save;
}


}
