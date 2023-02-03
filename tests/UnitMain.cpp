// ===========================================================================
// Copyright (c) 2018 Alexander Freed. ALL RIGHTS RESERVED.
//
// ISO C++11
//
// Contains the unit test and memory test code for Skiplist.
// ===========================================================================

#include <Skiplist.hpp>
#include <SkiplistDebug.hpp>
#include "Movable.h"

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cassert>
#include <random>
#include <map>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <iterator>
#include <thread>

using namespace fsl;


// ------------------------------------------------------------------
// Macros

// A concise to call check_assertion with file and line info
#define CHECK(x) check_assertion(x, __FILE__, __LINE__)


// ------------------------------------------------------------------
// Enums

/** Used to indicate which tests will be performed
*/
enum class ETestSelection
{
    FAST,
    FULL,
    FULL_MULTITHREADED
};


// ------------------------------------------------------------------
// Utilities

/** On failure, throw an exception.
Unlike regular assert, this works in release mode
@param[in] success  if false, an exception is thrown
@param[in] filename __FILE__ (the name of the file )
@param[in] line     __LINE__ (the line number)
*/
inline void check_assertion(const bool success, const char* filename, const int line)
{
    if (!success)
    {
        std::cerr << "Error on " << filename << " at line " << line << std::endl;
        throw;
    }
}


/** This overload takes a map's insert_return type and returns it.
general case
@param[in] retval the return value from an insert operation into a map skiplist
@return the same thing you pass in
*/
template <typename iterator>
std::pair<iterator, bool>& extractInsertRetval(std::pair<iterator, bool>& retval)
{
    return retval;
}
/** This overload takes a multimap's insert_return type and converts it into a pair.
special case
Multimap inserts never fail, so the bool is always true.
@param[in] retval the return value from an insert operation into a multimap skiplist
@return a pair made from the return value and the boolean value true
*/
template <typename iterator>
std::pair<iterator, bool> extractInsertRetval(iterator& retval)
{
    return std::pair<iterator, bool>(retval, true);
}


/** automatically checks the result of an insert operation
@param[in] container      the container to insert into
@param[in] key            the key for the new value
@param[in] val            the value to insert at the given key
@param[in] expectedResult (default=true) true if the insert should success, false if it should fail
@return the return value of the insert function
*/
template <typename CONTAINER, typename KEY, typename VAL>
std::pair<typename CONTAINER::iterator, bool> insertWithCheck(CONTAINER& container, KEY&& key, VAL&& val, const bool expectedResult=true)
{
    // We want to compare insides of the resulting iterator to make sure it is pointing to the right things,
    // but if the key or val was moved, we need to make a copy of it (for the comparison).
    // If it's not a move, just use a reference.
    using KeyCopyOrRef = typename std::conditional<std::is_move_constructible<KEY>::value,
        const KEY, const KEY&>::type;
    using ValCopyOrRef = typename std::conditional<std::is_move_constructible<VAL>::value,
        const VAL, const VAL&>::type;

    KeyCopyOrRef keyControl(key);
    ValCopyOrRef valControl(val);

    auto insertReturn = container.emplace(std::forward<KEY>(key), std::forward<VAL>(val));
    auto resultPair = extractInsertRetval(insertReturn);
    // success?
    CHECK( resultPair.second == expectedResult );  
    // iterator pointing to right place?
    CHECK( resultPair.first->first  == keyControl );
    CHECK( resultPair.first->second == valControl );

    return resultPair;
}


/** automatically checks the result of an insert operation
@param[in] container      the container to insert into
@param[in] pair           the key/value pair to insert
@param[in] expectedResult (default=true) true if the insert should success, false if it should fail
@return the return value of the insert function
*/
template <typename CONTAINER, typename KVPAIR>
std::pair<typename CONTAINER::iterator, bool> insertWithCheck(CONTAINER& container, KVPAIR&& pair, const bool expectedResult=true)
{
    // We want to compare insides of the resulting iterator to make sure it is pointing to the right things,
    // but if the key or val was moved, we need to make a copy of it (for the comparison).
    // If it's not a move, just use a reference.
    using CopyOrRef = typename std::conditional<std::is_move_constructible<KVPAIR>::value,
        const KVPAIR, const KVPAIR&>::type;

    CopyOrRef pairControl(pair);

    auto insertReturn = container.emplace(std::forward<KVPAIR>(pair));
    auto resultPair = extractInsertRetval(insertReturn);
    // success?
    CHECK( resultPair.second == expectedResult );  
    // iterator pointing to right place?
    CHECK( *resultPair.first == pairControl );

    return resultPair;
}


// ==================================================================
// Skiplist Unit Tests

/** test insert, find, at, [], remove, contains, clear, popfront, count, and empty
*/
void UnitTest1()
{    
    using Skiplist_t = Skiplist<Movable, Movable>;
    using fsl::make_pair;

    // check equality on default-initialized lists
    Skiplist_t skiplist;
    CHECK( skiplist == Skiplist_t() );
    CHECK( SkiplistDebug::Validate(skiplist) );

    // check insert. Throw in some moves
    Movable temp1(2);
    Movable temp2(4);

    insertWithCheck(skiplist, 2, temp2);             // 2,  4
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( temp2 == 4 );

    insertWithCheck(skiplist, 1, std::move(temp1));  // 1,  2
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( temp1 == Movable() );

    insertWithCheck(skiplist, 3, 8);                 // 3,  8
    CHECK( SkiplistDebug::Validate(skiplist) );

    insertWithCheck(skiplist, 3, 8, false);
    CHECK( SkiplistDebug::Validate(skiplist) );

    insertWithCheck(skiplist, std::move(temp2), 16); // 4, 16
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( temp2 == Movable() );
    
    auto p = make_pair(Movable(0), Movable(1));
    insertWithCheck(skiplist, p);                    // 0,  1
    CHECK( SkiplistDebug::Validate(skiplist) );

    temp1 = Movable(5);
    temp2 = Movable(32);
    insertWithCheck(skiplist, make_pair(std::move(temp1), std::move(temp2)));  // 5, 32
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( temp1 == Movable() );
    CHECK( temp2 == Movable() );

    // test initialization list construction
    Skiplist_t braceInitialized = {
        { 2,  4 },
        { 1,  2 },
        { 3,  8 },
        { 4, 16 },
        { 0,  1 },
        { 5, 32 } };
    CHECK( SkiplistDebug::Validate(braceInitialized) );
    CHECK( skiplist == braceInitialized );
    
    // test initialization list assignment
    braceInitialized = {
        { 5, 32 },
        { 4, 16 },
        { 3,  8 },
        { 2,  4 },
        { 1,  2 },
        { 0,  1 } };
    CHECK( SkiplistDebug::Validate(braceInitialized) );
    CHECK(skiplist == braceInitialized);

    // test initialization list construction
    Skiplist_t braceInitialized2{
        { 2,  4 },
        { 1,  2 },
        { 3,  8 },
        { 4, 16 },
        { 0,  1 },
        { 5, 32 } };
    CHECK( SkiplistDebug::Validate(braceInitialized2) );
    CHECK( skiplist == braceInitialized2 );

    // test initialization list insertion
    Skiplist_t braceInserted;
    braceInserted.insert({
        { 2,  4 },
        { 1,  2 },
        { 3,  8 },
        { 4, 16 },
        { 0,  1 },
        { 5, 32 } });
    CHECK( SkiplistDebug::Validate(braceInserted) );
    CHECK( skiplist == braceInserted );

    // test find and at
    CHECK( skiplist.find(3)->second == 8 );
    CHECK( skiplist[3] == 8 );
    CHECK( skiplist.at(3) == 8 );
    CHECK( skiplist.at(1) == 2 );
    CHECK( skiplist[0] == 1 );
    CHECK( skiplist.find(3)->second == 8 );
    CHECK( skiplist.at(5) == 32 );
    CHECK( skiplist.find(5)->second == 32 );
    // test const find and const at
    const Skiplist_t& constSkiplist{ skiplist };
    CHECK( constSkiplist.find(3)->second == 8 );
    CHECK( constSkiplist.at(3) == 8 );
    CHECK(SkiplistDebug::Validate(skiplist));

    // test removal
    CHECK( skiplist.erase(17) == 0 );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.erase(3) == 1 );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.erase(0) == 1 );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.erase(0) == 0 );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.erase(5) == 1 );
    CHECK( SkiplistDebug::Validate(skiplist) );

    // test non-reference find
    Movable result;
    CHECK( !skiplist.find(0, result) );
    CHECK(  skiplist.find(4, result) );
    CHECK( result.GetVal() == 16 );

    // test find-set and at-set
    skiplist[1] = 17;
    CHECK( skiplist.at(1) == 17 );
    skiplist.at(1) = 7;
    CHECK( skiplist[1] == 7 );
    CHECK( skiplist.find(17) == skiplist.end() );
    bool goodThrow;
    try
    {
        skiplist.at(17);
        goodThrow = false;
    }
    catch (const std::out_of_range&)
    {
        goodThrow = true;
    }
    catch (...)
    {
        goodThrow = false;
    }
    CHECK( goodThrow );

    // test contains()
    CHECK( !skiplist.contains(50) );
    CHECK( skiplist.contains(4) );

    // test front() and pop_front()
    CHECK( SkiplistDebug::Validate(skiplist) );
    auto result2 = skiplist.front();
    auto result3 = constSkiplist.front();
    CHECK( result2 == result3 );
    skiplist.pop_front();
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( result2.second.GetVal() == 7 );
    CHECK( !skiplist.contains(1) );

    // test back() and pop_back();
    skiplist.emplace(6, 64);
    auto result4 = skiplist.back();
    auto result5 = constSkiplist.back();
    CHECK( result4 == result5 );
    skiplist.pop_back();
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( result4.second.GetVal() == 64 );
    CHECK( !skiplist.contains(6) );
    
    // test size(), clear(), empty(), and max_size()
    CHECK( skiplist.size() == 2 );
    skiplist.clear();
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.empty() );
    CHECK( skiplist.max_size() == (std::size_t(0) - 1) );

    // empty destructor
}


