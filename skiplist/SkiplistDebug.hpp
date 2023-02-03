// ===========================================================================
// Copyright (c) 2014-2018 Alexander Freed. ALL RIGHTS RESERVED.
//
// ISO C++11
//
// This file contains debug functions for the Skiplist class. These functions
// are not part of the normal uses for the skiplist. They must be aware of 
// the internal structure, so SkiplistDebug is a friend class of Skiplist.
//
// ===========================================================================
#pragma once

#include "Skiplist.hpp"

#include <unordered_set>
#include <queue>


namespace fsl {


// ==================================================================

/** SkiplistDebug is a friend class of Skiplist.
It contains functions that are used only for debugging.
*/
class SkiplistDebug
{
public:
    template <typename CONTAINER>
    static std::size_t CountNodes(const CONTAINER& container);

    template <typename CONTAINER, typename OSTREAM> 
    static void DisplayHorizontally(const CONTAINER& container, OSTREAM&& os);

    template <typename CONTAINER, typename OSTREAM>
    static void DisplayVertically(const CONTAINER& container, OSTREAM&& os, const bool showKeys=false);
    
    template <typename CONTAINER>
    static bool Validate(const CONTAINER& container);

    template <typename Node, typename KEY, typename COMPARE>
    static int findDepth(const Node* const head, const KEY& key, COMPARE&& keyCompare, const int depth);
};


// ==================================================================
// SkiplistDebug Function Implementations

/** Counts the number of nodes in the skiplist.
Actually counts the nodes.
@param[in] container the skiplist
@return the number of nodes in the skiplist.
*/
template <typename CONTAINER>
std::size_t SkiplistDebug::CountNodes(const CONTAINER& container)
{
    using Node = typename CONTAINER::Node;

    int count = 0;
    const Node* levelHead = container.m_head;
    const Node* current;

    while (levelHead)
    {
        current = levelHead;
        while (current)
        {
            ++count;
            current = current->Next;
        }
        levelHead = levelHead->Down;
    }
    return count;
}


/** Display the structure of the skiplist for debugging
Displays the tree horizontally, with the fullests list on the bottom.
Not great for viewing when the list is very long.
OSTREAM as template so you can specify the stream you want.
@param[in] container The skiplist.
@param[in] os        The stream to print to.
*/
template <typename CONTAINER, typename OSTREAM>
void SkiplistDebug::DisplayHorizontally(const CONTAINER& container, OSTREAM&& os)
{
    using Node = typename CONTAINER::Node;

    if (!container.m_head)
        return;
    const Node* levelHead = container.m_head;
    const Node* current;
    do
    {
        current = levelHead->Next;
        os << "HEAD -> ";
        while (current)
        {
            os << current->KeyValPair->first;
            os << " -> ";
            current = current->Next;
        }
        os << "NULL\n";
        levelHead = levelHead->Down;
    } while (levelHead);
    os << "\n";
}


/** Display the structure of the skiplist for debugging
Displays the tree vertically, with the fullests list to the left.
Better for viewing the structure, as the width of the screen can handle 
a list with many elements, and it's easy to scroll up and down.
OSTREAM as template so you can specify the stream you want.
@param[in] container The skiplist.
@param[in] os        The stream to print to.
@param[in] showKey   (default false) true if you want to display the key along with the value
*/
template <typename CONTAINER, typename OSTREAM>
void SkiplistDebug::DisplayVertically(const CONTAINER& container, OSTREAM&& os, const bool showKeys)
{
    using Node = typename CONTAINER::Node;

    if (!container.m_head)
        return;
    // descend to bottom level
    const Node* current = container.m_head;
    while (current->Down)
        current = current->Down;

    int depth;
    while (current->Next)
    {
        current = current->Next;
        depth = findDepth(container.m_head, current->KeyValPair->first, container.m_keyCompare, 0);
        // display keys and values in a row
        for (int i = container.m_levelCount - 1; i >= 0; --i)
        {
            if (i >= depth)
            {
                if (showKeys)
                    os << current->KeyValPair->first << ": ";
                os << current->KeyValPair->second << "\t";
            }
        }
        os << "\n\n";
    }
}


/** Find the depth of the node with the given key
@param[in] head       The current node
@param[in] key        The key to search for
@param[in] keyCompare The skiplist's comparison function
@param[in] depth      The current depth
@return The depth of the node with the given key. -1 if the key was not found.
*/
template <typename Node, typename KEY, typename COMPARE>
int SkiplistDebug::findDepth(const Node* const head, const KEY& key, COMPARE&& keyCompare, const int depth)
{
    if (!head)
        return -1;
    const Node* const next = head->Next;
    if (!next || keyCompare(key, next->KeyValPair->first))  // went too far on this level
        return findDepth(head->Down, key, keyCompare, depth + 1);  // go down
    // It is assumed that two keys are equivalent if (!compare(A, B) && !compare(B, A)) is true.
    if (!keyCompare(next->KeyValPair->first, key))  // match found
        return depth;
    // traverse next
    return findDepth(head->Next, key, keyCompare, depth);
}


/** Debug function traverses the skiplist graph and validates that every
node is connected correctly and in the right spot.
@param[in] container the skiplist
@return true if the skiplist graph is in a valid state
*/
template <typename CONTAINER>
bool SkiplistDebug::Validate(const CONTAINER& container)
{
// Switch to 0 to skip this function. Useful for speeding up debugging if you already know where the problem is.
#if 1
    using Node = typename CONTAINER::Node;
    Node const* const& head = container.m_head;

    auto check = [](bool condition)->bool {
        assert(!condition);
        return condition;
    };
// Macro. If the assertion is true, exits this function and returns false
#define RETURN_IF_TRUE(x) if (check(x)) return false;
    auto countHeight = [](const Node* node)->int {
        int res = 0;
        while (node)
        {
            node = node->Up;
            ++res;
        }
        return res;
    };

    if (!head)
    {
        RETURN_IF_TRUE(container.m_tail);
        RETURN_IF_TRUE(container.m_begin);
        RETURN_IF_TRUE(container.m_count != 0);
        return true;
    }

    RETURN_IF_TRUE(!container.m_begin || !container.m_begin->Prev || container.m_begin->Prev->Prev);
    RETURN_IF_TRUE(!container.m_tail || container.m_tail->Next);
    RETURN_IF_TRUE(container.m_count == 0);

    // structures for keeping track of nodes-to-visit and already-visited-nodes
    std::unordered_set<const Node*> visited(container.m_count);
    std::queue<const Node*> toVisit;

    // before starting traversal, scan the left column. It should be the height of the
    // skiplist and only have dummy nodes in it.
    unsigned count = 0;
    const Node* current = head;
    const Node* prev = nullptr;

    while (current)
    {
        RETURN_IF_TRUE(current->Prev);
        RETURN_IF_TRUE(current->KeyValPair);
        if (!current->Up)
        {
            RETURN_IF_TRUE(current != head);
        }
        else
        {
            RETURN_IF_TRUE(current->Up->Down != current);
        }
        RETURN_IF_TRUE(current->Down && current->Down->Up != current);
        RETURN_IF_TRUE(current->Next && current->Next->Prev != current);
        // mark node as visited
        RETURN_IF_TRUE(!visited.insert(current).second);

        prev = current;
        current = current->Down;
        ++count;
    }
    RETURN_IF_TRUE(count != static_cast<unsigned>(container.m_levelCount));

    // next, scan the lowest list.

    // we already scanned the dummy node, so start with the first real node
    current = prev->Next;
    RETURN_IF_TRUE(current != container.m_begin);
    count = 0;
    typename CONTAINER::key_compare   keyCompare = container.key_comp();
    typename CONTAINER::value_compare valCompare = container.value_comp();

    while (current)
    {
        RETURN_IF_TRUE(!current->Prev);
        RETURN_IF_TRUE(!current->Prev->Next)
        RETURN_IF_TRUE(!current->KeyValPair);
        RETURN_IF_TRUE(current->Prev->KeyValPair && keyCompare(current->KeyValPair->first, current->Prev->KeyValPair->first));
        if (!container.s_MULTIMAP)
            RETURN_IF_TRUE(current->Prev->KeyValPair && !keyCompare(current->Prev->KeyValPair->first, current->KeyValPair->first));
        if (current->Up)
        {
            RETURN_IF_TRUE(container.m_levelCount == 1);
            RETURN_IF_TRUE(current->Up->Down != current);
            // add to the to-visit queue (used later, during traversal)
            toVisit.push(current->Up);
        }
        RETURN_IF_TRUE(current->Down);
        RETURN_IF_TRUE(current->Next && current->Next->Prev != current);

        // check height
        RETURN_IF_TRUE(container.is_balanced() && countHeight(current) != container.calcBalancedLevel(count + 1));


        // mark node as visited
        RETURN_IF_TRUE(!visited.insert(current).second);

        prev = current;
        current = current->Next;
        ++count;
    }
    RETURN_IF_TRUE(count != container.m_count);
    RETURN_IF_TRUE(prev != container.m_tail);

    // Now proceed with the breadth-first traversal

    // helpful lambda
    auto queueUnvisited = [&visited, &toVisit](const Node* const node) {
        if (node && visited.find(node) == visited.end())
            toVisit.push(node);
    };

    while (!toVisit.empty())
    {
        current = toVisit.front();
        toVisit.pop();

        // mark node as visited or skip if the node was already visited.
        if (!visited.insert(current).second)
            continue;
        
        // validate
        RETURN_IF_TRUE(!current->KeyValPair);
        RETURN_IF_TRUE(!current->Prev);
        RETURN_IF_TRUE(current->Prev == current);
        RETURN_IF_TRUE(current->Prev->Next != current);
        RETURN_IF_TRUE(current->Prev->KeyValPair && valCompare(*current->KeyValPair, *current->Prev->KeyValPair));
        if (!container.s_MULTIMAP)
            RETURN_IF_TRUE(current->Prev->KeyValPair && !valCompare(*current->Prev->KeyValPair, *current->KeyValPair));
        RETURN_IF_TRUE(!current->Down);
        RETURN_IF_TRUE(current->Down == current);
        RETURN_IF_TRUE(current->KeyValPair != current->Down->KeyValPair);
        RETURN_IF_TRUE(current->Down->Up != current);
        if (current->Next)
        {
            RETURN_IF_TRUE(current->Next->Prev != current);
            RETURN_IF_TRUE(current->Next == current);
        }
        if (current->Up)
        {
            RETURN_IF_TRUE(current->Up->Down != current);
            RETURN_IF_TRUE(current->Up == current);
        }

        // add connected neighbors to queue
        queueUnvisited(current->Up);
        queueUnvisited(current->Prev);
        queueUnvisited(current->Next);
        queueUnvisited(current->Down);
    }

    // run through the dummy node column and make sure the first real 
    // node (if it exists) has been marked as visited.
    current = head;
    while (current)
    {
        if (current->Next)
            RETURN_IF_TRUE(visited.find(current->Next) == visited.end());
        current = current->Down;
    }

    return true;

#undef RETURN_IF_TRUE

#else
    (void)container;
    return true;
#endif
}


}
