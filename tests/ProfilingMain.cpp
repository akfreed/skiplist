// ===========================================================================
// Copyright (c) 2018 Alexander Freed. ALL RIGHTS RESERVED.
//
// ISO C++11
// 
// Contains the code for doing profiling tests on Skiplist
// ===========================================================================

#include <Skiplist.hpp>
#include "Movable.h"

#include <iostream>
#include <string>
#include <random>
#include <algorithm>
#include <numeric>
#include <map>
#include <chrono>

using namespace fsl;


// ==================================================================

/** Profiling Test
Creates an input array--the shuffled integers from 0 to 999,999
Tests Add, Retrieve, and Remove
*/
template <typename CONTAINER>
void ProfilingTest1()
{
    using namespace std::chrono;

    // generate test input. 1 million entries
    // keys should be random and not repeated. Seed should be constant to ensure reproducibility.
    const int N = 1000000;
    using T = int;
    
    std::mt19937 rand;

    std::vector<T> input(N);
    std::iota(input.begin(), input.end(), 0);
    std::shuffle(input.begin(), input.end(), rand);


    std::vector<T> output(N, -1);

    CONTAINER container;
    using iterator = typename CONTAINER::iterator;

    high_resolution_clock::time_point start;
    high_resolution_clock::time_point stop;

    // insert
    start = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
    {
        container.emplace(input[i], i);
    }
    stop = high_resolution_clock::now();
    auto diff = stop - start;
    std::cout << "insert: " << diff.count() / 1000000.0 << std::endl;

    // find
    iterator iter;
    start = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
    {
        iter = container.find(input[i]);
        output[i] = iter->first;
    }
    stop = high_resolution_clock::now();
    diff = stop - start;
    std::cout << "find:   " << diff.count() / 1000000.0 << std::endl;

    // erase
    start = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
    {
        container.erase(input[i]);
    }
    stop = high_resolution_clock::now();
    diff = stop - start;
    std::cout << "erase:  " << diff.count() / 1000000.0 << std::endl;

    std::cout << output[0] << std::endl;
}


template <typename CONTAINER>
void ProfilingTest2()
{
    using namespace std::chrono;

    CONTAINER container;
    for (int i = 0; i < 1000000; ++i)
    {
        container.emplace(i, i);
    }

    typename CONTAINER::iterator iter;
    volatile int x;
    high_resolution_clock::time_point start = high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i)
    {
        x = container.begin()->second;
    }
    high_resolution_clock::time_point stop = high_resolution_clock::now();
    auto diff = stop - start;
    std::cout << "time: " << diff.count() / 1000000.0 << std::endl;
}


// ------------------------------------------------------------------
// Parse Arguments

/** Print out the possible arguments
*/
void displayHelp()
{
    std::cout 
        << "Arguments: \n"
        << "-s, --skiplist   Test skiplist \n"
        << "-m, --map        Test std::map \n"
        << "-h, --help       Display this message \n"
        << std::endl;             
}


/** parse the command arguments
@param[in]  argc              The number of arguments passed to main
@param[in]  argv              The arguments passed to main
@param[out] out_testSelection The tests to run. 1 for skiplist, 2 for std::map
@return true if parsing was successful. false if parsing was not successful and out_testSelection is invalid.
*/
bool parseArgs(const int argc, char const* const* const argv, int& out_testSelection)
{
    if (argc < 2)
    {
        std::cout << "Must specify --skiplist or --map\n\n";
        displayHelp();
        return false;
    }
    if (std::char_traits<char>::length(argv[1]) < 40)
    {
        const std::string arg1(argv[1]);
        if (arg1.compare("-s") == 0 || arg1.compare("--skiplist") == 0)
        {
            out_testSelection = 1;
            return true;
        }
        if (arg1.compare("-m") == 0 || arg1.compare("--map") == 0)
        {
            out_testSelection = 2;
            return true;
        }
        if (arg1.compare("-h") == 0 || arg1.compare("--help") == 0)
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
    int testSelection;
    bool parseSuccess;

#if 0  // Force test. Helps when testing in visual studio. Return to 0 before committing!
    testSelection = 1;
    parseSuccess = true;
    // suppress "unused" warning
    (void)argc;
    (void)argv;
#else
    parseSuccess = parseArgs(argc, argv, testSelection);
#endif

    if (parseSuccess)
    {
        // inconsistent measurements if we test more than 1 container in a run
        if (testSelection == 1)
            ProfilingTest1<Skiplist<int, int>>();
        else if (testSelection == 2)
            ProfilingTest1<std::map<int, int>>();
        else
        {
            std::cout << "Invalid test selection" << std::endl;
            return EXIT_FAILURE;
        }
    }

    //std::cin.ignore();
    return EXIT_SUCCESS;
}