/** Test copy/move
*/
void UnitTest2()
{
    using Skiplist_t = Skiplist<Movable, Movable>;
    Skiplist_t skip1;
    skip1.emplace(0, 1);
    skip1.emplace(1, 2);
    skip1.emplace(2, 4);
    skip1.emplace(3, 8);    
    skip1.emplace(4, 16);
    skip1.emplace(5, 32);
    CHECK( SkiplistDebug::Validate(skip1) );

    Skiplist_t skip2(skip1);
    CHECK( SkiplistDebug::Validate(skip2) );
    CHECK( skip1 == skip2 );
    CHECK( !skip1.is_balanced() );
    CHECK( skip2.is_balanced() );
    
    Skiplist_t skip3(std::move(skip1));
    CHECK( SkiplistDebug::Validate(skip3) );
    CHECK( SkiplistDebug::Validate(skip1) );
    CHECK( skip3 == skip2 );
    CHECK( skip1 == Skiplist_t() );
    CHECK( skip1.is_balanced() );
    CHECK( !skip3.is_balanced() );

    Skiplist_t skip4(skip1);
    CHECK( SkiplistDebug::Validate(skip4) );
    CHECK( skip4 == Skiplist_t() );
    CHECK( skip4.is_balanced() );
    skip4 = skip2;
    CHECK( SkiplistDebug::Validate(skip4) );
    CHECK( skip4 != Skiplist_t() );
    CHECK( skip4 == skip2 );
    CHECK( skip4.is_balanced() );
    skip4 = std::move(skip2);
    CHECK( SkiplistDebug::Validate(skip4) );
    CHECK( SkiplistDebug::Validate(skip2) );
    CHECK( skip4.is_balanced() );
    CHECK( skip4 == skip3 );
    CHECK( skip2 == Skiplist_t() );
    CHECK( !(skip2 != Skiplist_t()) );

    using std::swap;
    swap(skip2, skip3);
    CHECK( SkiplistDebug::Validate(skip2) );
    CHECK( SkiplistDebug::Validate(skip3) );
    CHECK( skip3 == Skiplist_t() );
    CHECK( skip2 == skip4 );
    CHECK( skip3.is_balanced() );
    CHECK( !skip2.is_balanced() );

    skip2.swap(skip3);
    CHECK( SkiplistDebug::Validate(skip2) );
    CHECK( SkiplistDebug::Validate(skip3) );
    CHECK( skip2 == Skiplist_t() );
    CHECK( skip3 == skip4 );
    CHECK( skip2.is_balanced() );
    CHECK( !skip3.is_balanced() );
}


/** Test balancing
*/
void UnitTest3()
{
    Skiplist<std::string, int> ages;
    insertWithCheck(ages, "Alex",   29);
    insertWithCheck(ages, "Sarah",  24);
    insertWithCheck(ages, "Robbie", 26);
    insertWithCheck(ages, "River",   7);
    insertWithCheck(ages, "Eden",    5);
    insertWithCheck(ages, "David",  28);
    CHECK( SkiplistDebug::Validate(ages) );

    ages.balance();
    CHECK( SkiplistDebug::Validate(ages) );
    CHECK( SkiplistDebug::CountNodes(ages) == 13 );  // hand-calculated value

    // 1 million random values
    const int LEN = 1024 * 1024;
    Skiplist<int, int> seq;
    std::uniform_int_distribution<int> distribution(0, 2000000000);
    std::mt19937 random;
    for (int i = 0; i < LEN; ++i)
    {
        if (!seq.emplace(distribution(random), i).second)
            --i;
    }
    CHECK( SkiplistDebug::Validate(seq) );

    long long sum = 0;
    auto sumFunc = [&sum](Skiplist<int, int>::reference kvPair) { sum += kvPair.second; };
    auto sumFuncConst = [&sum](Skiplist<int, int>::const_reference kvPair) { sum += kvPair.second; };

    // non balancing iteration
    auto numNodes = SkiplistDebug::CountNodes(seq);
    seq.for_each_no_balance(sumFunc);
    CHECK( !seq.is_balanced() );
    CHECK( SkiplistDebug::Validate(seq) );
    CHECK( sum == (long long)LEN * (LEN - 1) / 2 );  // closed form of summation
    CHECK( numNodes == SkiplistDebug::CountNodes(seq) );  // operation shouldn't have changed the number of nodes

    auto constSkiplistTest = [&sumFuncConst](const Skiplist<int, int>& skiplist) { skiplist.for_each(sumFuncConst); };
    sum = 0;
    constSkiplistTest(seq);
    CHECK( !seq.is_balanced() );
    CHECK( SkiplistDebug::Validate(seq) );
    CHECK( sum == (long long)LEN * (LEN - 1) / 2 );  // closed form of summation
    CHECK( numNodes == SkiplistDebug::CountNodes(seq) );  // operation shouldn't have changed the number of nodes

    // auto balancing iteration
    sum = 0;
    seq.for_each(sumFunc);
    CHECK( SkiplistDebug::Validate(seq) );
    CHECK( sum == (long long)LEN * (LEN - 1) / 2 );  // closed form of summation
    CHECK( SkiplistDebug::CountNodes(seq) == seq.size() * 2 + 20 );  // hand-tuned + 20 because LEN is 1024*1024
    CHECK( seq.is_balanced() );

    // test with duplicate keys = true
    Skiplist<int, int, true> duple;
    // 1 million random values
    for (int i = 0; i < LEN; ++i)
    {
        insertWithCheck(duple, distribution(random), i);
    }
    CHECK( SkiplistDebug::Validate(duple) );

    // auto balancing iteration
    sum = 0;
    duple.for_each(sumFunc);
    CHECK( SkiplistDebug::Validate(duple) );
    CHECK( sum == (long long)LEN * (LEN - 1) / 2 );  // closed form of summation
    CHECK( SkiplistDebug::CountNodes(duple) == duple.size() * 2 + 20 );  // hand-tuned + 20 because LEN is 1024*1024
    CHECK( duple.is_balanced() );
}


/** stress test / memory leak test
If MULTIMAP is true, test skiplists that allow duplicate keys
*/
template <bool MULTIMAP>
void UnitTest4()
{
    using fsl::make_pair;

    // create random sample set of ~1 million values that will be reused
    // (It needs to be on the heap)
    const int LEN = 1024 * 1024;
    std::unique_ptr<int[]> inputSet(new int[LEN]);

    {
        std::uniform_int_distribution<int> distribution(0, 2000000000);
        std::mt19937 random;
        for (int i = 0; i < LEN; ++i)
            inputSet[i] = distribution(random);
    }

    // insert 1 million random values
    Skiplist<int, int, MULTIMAP> seq;
    for (int i = 0; i < LEN; ++i)
    {
        seq.emplace(inputSet[i], i);
    }
    CHECK( SkiplistDebug::Validate(seq) );
    
    Skiplist<int, int, MULTIMAP> clearTest;
    clearTest = seq;
    CHECK( SkiplistDebug::Validate(clearTest) );
    clearTest.clear();
    CHECK( SkiplistDebug::Validate(clearTest) );
    
    Skiplist<int, int, MULTIMAP> destructorTest(seq);
    (void)seq;
    
    // find all the values
    for (int i = 0; i < LEN; ++i)
    {
        seq[inputSet[i]];
    }
    
    // remove all the values
    for (int i = 0; i < LEN; ++i)
    {
        seq.erase(inputSet[i]);
    }
    CHECK( SkiplistDebug::Validate(seq) );
    CHECK( seq.size() == 0 );
    
    // balance test
    
    // insert 1 million random values
    Skiplist<int, int, MULTIMAP> balanceTest;
    CHECK( SkiplistDebug::Validate(balanceTest) );
    for (int i = 0; i < LEN; ++i)
    {
        balanceTest.insert(make_pair(inputSet[i], i));
    }
    CHECK( SkiplistDebug::Validate(balanceTest) );
    balanceTest.balance();
    CHECK( SkiplistDebug::Validate(balanceTest) );

    // pop test & range-insert test

    // insert 1 million random values
    Skiplist<int, int, MULTIMAP> popTest;
    SLPair<int, int> kvPair;
    for (int i = 0; i < LEN; ++i)
    {
        kvPair = { inputSet[i], i };
        popTest.insert(kvPair);
    }
    CHECK( SkiplistDebug::Validate(popTest) );

    // test range-insert
    Skiplist<int, int, MULTIMAP> rangeInsert;
    rangeInsert.insert(popTest.cbegin(), popTest.cend());
    CHECK( SkiplistDebug::Validate(rangeInsert) );
    CHECK( rangeInsert == popTest );

    // pop_front test
    std::size_t stop = popTest.size();
    for (std::size_t i = 0; i < stop; ++i)
    {
        popTest.pop_front();
        if (i == stop / 2)
            CHECK( SkiplistDebug::Validate(popTest) );
    }
    CHECK( SkiplistDebug::Validate(popTest) );
    CHECK( popTest.size() == 0 );

    // pop_back test
    stop = rangeInsert.size();
    for (std::size_t i = 0; i < stop; ++i)
    {
        rangeInsert.pop_back();
        if (i == stop / 2)
            CHECK( SkiplistDebug::Validate(rangeInsert) );
    }
    CHECK( SkiplistDebug::Validate(rangeInsert) );
    CHECK( rangeInsert.size() == 0 );
}


/** Helper function for UnitTest5
validates consistency after erasing
@param[in] skiplist   the container
@param[in] gen        An instance of mt19937_64
@param[in] leftBound  The key to the element to the left of the one that got removed. Must be a non-negative int.
                          If the removed element was the beginning, put -1
@param[in] removed    The key to the element that was removed. Must be a non-negative int.
@param[in] rightBound The key to the element to the right of the one that got removed. Must be a non-negative int.
                          If the removed element was the beginning, put -1
*/
template <typename CONTAINER>
void eraseCheckHelper (CONTAINER& skiplist, std::mt19937_64& gen, int leftBound, int removed, int rightBound)
{
    std::uniform_int_distribution<int> rng(0, 3);

    assert(leftBound != rightBound);
    assert(leftBound >= -1 && rightBound >= -1);

    auto leftIter = skiplist.find(leftBound);
    auto rightIter = skiplist.find(rightBound);

    // -1 is passed in to indicate before the beginning (for leftBound) or end (for rightBound)
    if (leftBound == -1)
    {
        CHECK( leftIter == skiplist.end() );
        CHECK( rightIter == skiplist.begin() );
    }
    else
    {
        CHECK( leftIter->second == leftBound );
        CHECK( std::prev(rightIter) == leftIter );
        CHECK( std::next(leftIter) == rightIter );
    }
    if (rightBound == -1)
        CHECK( rightIter == skiplist.end() );
    else
        CHECK( rightIter->second == rightBound );
        
    CHECK( skiplist.find(removed) == skiplist.end() );
        
    if (leftBound != -1)
    {
        switch (rng(gen))
        {
        case 0:
            leftIter++;
            break;
        case 1:
            rightIter--;
            break;
        case 2:
            std::advance(leftIter, 1);
            break;
        case 3:
            std::advance(rightIter, -1);
            break;
        default:
            assert(false && "eraseCheckHelper rng not within bounds.");
            break;
        }
        CHECK( leftIter == rightIter );
    }

    CHECK( SkiplistDebug::Validate(skiplist) );
}


/** more robust removal testing of map and multimap skiplists
*/
void UnitTest5()
{
    Skiplist<Movable, Movable> skiplist;
    std::mt19937_64 gen(std::chrono::steady_clock::now().time_since_epoch().count());

    for (int i = 0; i < 20; ++i)
        skiplist.emplace(i, i);
    skiplist.balance();
    CHECK( SkiplistDebug::Validate(skiplist) );

    // 20 items, 5 levels
    //
    // h                                    15
    // h               7                    15
    // h       3       7        11          15          19
    // h   1   3   5   7   9    11    13    15    17    19
    // h 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19

    

    auto iter = skiplist.find(10);
    iter = skiplist.erase(iter);
    eraseCheckHelper(skiplist, gen, 9, 10, 11);

    iter = skiplist.erase(iter);
    eraseCheckHelper(skiplist, gen, 9, 11, 12);
    
    iter = skiplist.find(15);
    iter = skiplist.erase(iter);
    eraseCheckHelper(skiplist, gen, 14, 15, 16);

    iter = skiplist.find(19);
    iter = skiplist.erase(iter);
    eraseCheckHelper(skiplist, gen, 18, 19, -1);

    iter = skiplist.find(0);
    iter = skiplist.erase(iter);
    eraseCheckHelper(skiplist, gen, -1, 0, 1);

    iter = skiplist.begin();
    iter = skiplist.erase(iter);
    eraseCheckHelper(skiplist, gen, -1, 1, 2);

    iter = skiplist.find(7);
    iter = skiplist.erase(iter);
    eraseCheckHelper(skiplist, gen, 6, 7, 8);


    // reset
    skiplist.erase(skiplist.begin(), skiplist.end());
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.empty() );
    for (int i = 0; i < 1000; ++i)
        CHECK( skiplist.emplace(i, i).second );
    CHECK( SkiplistDebug::Validate(skiplist) );

    // test range erase
    iter = skiplist.erase(skiplist.find(400), skiplist.find(600));
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.find(399)->second == 399 );
    CHECK( skiplist.lower_bound(400)->second == 600 );
    CHECK( iter->second == 600 );
    CHECK( skiplist.size() == 800 );

    iter = skiplist.erase(skiplist.begin(), skiplist.find(200));
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.begin()->second == 200 );
    CHECK( iter->second == 200 );
    CHECK( skiplist.size() == 600 );

    iter = skiplist.erase(skiplist.find(800), skiplist.end());
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( iter == skiplist.end() );
    --iter;
    CHECK( iter->second == 799 );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.size() == 400 );

    iter = skiplist.erase(skiplist.begin(), skiplist.end());
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( iter == skiplist.end() );
    CHECK( skiplist.begin() == skiplist.end() );
    CHECK( skiplist.empty() );



    // multimap
    Skiplist<Movable, Movable, true> multimap;

    // insert the same element 20 times
    for (int i = 0; i < 20; ++i)
        multimap.insert({ 5, 5 });

    // erase 1, by iterator
    auto mIter = multimap.find(5);
    mIter = multimap.erase(mIter);
    CHECK( mIter->second == 5 );
    CHECK( multimap.size() == 19 );

    // erase all 5's
    CHECK( multimap.erase(5) == 19 );
    CHECK( multimap.size() == 0 );
    CHECK( SkiplistDebug::Validate(multimap) );

    // insert the same element 20 times
    for (int i = 0; i < 20; ++i)
        multimap.insert(make_pair(Movable(15), Movable(15)));
    // insert rightmost element
    multimap.insert(make_pair(Movable(16), Movable(16)));

    CHECK( multimap.erase(15) == 20 );
    CHECK( multimap.size() == 1 );
    eraseCheckHelper(multimap, gen, -1, 15, 16);

    multimap.clear();
    // insert the same element 20 times
    for (int i = 0; i < 20; ++i)
        multimap.emplace(15, 15);
    // insert leftmost element
    multimap.insert({ 0, 0 });

    CHECK( multimap.erase(15) == 20 );
    CHECK( multimap.size() == 1 );
    eraseCheckHelper(multimap, gen, 0, 15, -1);

    multimap.clear();
    // insert the same key 10000 times
    auto p = make_pair(Movable(15), Movable(0));
    for (int i = 0; i < 10000; ++i)
    {
        p.second = i;
        multimap.insert(p);
    }
    // perform check that multimap elements are inserted in order 
    int i = 0;
    for (auto cIter = multimap.cbegin(); cIter != multimap.cend(); ++cIter, ++i)
        CHECK(cIter->second == i);
    // insert leftmost element
    multimap.insert({ 0, 0 });
    // insert rightmost element
    multimap.insert({ 16, 16 });
    CHECK( SkiplistDebug::Validate(multimap) );

    CHECK( multimap.erase(15) == 10000 );
    CHECK( multimap.size() == 2 );
    eraseCheckHelper(multimap, gen, 0, 15, 16);


    // reset
    multimap.erase(multimap.begin(), multimap.end());
    CHECK( SkiplistDebug::Validate(multimap) );
    CHECK( multimap.empty() );
    for (i = 0; i < 1000; ++i)
        for (int j = 0; j < 3; ++j)
            multimap.emplace(i, j);
    CHECK( SkiplistDebug::Validate(multimap) );
    CHECK( multimap.size() == 3000 );

    // test range erase
    mIter = multimap.erase(multimap.lower_bound(400), multimap.upper_bound(599));
    CHECK( SkiplistDebug::Validate(multimap) );
    auto range = multimap.equal_range(399);
    CHECK( range.first->first  == 399 );
    CHECK( range.second->first == 600 );
    CHECK( std::distance(range.first, range.second) == 3 );
    CHECK( mIter->first == 600 );
    CHECK( multimap.size() == 2400 );

    mIter = multimap.erase(multimap.begin(), multimap.lower_bound(200));
    CHECK( SkiplistDebug::Validate(multimap) );
    CHECK( multimap.begin()->first  == 200 );
    CHECK( multimap.begin()->second == 0   );
    CHECK( mIter->first == 200 );
    CHECK( multimap.size() == 1800 );

    mIter = multimap.erase(multimap.upper_bound(799), multimap.end());
    CHECK( SkiplistDebug::Validate(multimap) );
    CHECK( mIter == multimap.end() );
    --mIter;
    CHECK( mIter->first == 799 );
    CHECK( SkiplistDebug::Validate(multimap) );
    CHECK( multimap.size() == 1200 );

    mIter = multimap.erase(multimap.begin(), multimap.end());
    CHECK( SkiplistDebug::Validate(multimap) );
    CHECK( mIter == multimap.end() );
    CHECK( multimap.begin() == multimap.end() );
    CHECK( multimap.empty() );
}


/** Test count, lower_bound, upper_bound, and equal_range with map and multimap variants of skiplist
*/
void UnitTest6()
{
    // test non-multimap (const and non-const)
    Skiplist<Movable, Movable> skiplist;
    const Skiplist<Movable, Movable>& constSkiplist = skiplist;
    const int N = 20;

    for (int i = 0; i < N; ++i)
        skiplist.emplace(i, i);

    // 20 items, 5 levels
    // Here is what it would look like if it were balanced
    //
    // h                                    15
    // h               7                    15
    // h       3       7        11          15          19
    // h   1   3   5   7   9    11    13    15    17    19
    // h 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19

    // check count
    CHECK( skiplist.count(-1)    == 0 );
    CHECK( skiplist.count(0)     == 1 );
    CHECK( skiplist.count(10)    == 1 );
    CHECK( skiplist.count(N - 1) == 1 );
    CHECK( skiplist.count(N)     == 0 );

    auto rangePair = skiplist.equal_range(10);
    CHECK( std::distance(rangePair.first, rangePair.second) == 1 );
    auto constRangePair = constSkiplist.equal_range(10);
    CHECK( std::distance(constRangePair.first, constRangePair.second) == 1 );

    // check lower_bound, including edge cases
    CHECK(      skiplist.lower_bound(-1)->second == 0 );
    CHECK( constSkiplist.lower_bound(-1)->second == 0 );
    for (int i = 0; i < N; ++i)
    {
        CHECK(      skiplist.lower_bound(i)->second == i );
        CHECK( constSkiplist.lower_bound(i)->second == i );
    }
    CHECK(      skiplist.lower_bound(N) == skiplist.cend() );
    CHECK( constSkiplist.lower_bound(N) == skiplist.cend() );

    // check upper_bound, including edge cases
    for (int i = 0; i < N; ++i)
    {
        CHECK(      skiplist.upper_bound(i - 1)->second == i );
        CHECK( constSkiplist.upper_bound(i - 1)->second == i );
    }
    CHECK(      skiplist.upper_bound(N - 1) == skiplist.cend() );
    CHECK( constSkiplist.upper_bound(N - 1) == skiplist.cend() );
    CHECK(      skiplist.upper_bound(N)     == skiplist.cend() );
    CHECK( constSkiplist.upper_bound(N)     == skiplist.cend() );


    // test multimaps (non-const first)
    using Multimap_t = Skiplist<Movable, Movable, true>;
    Multimap_t multimap;
    auto mmIter = multimap.end();
    auto mmRangePair = multimap.equal_range(0);
    const Multimap_t& constMultimap = multimap;
    auto cmmIter = constMultimap.cend();
    auto cmmRangePair = constMultimap.equal_range(0);

    // insert each key [0...N) 3 times
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < 3; ++j)
            multimap.emplace(i, i);

    // 60 items, 6 levels
    // Here is what it would look like if it were balanced
    //
    // h                                                                10
    // h                               5                                10                                              15
    // h               2               5               7                10                      13                      15                      18
    // h       1       2       3       5       6       7       9        10          11          13          14          15          17          18          19
    // h   0   1   1   2   3   3   4   5   5   6   7   7   8   9   9    10    11    11    12    13    13    14    15    15    16    17    17    18    19    19
    // h 0 0 0 1 1 1 2 2 2 3 3 3 4 4 4 5 5 5 6 6 6 7 7 7 8 8 8 9 9 9 10 10 10 11 11 11 12 12 12 13 13 13 14 14 14 15 15 15 16 16 16 17 17 17 18 18 18 19 19 19

    // check count
    CHECK( multimap.count(-1) == 0 );
    CHECK( multimap.count(0)  == 3 );
    CHECK( multimap.count(10) == 3 );
    CHECK( multimap.count(19) == 3 );
    CHECK( multimap.count(20) == 0 );

    // check lower_bound, including edge cases
    mmIter = multimap.lower_bound(-1);
    CHECK( mmIter == multimap.cbegin() );
    mmIter = multimap.lower_bound(0);
    CHECK( mmIter == multimap.cbegin() );

    for (int i = 1; i < N - 1; ++i)
    {
        mmIter = multimap.lower_bound(i);
        CHECK( std::prev(mmIter)->second == i - 1 );
        CHECK( (*mmIter++).second == i );
        CHECK( (mmIter++)->second == i );
        CHECK( (*mmIter).second   == i );
        CHECK( (++mmIter)->second == i + 1);

        mmRangePair = multimap.equal_range(i);  // also check equal_range
        CHECK( std::distance(mmRangePair.first, mmRangePair.second) == 3 );
        CHECK( std::all_of(mmRangePair.first, mmRangePair.second, [&i](Multimap_t::const_reference kvPair) { return kvPair.first == i; }) );
    }
    mmIter = multimap.lower_bound(N - 1);
    CHECK( std::prev(mmIter)->second == N - 2 );
    CHECK( (*mmIter++).second == N - 1 );
    CHECK( (mmIter++)->second == N - 1 );
    CHECK( (*mmIter).second   == N - 1 );
    CHECK( (++mmIter) == multimap.cend() );
    mmIter = multimap.lower_bound(N);
    CHECK( mmIter == multimap.cend() );

    // check upper_bound, including edge cases
    mmIter = multimap.upper_bound(-1);
    CHECK( mmIter == multimap.cbegin() );
    for (int i = 0; i < N - 2; ++i)
    {
        mmIter = multimap.upper_bound(i);
        CHECK( std::prev(mmIter)->second == i );
        CHECK( (*mmIter++).second == i + 1 );
        CHECK( (mmIter++)->second == i + 1 );
        CHECK( (*mmIter).second   == i + 1 );
        CHECK( (++mmIter)->second == i + 2 );
    }
    mmIter = multimap.upper_bound(N - 2);
    CHECK( std::prev(mmIter)->second == N - 2 );
    CHECK( (*mmIter++).second == N - 1 );
    CHECK( (mmIter++)->second == N - 1 );
    CHECK( (*mmIter).second   == N - 1 );
    CHECK( ++mmIter == multimap.cend() );
    mmIter = multimap.upper_bound(N - 1);
    CHECK( mmIter == multimap.cend() );
    mmIter = multimap.upper_bound(N);
    CHECK( mmIter == multimap.cend() );

    // const multimap
    // check count
    CHECK( constMultimap.count(-1) == 0 );
    CHECK( constMultimap.count(0)  == 3 );
    CHECK( constMultimap.count(10) == 3 );
    CHECK( constMultimap.count(19) == 3 );
    CHECK( constMultimap.count(20) == 0 );

    // check lower_bound, including edge cases
    cmmIter = constMultimap.lower_bound(-1);
    CHECK( cmmIter == constMultimap.cbegin() );
    cmmIter = constMultimap.lower_bound(0);
    CHECK( cmmIter == constMultimap.cbegin() );

    for (int i = 1; i < N - 1; ++i)
    {
        cmmIter = constMultimap.lower_bound(i);
        CHECK( std::prev(cmmIter)->second == i - 1 );
        CHECK( (*cmmIter++).second == i );
        CHECK( (cmmIter++)->second == i );
        CHECK( (*cmmIter).second   == i );
        CHECK( (++cmmIter)->second == i + 1);

        cmmRangePair = constMultimap.equal_range(i);  // also check equal_range
        CHECK( std::distance(cmmRangePair.first, cmmRangePair.second) == 3 );
        CHECK( std::all_of(cmmRangePair.first, cmmRangePair.second, [&i](Multimap_t::const_reference kvPair) { return kvPair.first == i; }) );
    }
    cmmIter = constMultimap.lower_bound(N - 1);
    CHECK( std::prev(cmmIter)->second == N - 2 );
    CHECK( (*cmmIter++).second == N - 1 );
    CHECK( (cmmIter++)->second == N - 1 );
    CHECK( (*cmmIter).second   == N - 1 );
    CHECK( (++cmmIter) == constMultimap.cend() );
    cmmIter = constMultimap.lower_bound(N);
    CHECK( cmmIter == constMultimap.cend() );

    // check upper_bound, including edge cases
    cmmIter = constMultimap.upper_bound(-1);
    CHECK( cmmIter == constMultimap.cbegin() );
    for (int i = 0; i < N - 2; ++i)
    {
        cmmIter = constMultimap.upper_bound(i);
        CHECK( std::prev(cmmIter)->second == i );
        CHECK( (*cmmIter++).second == i + 1 );
        CHECK( (cmmIter++)->second == i + 1 );
        CHECK( (*cmmIter).second   == i + 1 );
        CHECK( (++cmmIter)->second == i + 2 );
    }
    cmmIter = constMultimap.upper_bound(N - 2);
    CHECK( std::prev(cmmIter)->second == N - 2 );
    CHECK( (*cmmIter++).second == N - 1 );
    CHECK( (cmmIter++)->second == N - 1 );
    CHECK( (*cmmIter).second   == N - 1 );
    CHECK( ++cmmIter == constMultimap.cend() );
    cmmIter = constMultimap.upper_bound(N - 1);
    CHECK( cmmIter == constMultimap.cend() );
    cmmIter = constMultimap.upper_bound(N);
    CHECK( cmmIter == constMultimap.cend() );
}


/** More robust testing of various insert and emplace functions for maps and multimaps.
*/
void UnitTest7()
{
    // map
    {
        using Map = Skiplist<Movable, Movable, false>;
        Map map;

        // copy-insert {5, 5}
        Map::value_type temp(5, 5);
        auto retval = map.insert(temp);
        CHECK( retval.second == true );
        CHECK( temp.first == 5 );
        CHECK( retval.first->first == 5 );
        CHECK( SkiplistDebug::Validate(map) );

        // move-insert {5, 5}. Should fail, meaning the value_type won't be moved
        retval = map.insert(std::move(temp));
        CHECK( retval.second == false );
        CHECK( temp.first == 5 );
        CHECK( retval.first->first == 5 );
        CHECK( SkiplistDebug::Validate(map) );

        // move-insert {7, 7}. Should succeed, and the value_type will have been moved
        temp = { 7, 7 };
        retval = map.insert(std::move(temp));
        CHECK( retval.second == true );
        CHECK( temp.first == Movable{} );
        CHECK( retval.first->first == 7 );
        CHECK( SkiplistDebug::Validate(map) );

        retval = map.insert({ 3, 3 });
        CHECK( retval.second == true );
        CHECK( retval.first->first == 3 );
        CHECK( SkiplistDebug::Validate(map) );

        retval = map.insert(make_pair(Movable(4), Movable(4)));
        CHECK( retval.second == true );
        CHECK( retval.first->first == 4 );
        CHECK( SkiplistDebug::Validate(map) );


        map.clear();
    
        // good hint
        Map::iterator iter = map.insert(map.end(), { 5, 5 });
        CHECK( iter->first == 5 );
        CHECK( SkiplistDebug::Validate(map) );

        // good hint
        temp = { 7, 7 };
        iter = map.insert(map.end(), temp);
        CHECK( iter->first == 7 );
        CHECK( temp.first == 7 );
        CHECK( SkiplistDebug::Validate(map) );

        // good hint
        temp = { 3, 3 };
        iter = map.insert(map.begin(), std::move(temp));
        CHECK( iter->first == 3 );
        CHECK( temp.first == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // bad hint
        iter = map.insert(map.end(), make_pair(Movable(1), Movable(1)));
        CHECK( iter->first == 1 );
        CHECK( iter == map.begin() );
        CHECK( SkiplistDebug::Validate(map) );

        // bad hint
        iter = map.insert(map.begin(), { 9, 9 });
        CHECK( iter->first == 9 );
        CHECK( SkiplistDebug::Validate(map) );

        // bad hint
        Map::iterator hint = map.find(7);
        CHECK( hint != map.end() );
        iter = map.insert(hint, { 4, 4 });
        CHECK( iter->first == 4 );
        CHECK( SkiplistDebug::Validate(map) );

        // good hint but duplicate key
        CHECK( map.size() == 6 );
        iter = map.insert(hint, { 5, 5 });
        CHECK( iter->first == 5 );
        CHECK( map.size() == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // bad hint and duplicate key
        iter = map.insert(hint, { 7, 7 });
        CHECK( iter->first == 7 );
        CHECK( map.size() == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // bad hint and duplicate key
        iter = map.insert(map.end(), { 7, 7 });
        CHECK( iter->first == 7 );
        CHECK( map.size() == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // bad hint and duplicate key
        iter = map.insert(map.begin(), { 7, 7 });
        CHECK( iter->first == 7 );
        CHECK( map.size() == 6 );
        CHECK( SkiplistDebug::Validate(map) );


        map.clear();

        // copy-emplace {5, 5}
        temp = { 5, 5 };
        retval = map.emplace(temp);
        CHECK( retval.second == true );
        CHECK( temp.first == 5 );
        CHECK( retval.first->first == 5 );
        CHECK( SkiplistDebug::Validate(map) );

        // move-emplace {5, 5}. Should fail. Since this is emplace and not insert, the value_type will be moved
        retval = map.emplace(std::move(temp));
        CHECK( retval.second == false );
        CHECK( temp.first == Movable{} );
        CHECK( retval.first->first == 5 );
        CHECK( SkiplistDebug::Validate(map) );

        // move-emplace {7, 7}. Should succeed, and the value_type will have been moved
        temp = { 7, 7 };
        retval = map.emplace(std::move(temp));
        CHECK( retval.second == true );
        CHECK( temp.first == Movable{} );
        CHECK( retval.first->first == 7 );
        CHECK( SkiplistDebug::Validate(map) );

        // copy-emplace {3, 3}
        Movable m1(3);
        retval = map.emplace(3, m1);
        CHECK( retval.second == true );
        CHECK( retval.first->first == 3 );
        CHECK( m1 == 3 );
        CHECK( SkiplistDebug::Validate(map) );

        // piecewise construct emplace arg/copy {4, 4} 
        retval = map.emplace(std::piecewise_construct, std::forward_as_tuple(4), std::forward_as_tuple(Movable(4)));
        CHECK( retval.second == true );
        CHECK( retval.first->first == 4 );
        CHECK( SkiplistDebug::Validate(map) );

        // piecewise construct emplace copy/move {6, 6}
        m1 = 6;
        Movable m2(6);
        retval = map.emplace(std::piecewise_construct, std::forward_as_tuple(m1), std::forward_as_tuple(std::move(m2)));
        CHECK( retval.second == true );
        CHECK( retval.first->first == 6 );
        CHECK( m1 == 6 );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // try-emplace move {6, 6} fail
        m1 = 6;
        m2 = 6;
        retval = map.try_emplace(std::move(m1), std::move(m2));
        CHECK( retval.second == false );
        CHECK( retval.first->first == 6 );
        CHECK( m1 == 6 );
        CHECK( m2 == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // try-emplace move {0, 0}
        m1 = 0;
        m2 = 0;
        retval = map.try_emplace(std::move(m1), std::move(m2));
        CHECK( retval.second == true );
        CHECK( retval.first->first == 0 );
        CHECK( m1 == Movable{} );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // try-emplace copy/move {0, 0} fail
        m1 = 0;
        m2 = 0;
        retval = map.try_emplace(m1, std::move(m2));
        CHECK( retval.second == false );
        CHECK( retval.first->first == 0 );
        CHECK( m1 == 0 );
        CHECK( m2 == 0 );
        CHECK( SkiplistDebug::Validate(map) );

        // try-emplace copy/move {10, 10}
        m1 = 10;
        m2 = 10;
        retval = map.try_emplace(m1, std::move(m2));
        CHECK( retval.second == true );
        CHECK( retval.first->first == 10 );
        CHECK( m1 == 10 );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // operator[] {-1, null}
        m1 = -1;
        m2 = map[m1];
        CHECK( m1 == -1 );
        CHECK( m2 == Movable{} );
        CHECK( map.size() == 8 );
        CHECK( SkiplistDebug::Validate(map) );

        // m1 will not be moved (because key exists)
        CHECK( map[std::move(m1)] == Movable{} );
        CHECK( m1 == -1 );
        CHECK( map.size() == 8 );
        CHECK( SkiplistDebug::Validate(map) );

        // m1 will not be moved (because key exists)
        map[std::move(m1)] = -1;
        CHECK( m1 == -1 );
        CHECK( map.at(std::move(m1)) == -1 );
        CHECK( m1 == -1 );
        CHECK( map.size() == 8 );
        CHECK( SkiplistDebug::Validate(map) );

        // operator[] {11, 11}
        m1 = 11;
        m2 = 11;
        map[std::move(m1)] = std::move(m2);
        CHECK( m1 == Movable{} );
        CHECK( m2 == Movable{} );
        CHECK( map.size() == 9 );
        CHECK( map.find(11)->first  == 11 );
        CHECK( map.find(11)->second == 11 );
        CHECK( map[11] == 11 );
        CHECK( SkiplistDebug::Validate(map) );

        m1 = 11;
        // m1 will not be moved (because key exists)
        CHECK( map[std::move(m1)] == 11 );
        CHECK( m1 == 11 );
        CHECK( map.size() == 9 );
        CHECK( SkiplistDebug::Validate(map) );

        // operator[] {5, 5}
        m1 = 5;
        CHECK( map[std::move(m1)] == 5 );
        CHECK( m1 == 5 );
        CHECK( map.size() == 9 );
        CHECK( SkiplistDebug::Validate(map) );

        CHECK( map[5] == 5 );
        CHECK( map.size() == 9 );
        CHECK( SkiplistDebug::Validate(map) );

        // operator[] {8, 8}
        CHECK( (map[8] = 8) == 8 );
        CHECK( map.size() == 10 );
        CHECK( SkiplistDebug::Validate(map) );


        map.clear();

        // hinted-try_emplace, good hint
        Map::const_iterator cHint;

        // {5, 5} success
        m1 = 5;
        m2 = 5;
        iter = map.try_emplace(map.cend(), std::move(m1), std::move(m2));
        CHECK( iter->second == 5 );
        CHECK( m1 == Movable{} );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // {5, 6} fail
        m1 = 5;
        m2 = 6;
        cHint = map.cend();
        iter = map.try_emplace(cHint, std::move(m1), std::move(m2));
        CHECK( iter->second == 5 );
        CHECK( map.size() == 1 );
        CHECK( m1 == 5 );
        CHECK( m2 == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // {7, 7} success
        m1 = 7;
        m2 = 7;
        iter = map.try_emplace(cHint, m1, std::move(m2));
        CHECK( iter->first == 7 );
        CHECK( m1 == 7 );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // {7, 8} fail
        m1 = 7;
        m2 = 8;
        cHint = map.find(7);
        iter = map.try_emplace(cHint, std::move(m1), m2);
        CHECK( iter->second == 7 );
        CHECK( map.size() == 2 );
        CHECK( m1 == 7 );
        CHECK( m2 == 8 );
        CHECK( SkiplistDebug::Validate(map) );

        // {3, 3} success
        m1 = 3;
        m2 = 3;
        cHint = map.lower_bound(3);
        iter = map.try_emplace(cHint, std::move(m1), m2);
        CHECK( iter->second == 3 );
        CHECK( m1 == Movable{} );
        CHECK( m2 == 3 );
        CHECK( SkiplistDebug::Validate(map) );

        // {3, 4} fail
        m1 = 3;
        m2 = 4;
        cHint = iter;
        iter = map.try_emplace(cHint, m1, std::move(m2));
        CHECK( iter->second == 3 );
        CHECK( map.size() == 3 );
        CHECK( m1 == 3 );
        CHECK( m2 == 4 );
        CHECK( SkiplistDebug::Validate(map) );

        // {6, 6} success
        cHint = map.upper_bound(6);
        iter = map.try_emplace(cHint, 6, 6);
        CHECK( iter->second == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // {6, 7} fail
        m1 = 6;
        cHint = map.lower_bound(6);
        iter = map.try_emplace(cHint, m1, 7);
        CHECK( iter->second == 6 );
        CHECK( map.size() == 4 );
        CHECK( SkiplistDebug::Validate(map) );


        map.clear();

        // hinted-try_emplace, bad hint

        // {5, 5} success
        m1 = 5;
        m2 = 5;
        iter = map.try_emplace(map.cbegin(), std::move(m1), std::move(m2));
        CHECK( iter->second == 5 );
        CHECK( m1 == Movable{} );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // {5, 6} fail
        m1 = 5;
        m2 = 6;
        cHint = map.cbegin();
        iter = map.try_emplace(cHint, std::move(m1), std::move(m2));
        CHECK( iter->second == 5 );
        CHECK( map.size() == 1 );
        CHECK( m1 == 5 );
        CHECK( m2 == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // {7, 7} success
        m1 = 7;
        m2 = 7;
        cHint = iter;
        iter = map.try_emplace(cHint, m1, std::move(m2));
        CHECK( iter->first == 7 );
        CHECK( m1 == 7 );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(map) );

        // {7, 8} fail
        m1 = 7;
        m2 = 8;
        cHint = map.upper_bound(4);
        iter = map.try_emplace(cHint, std::move(m1), m2);
        CHECK( iter->second == 7 );
        CHECK( map.size() == 2 );
        CHECK( m1 == 7 );
        CHECK( m2 == 8 );
        CHECK( SkiplistDebug::Validate(map) );

        // {3, 3} success
        m1 = 3;
        m2 = 3;
        cHint = map.lower_bound(11);
        iter = map.try_emplace(cHint, std::move(m1), m2);
        CHECK( iter->second == 3 );
        CHECK( m1 == Movable{} );
        CHECK( m2 == 3 );
        CHECK( SkiplistDebug::Validate(map) );

        // {3, 4} fail
        m1 = 3;
        m2 = 4;
        cHint = map.upper_bound(7);
        iter = map.try_emplace(cHint, m1, std::move(m2));
        CHECK( iter->second == 3 );
        CHECK( map.size() == 3 );
        CHECK( m1 == 3 );
        CHECK( m2 == 4 );
        CHECK( SkiplistDebug::Validate(map) );

        // {6, 6} success
        cHint = map.upper_bound(-1);
        iter = map.try_emplace(cHint, 6, 6);
        CHECK( iter->second == 6 );
        CHECK( SkiplistDebug::Validate(map) );

        // {6, 7} fail
        m1 = 6;
        cHint = map.lower_bound(-1);
        iter = map.try_emplace(cHint, m1, 7);
        CHECK( iter->second == 6 );
        CHECK( map.size() == 4 );
        CHECK( SkiplistDebug::Validate(map) );

    }

    // multimap
    {
        using Multimap = Skiplist<Movable, Movable, true>;
        Multimap mmap;

        // copy-insert {5, 5}
        Multimap::value_type temp(5, 5);
        Multimap::iterator iter = mmap.insert(temp);
        CHECK( temp.first == 5 );
        CHECK( iter->first == 5 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // move-insert {5, 5}. Should succeed, and the value_type will have been moved
        iter = mmap.insert(std::move(temp));
        CHECK( temp.first == Movable{} );
        CHECK( iter->first == 5 );
        CHECK( SkiplistDebug::Validate(mmap) );

        iter = mmap.insert({ 3, 3 });
        CHECK( iter->first == 3 );
        CHECK( SkiplistDebug::Validate(mmap) );

        iter = mmap.insert(make_pair(Movable(4), Movable(4)));
        CHECK( iter->first == 4 );
        CHECK( SkiplistDebug::Validate(mmap) );


        mmap.clear();
    
        // good hint
        iter = mmap.insert(mmap.end(), { 5, 5 });
        CHECK( iter->first == 5 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // good hint
        temp = { 7, 7 };
        iter = mmap.insert(mmap.end(), temp);
        CHECK( iter->first == 7 );
        CHECK( temp.first == 7 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // good hint
        temp = { 3, 3 };
        iter = mmap.insert(mmap.begin(), std::move(temp));
        CHECK( iter->first == 3 );
        CHECK( temp.first == Movable{} );
        CHECK( SkiplistDebug::Validate(mmap) );

        // bad hint
        iter = mmap.insert(mmap.end(), make_pair(Movable(1), Movable(1)));
        CHECK( iter->first == 1 );
        CHECK( iter == mmap.begin() );
        CHECK( SkiplistDebug::Validate(mmap) );

        // bad hint
        iter = mmap.insert(mmap.begin(), { 9, 9 });
        CHECK( iter->first == 9 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // bad hint
        Multimap::iterator hint = mmap.find(7);
        CHECK( hint != mmap.end() );
        iter = mmap.insert(hint, { 4, 4 });
        CHECK( iter->first == 4 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // good hint, duplicate key
        CHECK( mmap.size() == 6 );
        iter = mmap.insert(hint, { 5, 5 });
        CHECK( iter->first == 5 );
        CHECK( mmap.size() == 7 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // good hint, duplicate key
        iter = mmap.insert(hint, { 7, 7 });
        CHECK( iter->first == 7 );
        CHECK( iter == mmap.lower_bound(7) );
        CHECK( std::next(iter) == hint );
        CHECK( std::next(hint) == mmap.upper_bound(7) );
        CHECK( mmap.size() == 8 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // bad hint and duplicate key
        iter = mmap.insert(mmap.end(), { 7, 7 });
        CHECK( iter->first == 7 );
        CHECK( mmap.size() == 9 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // bad hint and duplicate key
        iter = mmap.insert(mmap.begin(), { 7, 7 });
        CHECK( iter->first == 7 );
        CHECK( mmap.size() == 10 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // good hint, duplicate key
        iter = mmap.insert(mmap.begin(), { 1, 1 });
        CHECK( iter == mmap.begin() );
        CHECK( iter->first == 1 );
        CHECK( mmap.size() == 11 );
        CHECK(SkiplistDebug::Validate(mmap));

        // good hint, duplicate key
        iter = mmap.insert(mmap.end(), { 9, 9 });
        CHECK( iter->first == 9 );
        CHECK( mmap.size() == 12 );
        CHECK( std::next(iter) == mmap.end() );
        CHECK(SkiplistDebug::Validate(mmap));

        // good hint, duplicate key
        hint = iter;
        iter = mmap.insert(hint, { 9, 9 });
        CHECK( iter->first == 9 );
        CHECK( mmap.size() == 13 );
        CHECK( std::next(iter) == hint );
        CHECK(SkiplistDebug::Validate(mmap));


        mmap.clear();

        // copy-emplace {5, 5}
        temp = { 5, 5 };
        iter = mmap.emplace(temp);
        CHECK( temp.first == 5 );
        CHECK( iter->first == 5 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // move-emplace {5, 5}. Will succeed. The value_type will have been moved
        iter = mmap.emplace(std::move(temp));
        CHECK( temp.first == Movable{} );
        CHECK( iter->first == 5 );
        CHECK( SkiplistDebug::Validate(mmap) );

        // move-emplace {7, 7}. Should succeed, and the value_type will have been moved
        temp = { 7, 7 };
        iter = mmap.emplace(std::move(temp));
        CHECK( temp.first == Movable{} );
        CHECK( iter->first == 7 );
        CHECK( SkiplistDebug::Validate(mmap) );

        Movable m1(3);
        iter = mmap.emplace(3, m1);
        CHECK( iter->first == 3 );
        CHECK( iter->second == 3 );
        CHECK( m1 == 3 );
        CHECK( SkiplistDebug::Validate(mmap) );

        iter = mmap.emplace(std::piecewise_construct, std::forward_as_tuple(4), std::forward_as_tuple(Movable(4)));
        CHECK( iter->first == 4 );
        CHECK( SkiplistDebug::Validate(mmap) );

        m1 = 6;
        Movable m2(6);
        iter = mmap.emplace(std::piecewise_construct, std::forward_as_tuple(m1), std::forward_as_tuple(std::move(m2)));
        CHECK( iter->first == 6 );
        CHECK( iter->second == 6 );
        CHECK( m1 == 6 );
        CHECK( m2 == Movable{} );
        CHECK( SkiplistDebug::Validate(mmap) );
    }
}


/** random / mem test for insert and emplace in maps and multimaps
MULTIMAP indicates multimap (if true) or map (if false)
*/
template <bool MULTIMAP>
void UnitTest8()
{
#ifdef _MSC_VER
    const bool isMsvc = true;
#else
    const bool isMsvc = false;
#endif
    // MSVC's std::multimap insert-with-hint has slightly different behavior than GCC's.
    // The difference is how insert-with-hint handles a bad hint.
    // Skiplist follows GCC behavior. Don't test insert with bad hint on MSVC
    const int testRange = (MULTIMAP && isMsvc) ? 0 : -1;

    using Skiplist_t = Skiplist<Movable, Movable, MULTIMAP, std::greater<Movable>>;
    using MControl_t = typename std::conditional<MULTIMAP, std::multimap<Movable, Movable, std::greater<Movable>>, std::map<Movable, Movable, std::greater<Movable>>>::type;
    static_assert(Skiplist_t::s_MULTIMAP == MULTIMAP, "s_MULTIMAP not set properly at compile-time.");
    CHECK( Skiplist_t::is_multimap() == MULTIMAP );
    Skiplist_t skiplist;
    MControl_t mcontrol;

    std::mt19937_64 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> keyValRange(0, 1000000);
    std::uniform_int_distribution<int> chooseFunction(testRange, 5);
    auto rng = [&gen, &keyValRange] { return keyValRange(gen); };

    const int N = 100000;

    typename Skiplist_t::const_iterator sHint;
    typename MControl_t::const_iterator mHint;
    typename Skiplist_t::iterator sIter;
    typename MControl_t::iterator mIter;
    bool sInsertSuccess;
    bool mInsertSuccess;
    Movable key;

    for (int i = 0; i < N; ++i)
    {
        key = Movable(rng());

        switch (chooseFunction(gen))
        {
        case -1:
        {
            // insert with random hint (i.e. bad)
            const int randomKey = rng();
            sHint = skiplist.lower_bound(randomKey);
            mHint = mcontrol.lower_bound(randomKey);
            sIter = skiplist.insert(sHint, typename Skiplist_t::value_type(key, i));
            mIter = mcontrol.insert(mHint, typename MControl_t::value_type(key, i));
            CHECK( sIter->first  == mIter->first  );
            CHECK( sIter->second == mIter->second );
            break;
        }

        case 0:
        {
            // insert
            std::pair<Movable, Movable> p(key, i);
            auto sRetval = skiplist.insert(p);
            auto mRetval = mcontrol.insert(p);
            std::tie(sIter, sInsertSuccess) = extractInsertRetval(sRetval);
            std::tie(mIter, mInsertSuccess) = extractInsertRetval(mRetval);
            CHECK( sInsertSuccess == mInsertSuccess );
            CHECK( sIter->first  == mIter->first  );
            CHECK( sIter->second == mIter->second );
            CHECK( p.first  == key );
            CHECK( p.second == i );
            break;
        }

        case 1:
        {
            // insert with good hint
            sHint = skiplist.upper_bound(key);
            mHint = mcontrol.upper_bound(key);
            sIter = skiplist.insert(sHint, std::pair<Movable, Movable>(key, i));
            mIter = mcontrol.insert(mHint, std::pair<Movable, Movable>(key, i));
            CHECK( sIter->first  == mIter->first  );
            CHECK( sIter->second == mIter->second );
            break;
        }

        case 2:
        {
            // emplace (value_type)
            auto sRetval = skiplist.emplace(typename Skiplist_t::value_type(key, i));
            auto mRetval = mcontrol.emplace(typename MControl_t::value_type(key, i));
            std::tie(sIter, sInsertSuccess) = extractInsertRetval(sRetval);
            std::tie(mIter, mInsertSuccess) = extractInsertRetval(mRetval);
            CHECK( sInsertSuccess == mInsertSuccess );
            CHECK( sIter->first  == mIter->first  );
            CHECK( sIter->second == mIter->second );
            break;
        }

        case 3:
        {
            // emplace (args)
            auto sRetval = skiplist.emplace(key, i);
            auto mRetval = mcontrol.emplace(key, i);
            std::tie(sIter, sInsertSuccess) = extractInsertRetval(sRetval);
            std::tie(mIter, mInsertSuccess) = extractInsertRetval(mRetval);
            CHECK( sInsertSuccess == mInsertSuccess );
            CHECK( sIter->first  == mIter->first  );
            CHECK( sIter->second == mIter->second );
            break;
        }

        case 4:
        {
            // emplace (piecewise construct)
            auto sRetval = skiplist.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(i));
            auto mRetval = mcontrol.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(i));
            std::tie(sIter, sInsertSuccess) = extractInsertRetval(sRetval);
            std::tie(mIter, mInsertSuccess) = extractInsertRetval(mRetval);
            CHECK( sInsertSuccess == mInsertSuccess );
            CHECK( sIter->first  == mIter->first  );
            CHECK( sIter->second == mIter->second );
            break;
        }

        case 5:
        {
            // emplace with good hint
            sHint = skiplist.upper_bound(key);
            mHint = mcontrol.upper_bound(key);
            sIter = skiplist.emplace_hint(sHint, key, i);
            mIter = mcontrol.emplace_hint(mHint, key, i);
            CHECK( sIter->first  == mIter->first  );
            CHECK( sIter->second == mIter->second );
            break;
        }

        default:
            assert(false && "Random distribution has bad range.");
        }
    }

    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.size() == mcontrol.size() );
    
    for (sIter = skiplist.begin(), mIter = mcontrol.begin(); sIter != skiplist.end(); ++sIter, ++mIter)
    {
        CHECK( sIter->first == mIter->first && sIter->second == mIter->second );
    }

}


/** Test lexographical comparisons
*/
void UnitTest9()
{
    // maps
    {
        auto verify = [](std::initializer_list<std::pair<const Movable, Movable>> initList1,
                         std::initializer_list<std::pair<const Movable, Movable>> initList2)
        {
            Skiplist<Movable, Movable> s1 = initList1;
            CHECK( SkiplistDebug::Validate(s1) );
            std::map<Movable, Movable> m1 = initList1;
            Skiplist<Movable, Movable> s2 = initList2;
            CHECK( SkiplistDebug::Validate(s2) );
            std::map<Movable, Movable> m2 = initList2;
            CHECK( (s1 == s2) == (m1 == m2) );
            CHECK( (s1 != s2) == (m1 != m2) );
            CHECK( (s1 <  s2) == (m1 <  m2) );
            CHECK( (s1 >  s2) == (m1 >  m2) );
            CHECK( (s1 <= s2) == (m1 <= m2) );
            CHECK( (s1 >= s2) == (m1 >= m2) );
        };

        std::initializer_list<std::pair<const Movable, Movable>> initList{
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 4, 4 },
            { 5, 5 } };
        std::initializer_list<std::pair<const Movable, Movable>> equal{
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 4, 4 },
            { 5, 5 } };
        std::initializer_list<std::pair<const Movable, Movable>> valLess{
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 4, 3 },
            { 5, 5 } };
        std::initializer_list<std::pair<const Movable, Movable>> valGreater{
            { 1, 1 },
            { 2, 2 },
            { 4, 3 },
            { 4, 4 },
            { 5, 6 } };
        std::initializer_list<std::pair<const Movable, Movable>> keyLess{
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 4, 4 },
            { 0, 5 } };
        std::initializer_list<std::pair<const Movable, Movable>> keyGreater{
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 5, 5 },
            { 6, 6 } };
        std::initializer_list<std::pair<const Movable, Movable>> lenLess{
            { 2, 2 },
            { 3, 3 },
            { 4, 4 },
            { 5, 5 } };
        std::initializer_list<std::pair<const Movable, Movable>> lenGreater{
            { 0, 0 },
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 4, 4 },
            { 5, 5 } };

        verify(initList, equal);
        verify(initList, valLess);
        verify(initList, valGreater);
        verify(initList, keyLess);
        verify(initList, keyGreater);
        verify(initList, lenLess);
        verify(initList, lenGreater);
    }

    // multimaps
    {
        auto verify = [](std::initializer_list<std::pair<const Movable, Movable>> initList1,
                         std::initializer_list<std::pair<const Movable, Movable>> initList2)
        {
            Skiplist<Movable, Movable, true> s1 = initList1;
            CHECK( SkiplistDebug::Validate(s1) );
            std::multimap<Movable, Movable> m1 = initList1;
            Skiplist<Movable, Movable, true> s2 = initList2;
            CHECK( SkiplistDebug::Validate(s2) );
            std::multimap<Movable, Movable> m2 = initList2;
            CHECK( (s1 == s2) == (m1 == m2) );
            CHECK( (s1 != s2) == (m1 != m2) );
            CHECK( (s1 <  s2) == (m1 <  m2) );
            CHECK( (s1 >  s2) == (m1 >  m2) );
            CHECK( (s1 <= s2) == (m1 <= m2) );
            CHECK( (s1 >= s2) == (m1 >= m2) );
        };

        std::initializer_list<std::pair<const Movable, Movable>> initList{
            { 1, 1 },
            { 1, 2 },
            { 3, 3 },
            { 3, 4 },
            { 5, 5 },
            { 5, 6 } };
        std::initializer_list<std::pair<const Movable, Movable>> equal{
            { 1, 1 },
            { 1, 2 },
            { 3, 3 },
            { 3, 4 },
            { 5, 5 },
            { 5, 6 } };
        std::initializer_list<std::pair<const Movable, Movable>> valLess{
            { 1, 0 },
            { 1, 2 },
            { 3, 3 },
            { 3, 4 },
            { 5, 5 },
            { 5, 5 } };
        std::initializer_list<std::pair<const Movable, Movable>> valGreater{
            { 1, 1 },
            { 1, 2 },
            { 3, 3 },
            { 3, 4 },
            { 5, 5 },
            { 5, 7 } };
        std::initializer_list<std::pair<const Movable, Movable>> keyLess{
            { 1, 1 },
            { 1, 2 },
            { 2, 3 },
            { 3, 4 },
            { 5, 5 },
            { 5, 6 } };
        std::initializer_list<std::pair<const Movable, Movable>> keyGreater{
            { 1, 1 },
            { 1, 2 },
            { 3, 3 },
            { 3, 4 },
            { 6, 5 },
            { 6, 6 } };
        std::initializer_list<std::pair<const Movable, Movable>> lenLess{
            { 7, 1 },
            { 8, 2 },
            { 9, 3 },
            { 9, 4 },
            { 9, 5 } };
        std::initializer_list<std::pair<const Movable, Movable>> lenGreater{
            { 0, 1 },
            { 1, 2 },
            { 3, 3 },
            { 3, 4 },
            { 5, 5 },
            { 5, 6 },
            { 7, 7 } };

        verify(initList, equal);
        verify(initList, valLess);
        verify(initList, valGreater);
        verify(initList, keyLess);
        verify(initList, keyGreater);
        verify(initList, lenLess);
        verify(initList, lenGreater);
    }
}


// ------------------------------------------------------------------
// Iterator Tests

/** Helper function for Iterator Test 1
const/non-const test with auto
checks that iterators do iteration in correct order by comparing against a map with the same contents
@param[in] slIter  start iterator for skiplist
@param[in] slEnd   exclusive end iterator for skiplist
@param[in] mapIter start iterator for map. Map should have contents matching the skiplist.
@param[in] mapEnd  exclusive end iterator for map
*/
template <typename SKIPLIST_ITER, typename MAP_ITER>
void iterateHelper(SKIPLIST_ITER slIter, SKIPLIST_ITER slEnd, MAP_ITER mapIter, MAP_ITER mapEnd)
{
    for (; slIter != slEnd && mapIter != mapEnd; ++slIter, ++mapIter)
    {
        CHECK( slIter->first  == mapIter->first  );
        CHECK( slIter->second == mapIter->second );
    }
    CHECK( slIter == slEnd && mapIter == mapEnd );
}


/** Helper function for Iterator Test 1
const/non-const with range-based for
checks that range-based-for works with iterators by comparing against a map with the same contents
range-for also tests operator* for iterators
@param[in] skiplist   The skiplist to iterator over
@param[in] mapControl A map with the contents matching the skiplist
*/
template <typename SKIPLIST, typename MAP>
void rangeBasedFor(SKIPLIST& skiplist, MAP& mapControl)
{
    auto mapIter = mapControl.begin();
    // range-for also tests operator* for iterators
    for (auto& val : skiplist)
    {
        CHECK( mapIter != mapControl.end() );
        CHECK( val.first  == mapIter->first  );
        CHECK( val.second == mapIter->second );
        ++mapIter;
    }
    CHECK( mapIter == mapControl.end() );
}


/** Iterator Test 1
Tests normal iterating and range-based-for
forward and reverse iteration
balancing action of iterators
const iterators
*/
void IteratorTest1()
{
    using Skiplist_t = Skiplist<Movable, Movable>;
    const int N = 10000;

    static_assert(std::is_convertible<Skiplist_t::iterator, Skiplist_t::const_iterator>::value, "Skiplist iterator must be convertible to const_iterator.");

    // create test set and std::map to check
    // We'll be doing this several times, so save the test set in an std::vector
    std::vector<Movable> testset;
    testset.reserve(N);
    Skiplist_t skiplist;
    std::map<Movable, Movable> control;
    std::mt19937 rng;
    std::uniform_int_distribution<int> distribution;
    for (int i = 0; i < N; ++i)
    {
        auto number = distribution(rng);
        if (skiplist.emplace(number, i).second)
        {
            testset.push_back(number);
            control[number] = i;
        }
    }
    CHECK( SkiplistDebug::Validate(skiplist) );

    // lambda for resetting skiplist
    auto resetSkiplist = [&skiplist, &testset]() 
    {
        skiplist.clear();
        for (unsigned i = 0; i < testset.size(); ++i)
            skiplist.emplace(testset[i], i);
        CHECK( SkiplistDebug::Validate(skiplist) );
        CHECK( skiplist.size() == testset.size() );
    };

    // const reference to container
    const Skiplist_t& constSkiplist{ skiplist };

    // check cbegin() returns non-balancing const_iterator
    auto constSkiplistIter1 = skiplist.cbegin();
    static_assert(std::is_same<
        decltype(constSkiplistIter1), 
        SkiplistNonBalancingIterator<Skiplist_t>>::value, "wrong return value: cbegin()");
        
    // check begin() const returns non-balancing const_iterator on const container
    auto constSkiplistIter2 = constSkiplist.begin();
    static_assert(std::is_same<
        decltype(constSkiplistIter2), 
        SkiplistNonBalancingIterator<Skiplist_t>>::value, "wrong return value: begin() const");

    // check begin() returns balancing iterator on non-const container
    auto skiplistIter1 = skiplist.begin();
    static_assert(std::is_same<
        decltype(skiplistIter1),
        SkiplistBalancingIterator<Skiplist_t>>::value, "wrong return value: begin()");

    // check cbegin() returns non-balancing const_iterator
    auto constSkiplistIter3 = skiplist.crbegin();
    static_assert(std::is_same<
        decltype(constSkiplistIter3),
        std::reverse_iterator<SkiplistNonBalancingIterator<Skiplist_t>>>::value, "wrong return value: crbegin()");
        
    // check begin() const returns non-balancing const_iterator on const container
    auto constSkiplistIter4 = constSkiplist.rbegin();
    static_assert(std::is_same<
        decltype(constSkiplistIter4), 
        std::reverse_iterator<SkiplistNonBalancingIterator<Skiplist_t>>>::value, "wrong return value: rbegin() const");

    // check begin() returns balancing iterator on non-const container
    auto skiplistIter2 = skiplist.rbegin();
    static_assert(std::is_same<
        decltype(skiplistIter2),
        std::reverse_iterator<SkiplistBalancingIterator<Skiplist_t>>>::value, "wrong return value: rbegin()");


    // test iterator swap functions
    // non-const
    auto swapIterLeft  = skiplist.find(testset[N / 2]);
    auto swapIterRight = skiplist.find(testset[N / 2 + 1]);
    auto distLeft  = std::distance(swapIterLeft,  skiplist.end());
    auto distRight = std::distance(swapIterRight, skiplist.end());
    using std::swap;
    swap(swapIterLeft, swapIterRight);
    CHECK( swapIterLeft->GetVal() == N / 2 + 1 && swapIterRight->GetVal() == N / 2 );
    CHECK( distRight == std::distance(swapIterLeft,  skiplist.end()) );
    CHECK( distLeft  == std::distance(swapIterRight, skiplist.end()) );
    // const
    auto swapConstIterLeft  = constSkiplist.find(testset[N / 2]);
    auto swapConstIterRight = constSkiplist.find(testset[N / 2 + 1]);
    swap(swapConstIterLeft, swapConstIterRight);
    CHECK( swapConstIterLeft->GetVal() == N / 2 + 1 && swapConstIterRight->GetVal() == N / 2 );
    CHECK( distRight == std::distance(swapConstIterLeft,  skiplist.cend()) );
    CHECK( distLeft  == std::distance(swapConstIterRight, skiplist.cend()) );
    CHECK( SkiplistDebug::Validate(constSkiplist) );


    // test iterator_traits
    using it  = Skiplist_t::iterator;
    using cit = Skiplist_t::const_iterator;
    using itTraits  = std::iterator_traits<it>;
    using citTraits = std::iterator_traits<cit>;

    static_assert(std::is_same<itTraits::difference_type,   it::difference_type>::value,   "iterator traits: no difference_type");
    static_assert(std::is_same<itTraits::value_type,        it::value_type>::value,        "iterator traits: no value_type");
    static_assert(std::is_same<itTraits::pointer,           it::pointer>::value,           "iterator traits: no pointer");
    static_assert(std::is_same<itTraits::reference,         it::reference>::value,         "iterator traits: no reference");
    static_assert(std::is_same<itTraits::iterator_category, it::iterator_category>::value, "iterator traits: no iterator_category");

    static_assert(std::is_same<citTraits::difference_type,   cit::difference_type>::value,   "const_iterator traits: no difference_type");
    static_assert(std::is_same<citTraits::value_type,        cit::value_type>::value,        "const_iterator traits: no value_type");
    static_assert(std::is_same<citTraits::pointer,           cit::pointer>::value,           "const_iterator traits: no pointer");
    static_assert(std::is_same<citTraits::reference,         cit::reference>::value,         "const_iterator traits: no reference");
    static_assert(std::is_same<citTraits::iterator_category, cit::iterator_category>::value, "const_iterator traits: no iterator_category");


    // const/non-const test with auto
    iterateHelper(constSkiplist.begin(), constSkiplist.end(), control.begin(), control.end());
    CHECK( SkiplistDebug::Validate(constSkiplist) );
    CHECK( !constSkiplist.is_balanced() );
    iterateHelper(skiplist.begin(), skiplist.end(), control.begin(), control.end());
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( constSkiplist.is_balanced() );

    resetSkiplist();

    // const/non-const test with auto, reverse iteration
    iterateHelper(constSkiplist.rbegin(), constSkiplist.rend(), control.rbegin(), control.rend());
    CHECK( SkiplistDebug::Validate(constSkiplist) );
    CHECK( !constSkiplist.is_balanced() );
    iterateHelper(skiplist.rbegin(), skiplist.rend(), control.rbegin(), control.rend());
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( constSkiplist.is_balanced() );

    resetSkiplist();

    // const/non-const with range-based for
    rangeBasedFor(constSkiplist, control);
    CHECK( SkiplistDebug::Validate(constSkiplist) );
    CHECK( !constSkiplist.is_balanced() );
    rangeBasedFor(skiplist, control);
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( constSkiplist.is_balanced() );

    // check that iterators that are const can still be dereferenced
    const it iter = skiplist.begin();
    CHECK( iter->first == control.begin()->first );
    const cit citer = skiplist.cbegin();
    CHECK( citer->first == control.cbegin()->first );
}


/** Iterator Test 2
tests post-fix increment and decrement
also tests COMPARE template argument with std::greater
*/
void IteratorTest2()
{
    using Skiplist_t = Skiplist<int, int, false, std::greater<int>>;
    const int N = 100;

    auto GetSkiplist = [N]()->Skiplist_t {
        Skiplist_t skiplist;
        for (int i = 0; i < N; ++i)
            skiplist.emplace(i, 0);
        CHECK( SkiplistDebug::Validate(skiplist) );
        CHECK( !skiplist.is_balanced() );  // chances are extremely low
        return skiplist; 
    };
    auto compareValLess    = [](Skiplist_t::const_iterator::value_type left, Skiplist_t::const_iterator::value_type right) { return left.second < right.second; };
    auto compareValGreater = [](Skiplist_t::const_iterator::value_type left, Skiplist_t::const_iterator::value_type right) { return left.second > right.second; };
    auto compareKeyGreater = [](Skiplist_t::const_iterator::value_type left, Skiplist_t::const_iterator::value_type right) { return left.first  > right.first;  };

    auto skiplist = GetSkiplist();
    CHECK( std::is_sorted(skiplist.cbegin(), skiplist.cend(), compareKeyGreater) );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( !skiplist.is_balanced() );
    

    Skiplist_t::const_iterator cIter;
    cIter = --skiplist.cend();
    for (int i = 0; i < N-1; ++i)
        CHECK( (*cIter--).second == 0 );
    CHECK( cIter == skiplist.cbegin() );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( !skiplist.is_balanced() );

    Skiplist_t::const_reverse_iterator crIter;
    int sum = 0;
    crIter = --skiplist.crend();
    for (int i = 0; i < N-1; ++i)
        sum += (*crIter--).second;
    CHECK( crIter == skiplist.crbegin() );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( !skiplist.is_balanced() );
    CHECK( sum == 0 );

    Skiplist_t::iterator iter;
    iter = --skiplist.end();
    for (int i = 0; i < N-1; ++i)
        (*iter--).GetVal() = i;
    iter->GetVal() = N-1;
    CHECK( iter == skiplist.begin() );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.is_balanced() );
    CHECK( std::is_sorted(skiplist.crbegin(), skiplist.crend(), compareValLess) );

    Skiplist_t::reverse_iterator rIter;
    rIter = --skiplist.rend();
    for (int i = 0; i < N-1; ++i)
        (*rIter--).SetVal(i);
    rIter->SetVal(N-1);
    CHECK( rIter == skiplist.rbegin() );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.is_balanced() );
    CHECK( std::is_sorted(skiplist.crbegin(), skiplist.crend(), compareValGreater) );

    skiplist = GetSkiplist();
    CHECK( SkiplistDebug::Validate(skiplist) );

    rIter = --skiplist.rend();
    for (int i = 0; i < N-1; ++i)
        (*rIter--).second = i;
    rIter->second = N-1;
    CHECK( rIter == skiplist.rbegin() );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.is_balanced() );
    CHECK( std::is_sorted(skiplist.cbegin(), skiplist.cend(), compareValLess) );

    skiplist = GetSkiplist();
    CHECK( SkiplistDebug::Validate(skiplist) );

    cIter = skiplist.cbegin();
    for (int i = 0; i < N; ++i)
        CHECK( (*cIter++).second == 0 );
    CHECK( cIter == skiplist.cend() );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( !skiplist.is_balanced() );

    iter = skiplist.begin();
    for (int i = 0; i < N; ++i)
        (*iter++).second = i;
    CHECK( iter == skiplist.end() );
    CHECK( SkiplistDebug::Validate(skiplist) );
    CHECK( skiplist.is_balanced() );
    CHECK( std::is_sorted(skiplist.cbegin(), skiplist.cend(), compareValLess) );

}


// ------------------------------------------------------------------

/** Makes sure that the initializer list constructors work.
*/
void CompileCheck1()
{
    std::map<int, int> ma{    { 1, 1 }, { 2, 2 }, { 3, 3 } };
    std::map<int, int> mb = { { 1, 1 }, { 2, 2 }, { 3, 3 } };
    std::map<int, int> mc(  { { 1, 1 }, { 2, 2 }, { 3, 3 } });

    Skiplist<int, int> sa1{    { 1, 1 }, { 2, 2 }, { 3, 3 } };
    Skiplist<int, int> sb1 = { { 1, 1 }, { 2, 2 }, { 3, 3 } };
    Skiplist<int, int> sc1(  { { 1, 1 }, { 2, 2 }, { 3, 3 } });

    Skiplist<int, int, true> sa2{    { 1, 1 }, { 2, 2 }, { 3, 3 } };
    Skiplist<int, int, true> sb2 = { { 1, 1 }, { 2, 2 }, { 3, 3 } };
    Skiplist<int, int, true> sc2({   { 1, 1 }, { 2, 2 }, { 3, 3 } });

    sb1 = { { 1, 1 }, { 2, 2 }, { 3, 3 } };

    // didn't have a better place to put this since there aren't SLPair unit tests
    const SLPairConst<Movable, Movable> cp(3, 4);
    SLPair<Movable, Movable> p = { 1, 2 };
    p = { 5, 6 };
    p = Skiplist<Movable, Movable>::value_type(cp);
    CHECK( p.first  == cp.first  );
    CHECK( p.second == cp.second );
}


// ------------------------------------------------------------------

/** Run the unit tests for class Skiplist
@param[in] testSelection allow slow tests to be skipped or run in parallel
*/
void RunUnitTests(const ETestSelection testSelection)
{
    UnitTest1();
    UnitTest2();
    UnitTest5();
    UnitTest6();
    UnitTest7();
    UnitTest8<false>();
    UnitTest8<true>();
    UnitTest9();

    // 3 and 4 take a long time
    if (testSelection == ETestSelection::FULL)
    {
        UnitTest3();
        UnitTest4<false>();
        UnitTest4<true>();
    }
    else if (testSelection == ETestSelection::FULL_MULTITHREADED)
    {
        const bool bit64 = (sizeof(int*) >= 8);
        if (bit64)  // 64-bit (or more) build
        {
            std::thread ut3(UnitTest3);
            std::thread ut4a(UnitTest4<false>);
            std::thread ut4b(UnitTest4<true>);
            ut3.join();
            ut4a.join();
            ut4b.join();
        }
        else  // less than 64-bit build
        {
            // If you run all 3 of these at the same time, you will go past 4GB.
            // So on 32-bit builds, we can only run 2 at a time.
            // Run 4b on a separate thread and 3 and 4a on the main thread.

            std::thread ut4b(UnitTest4<true>);

            UnitTest3();
            UnitTest4<false>();

            ut4b.join();
        }
    }
}


/** Run the unit tests for the iterators
*/
void RunIteratorTests()
{
    IteratorTest1();
    IteratorTest2();
}


/** These don't need to run as much as they just need to compile.
*/
void CompileChecks()
{
    CompileCheck1();
}


// ------------------------------------------------------------------
// Parse Arguments

/** Print out the possible arguments
*/
void displayHelp()
{
    std::cout 
        << "Arguments: \n"
        << "--fast                Skip slow-running stress tests and memory leak tests. \n"
        << "--full                Run all tests. \n"
        << "--multicore (default) Run all tests. Slow tests run in parallel. \n"
        << "--help                Display this message. \n"
        << std::endl;             
}


/** parse the command arguments
@param[in]  argc              The number of arguments passed to main
@param[in]  argv              The arguments passed to main
@param[out] out_testSelection The tests to run.
@return true if parsing was successful. false if parsing was not successful and out_testSelection is invalid.
*/
bool parseArgs(const int argc, char const* const* const argv, ETestSelection& out_testSelection)
{
    // no args
    if (argc == 1)
    {
        out_testSelection = ETestSelection::FULL_MULTITHREADED;
        return true;
    }
    if (argc > 1 && std::char_traits<char>::length(argv[1]) < 40)
    {
        const std::string arg1(argv[1]);
        if (arg1.compare("--fast") == 0)
        {
            out_testSelection = ETestSelection::FAST;
            return true;
        }
        else if (arg1.compare("--full") == 0)
        {
            out_testSelection = ETestSelection::FULL;
            return true;
        }
        else if (arg1.compare("--multicore") == 0)
        {
            out_testSelection = ETestSelection::FULL_MULTITHREADED;
            return true;
        }
        else if (arg1.compare("--help") == 0)
        {
            displayHelp();
            return false;
        }
    }

    std::cout << "Invalid argument: " << argv[1] << "\n\n";
    displayHelp();

    return false;
}


// ------------------------------------------------------------------
// Main

int main(int argc, char** argv)
{
    ETestSelection testSelection;
    if (parseArgs(argc, argv, testSelection))
    {
        RunUnitTests(testSelection);
        RunIteratorTests();
        CompileChecks();
    }

    return 0;
}
