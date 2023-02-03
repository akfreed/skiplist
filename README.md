# What

This project is a C++11 implementation of the `std::map` and `std::multimap` interfaces with a skiplist back-end. It is portable (tested on MSVC 19.14.x.x,  GCC 5.4.0, and MinGW 5.1.0) and (essentially) unoptimized. This skiplist’s iterators automatically balance the skiplist as they iterate.

See [Q&A: What is a skiplist?](#what-is-a-skiplist)

**NOTE: Bitbucket uses CommonMark, a variant of MarkDown. It appears that internal links are not implemented.**

# Just browsing?

Navigate over to the source directory [_skiplist/_](https://bitbucket.org/akfreed/skiplist/src/5ff6305c6bbb43a5545ff32f95cae0de8233ac0a/skiplist/?at=default) and take a look at [_Skiplist.hpp_](https://bitbucket.org/akfreed/skiplist/src/5ff6305c6bbb43a5545ff32f95cae0de8233ac0a/skiplist/Skiplist.hpp?at=default&fileviewer=file-view-default) and [_SkiplistPair.hpp_](https://bitbucket.org/akfreed/skiplist/src/5ff6305c6bbb43a5545ff32f95cae0de8233ac0a/skiplist/SkiplistPair.hpp?at=default&fileviewer=file-view-default)

# Add to your project

_Skiplist_ is a header-only library. There are many ways to add it into a project.
There are only two required files: [_Skiplist.hpp_](https://bitbucket.org/akfreed/skiplist/src/5ff6305c6bbb43a5545ff32f95cae0de8233ac0a/skiplist/Skiplist.hpp?at=default&fileviewer=file-view-default) and [_SkiplistPair.hpp_](https://bitbucket.org/akfreed/skiplist/src/5ff6305c6bbb43a5545ff32f95cae0de8233ac0a/skiplist/SkiplistPair.hpp?at=default&fileviewer=file-view-default), found in the [_skiplist/_](https://bitbucket.org/akfreed/skiplist/src/5ff6305c6bbb43a5545ff32f95cae0de8233ac0a/skiplist/?at=default) directory.
You could simply include these. You could also use include the [_skiplist/CMakeLists.txt_](https://bitbucket.org/akfreed/skiplist/src/5ff6305c6bbb43a5545ff32f95cae0de8233ac0a/skiplist/CMakeLists.txt?at=default&fileviewer=file-view-default) file in your own CMake script. This script creates a CMake target called `Skiplist` that can be referenced by another target with the `target_link_libraries` function. This will automatically add the source files to the target project.

Using the skiplist is as simple as:
```C++
#include <Skiplist.hpp>

fsl::Skiplist<Key_t, Val_t> s;
```

A `Skiplist` functions almost exactly like a `std::map`. It is a namespace called `fsl`

**Note:** I want people to be able to test and try out this implementation, but keep in mind that I haven't decided how I want to license it yet. See [Q&A: Can I Use Your Skiplist?](#can-i-use-your-skiplist)

# Class structure

5 template classes:

* `Skiplist`
* `SLPairConst`
* `SLPair` (derived from `SLPairConst`)
* `SkiplistNonBalancingIterator`
* `SkiplistBalancingIterator`

All are in a namespace called `fsl`

Each element in the `Skiplist` is an `SLPair`, which contains a key and value. This part of the implementation differs from the standard, but `SLPair` is similar to `std::pair`. `SLPair` is derived from `SLPairConst`. An `SLPairConst` cannot modify the key or value. Thus, it can be used to read values from a `const Skiplist`. An `SLPairConst` from a `Skiplist` can, of course, be up-casted to an `SLPair` to allow modification of a `const Skiplist`. But if someone is willing to do that, they could alternatively use a `const_cast` anyway.

`SkiplistNonBalancingIterator` is the `const_iterator` type and `SkiplistBalancingIterator` is the `iterator` type. They are not related in a hierarchy. Like any other STL iterator, these types act like pointers to `SLPairConst` and `SLPair` instances, respectively.

If you follow good STL style, you can completely avoid calling `SLPairConst`, `SLPair`, `SkiplistNonBalancingIterator`, and `SkiplistBalancingIterator` by name. They are `typedef`-ed  in `Skiplist` as `value_type`, `const_value_type`, `const_iterator`, and `iterator` respectively.


# Q&A

* [What is a skiplist?](#what-is-a-skiplist)
* [What is a simple skiplist’s structure?](#what-is-a-simple-skiplists-structure)
* [What is the structure of the skiplist in this project?](#what-is-the-structure-of-the-skiplist-in-this-project)
* [What is CMake?](#what-is-cmake)
* [Why C++?](#why-c++)
* [What got you interested in skiplists?](#what-got-you-interested-in-skiplists)
* [Why do you have an inconsistent function naming convention?](#why-do-you-have-an-inconsistent-function-naming-convention)
* [What’s up with your unit tests?](#whats-up-with-your-unit-tests)
* [How does your implementation compare against others?](#how-does-your-implementation-compare-against-others)
* [Does your skiplist have exception safety guarantees?](#does-your-skiplist-have-exception-safety-guarantees)
* [What is the complexity analysis of a skiplist?](#what-is-the-complexity-analysis-of-a-skiplist)
* [Have you profiled your skiplist?](#have-you-profiled-your-skiplist)
* [What are you going to add in the future?](#what-are-you-going-to-add-in-the-future)
* [Who are you?](#who-are-you)
* [Can I use your skiplist?](#can-i-use-your-skiplist)

**Note**: This write-up references C++ standard library functions, classes, and lingo. However, most of the concepts have analogues in other languages’ standard libraries.

**Note**: CppReference.com (https://en.cppreference.com/w/) is a great resource. (Just be aware that anyone can make changes to it.)


### What is a skiplist?

A skiplist is a data structure designed to achieve O(log(N)) complexity for insert, find, and erase. It maintains sorted order, allowing in-order traversal. The performance specifications of a skiplist are similar to a red-black tree. Therefore, it can be used as the back-end to ordered associative container interfaces such as map, set, multimap, and multiset.

### What is a simple skiplist’s structure?

A skiplist has layers of linear linked lists. The bottom layer has all the elements. The layer above has references to half the elements from the bottom layer (randomly selected). The next layer above that has references to half of those elements. This continues until the top list only has a few elements.
```
head             →            20
  ↓                            ↓
head             →     17  →  20                    →                   42
  ↓                     ↓      ↓                                         ↓
head  →  10      →     17  →  20         →         33         →         42  →  43
  ↓       ↓             ↓      ↓                    ↓                    ↓      ↓
head  →  10  →  15  →  17  →  20  →  21  →  28  →  33  →  36  →  40  →  42  →  43  →  48
```
_Figure 1: Visualization example of a simple skiplist implementation's possible structure_

From any given node, you can traverse `next` or `down`. `down` has the same key, but is in the level below. If there is no `down`, you are in the lowest list. `next` leads to the next element in order in your current list. If you are in the bottom list, this is the next element in the sequence. If you are not in the bottom list, `next` will likely skip over several elements.

In a balanced skiplist, each upper list has exactly every other element from the list below.
```
head                           →                          36
  ↓                                                        ↓
head             →            20             →            36             →            48
  ↓                            ↓                           ↓                           ↓
head      →     15      →     20      →     28      →     36      →     42      →     48
  ↓              ↓             ↓             ↓             ↓             ↓             ↓
head  →  10  →  15  →  17  →  20  →  21  →  28  →  33  →  36  →  40  →  42  →  43  →  48
```
_Figure 2: Visualization example of a simple skiplist implementation's possible structure when balanced_

In a balanced skiplist, the 2nd from bottom list skips 1 element, the 3rd from bottom skips 3 elements, the 4th skips 7, and so on.

Suppose you want to find 33. Start at the head. 36 is the next element. 36 overshoots 33, so instead of traversing next, we go down a level to level B. The next element is 20, which is less than 33, so we traverse to 20. The next element is 36. 36 overshoots 33, so we go down to level C (the current location is now the 20 on level C). The next element is 28, which is less than 33, so we traverse next. Again, 36 overshoots 33, so go down. The next element is 33, our match.

When we want to iterate through the items of the skiplist, we simply start with the lowest list. The neat thing about a skiplist is that you can balance when you iterate.

### What is the structure of the skiplist in this project?

This project’s skiplist implementation is actually an undirected graph.

```
dummy (head)      ↔            20
  ↕                             ↕
dummy             ↔     17  ↔  20                    ↔                   42
  ↕                      ↕      ↕                                         ↕
dummy  ↔  10      ↔     17  ↔  20         ↔         33         ↔         42  ↔  43
  ↕        ↕             ↕      ↕                    ↕                    ↕      ↕
dummy  ↔  10  ↔  15  ↔  17  ↔  20  ↔  21  ↔  28  ↔  33  ↔  36  ↔  40  ↔  42  ↔  43  ↔  48
```
_Figure 3: Visualization example of this project's skiplist structure_

Each node has a `next`, `previous`, `down`, and `up`. `previous` allows reverse traversal, and `up` allows amortized constant-time balancing when only the bottom-most node is known.

“Columns” are formed by sequences of nodes connected by their `up`/`down` pointers. These nodes all share the same key and value. Some implementations store the columns as an array. In this implementation, nodes in a column are distinct nodes.

There are begin and end pointers to the first and last elements in the bottom list. A count of elements is kept, and this is used to determine how many levels the skiplist should have.

The first element in each list is a dummy node that has no key/value pair. These dummy nodes are connected vertically in a column.

Traversals typically start with the top-most node (marked _head_).

### What is CMake?

CMake is all the rage these days. CMake is a meta-makefile—it makes makefiles. In other words, it creates a makefile that you can use to compile. CMake is cross platform and works with a wide range of compilers (e.g. MSVC, GCC, MinGW), operating systems (e.g. Windows, Linux), and IDEs (e.g. Visual Studio, Code::Blocks, Make). To use it, navigate to the directory with the bottommost _CMakeList.txt_ file. Add a new directory. Go into it and execute `cmake ..` . On Linux, for example, this might create a _Makefile_. You can then type `make` to build the project. On Windows, this might create a Visual Studio solution. You can open the solution and compile.

### Why C++?
I chose C++11 for this project because it's my favorite language and I want to get better at it. It seemed like a good project to practice C++11, learn about STL containers, and figure out how to implement iterators. I have implemented (the early version of) this skiplist in Java and C# as well.

This project still has more learning potential to offer with exception safety and unit testing. See [Q&A: What are you going to add in the future?](#what-are-you-going-to-add-in-the-future)

I chose C++11 over C++14 or C++17 because more compatibility is better. At my job we're on C++11, and I consider _that_ a miracle. PSU's Linux lab computers still max out at C++11. And there were only a few things from C++14 that would have been nice, but it only caused a little extra work to do them in C++11. With that said, I hope to make a C++17 version in the future.

### What got you interested in skiplists?

I’ve been writing this skiplist on and off since 2012. It was for a class; we had to implement a data structure of our choosing. Back then it was in Java. In 2014, I ported it to C++ for use as a portfolio project. In 2016, I did a project analyzing the complexity of the skiplist’s operations for an algorithms class. This year, 2018, I really outdid myself. I wanted to spruce it up for my portfolio again. I started out just wanting to modernized it for C++11 and add some doxygen-style function headers. When I finished with that, I saw an old note where I mentioned that I wanted to add iterators, and I figured now was the time. After a bit of work, I added some STL-compatible iterators that automatically balance as they iterate. The final thing I wanted to add was compatibility with `std::insert_iterator`. That really did me in. It turns out that in order to use `std::insert_iterator`, you have to have the standard hinted-insert function implemented. My functions, until that point, were named `Add`, `Retrieve`, and `Remove` instead of the standard `insert`, `find`, and `erase`. This led me down the rabbit hole of reworking my functions to match the standard signatures. Once you have two standard insert functions, you may as well do all six (C++11).  And once you have those, you may as well complete every find and erase overload too. Eventually, this led me on a classic _If You Give a Mouse a Cookie_ journey until I had finished a full `std::map` and `std::multimap` interface.

I've put a lot of time into this, but it still has a long way to go. I haven't yet found an application for which it outperforms the MSVC or GCC red-black tree implementations. (For more discussion about performance, see the other questions.) I'm ready to move on to something else for now, but I'll inevitably come back to this project again.

### Why do you have an inconsistent function naming convention?

I actually prefer Microsoft coding conventions with a dash of Hungarian.
* public class functions – `PascalCase` (each word capitalized)
* private class functions – `camelCase` (each word capitalized except the first)
* class names – `PascalCase`
* local variables – `camelCase`

Before I decided to do a full std implementation, all the function names followed my convention. When I implemented std, I had to follow the C++ STL naming convention for public class functions:
* public class functions – `under_score` (lowercase, each word separated by an underscore)

But I kept my convention for private class functions. It’s my code, so I can do what I want. If this was industry code, I would have made a different choice.

### What’s up with your unit tests?

Ok, this is probably the most embarrassing thing about my project. I hacked together my own unit-test system instead of using an established framework. I was doing some really rapid development. I just needed to throw some tests together real quick. I didn't expect it to grow to 2,500 LOC. I taught myself a lot of things I didn’t know about C++11 and STL containers. I didn’t want to spend the last 3 months on this project; I thought it would be done in 2. I didn’t want to take the time to teach myself an established unit-testing framework. I promise it’ll be the next thing I do on this project, ok? But right now I have 4 other projects to work on and there’s only 1 month of summer left.

### How does your implementation compare against others?

As far as I can find, no one else has done a full C++11 STL implementation. To be fair, mine isn’t fully STL, but it’s close and and none of the other implementations I saw go nearly as far. I haven’t been able to look too closely at this one yet, but it looks very promising: https://github.com/greensky00/skiplist.

My implementation is about 5x slower than the red-black tree `std::map` implementations from both MSVC and GCC. Remember that mine is, for the most part, unoptimized. Why? Because you can’t optimize until you know what you’re optimizing for. Optimization is at the other end of the spectrum from portability. This implementation should be a good place to start when you want to start optimizing for a particular system and/or particular use.

A common optimization for skiplists is to use an array to store the nodes in a column (see [Q&A: What is the structure of the skiplist in this project?](#what-is-the-structure-of-the-skiplist-in-this-project)). Because the nodes are next to each other in memory, this should optimize for cache hits, theoretically halving the average number of cache misses. But say you wanted to have a concurrent skiplist with multiple readers and a single writer (a common use-case for skiplists). Caching and data locality is the enemy of scalability in the presence of concurrent writers. So it's better to know the application and take some speed measurments before you start trying to optimize.

### Does your skiplist have exception safety guarantees?

No. I’ll use the same excuse here that I used about the unit-tests (See [Q&A: What's up with your unit tests?](#whats-up-with-your-unit-tests)). I’ve already learned a lot and didn’t have the time to teach myself the ins and outs of modern exception safety theory in C++, which is quite a big topic. But it will be the 2nd next thing I do on this project, right after switching to a proper unit-test framework.

### What is the complexity analysis of a skiplist?

I wrote a report about this in 2016. It was a project in my undergrad algorithms class. The conclusion was no surprise. O(log(N)) for insert, find, and erase. O(N) for balance.

I started re-running the tests with the new skiplist version, but it will take a lot of time to work out the results. The tests weren’t that helpful anyway; since they focused on complexity analysis, delays were added to the basic operation to force it to dominate.

By the way, a red-black tree also has O(log(N)) for insert, find, and erase. The guaranteed maximum number of rotations needed to balance a red-black tree is two. The deepest leaf is guaranteed to be no more than twice as deep as the shallowest leaf.

### Have you profiled your skiplist?

Yup. Linear linked list traversals are killing me. The bottleneck, by far, is the comparison of the search key with the next node. Not the comparison itself, but the fact that the function does two dereferences. Linear linked lists are optimized for cache misses. Check out what Bjarne Stroustrup has to say about linear linked lists vs. vectors https://www.youtube.com/watch?v=YQs6IC-vgmo.

It’s also likely that the 50/50 random pattern of choosing to traverse either next or down causes the CPU to suffer branch misprediction effects.

### What are you going to add in the future?

* I’d like to use a bone fide unit-test framework. I’ll try out Google Test and see if I like that. I need to implement exception safety / memory leak safety in the presence of exceptions. These two topics will contribute most to my progress towards becoming an expert C++ programmer.

* One of the first use-case that I dreamed up for Skiplist was as the open-list for an A-star pathfinding algorithm. This is essentially a priority queue. Graph nodes are sorted by how promising they look to a heuristic function. The cheapest node is popped and new nodes are typically close in heuristic value. Theoretically, a skiplist could beat a red-black tree because it doesn’t need to do any rotations when removing or inserting heavily on one side. This use-case is the main reason I’ve kept the `std::multimap` interface, even though it’s a pain in the butt. Preliminary benchmarks have shown that the `std::map` totally destroys my skiplist in this use case, but more testing is needed.

* `SLPair` and `SLPairConst` don’t have any unit tests.

* A big part of this project has been adding Doxygen method headers. I would like to go through and generate the Doxygen documentation.

* I’d like to use something better than `assert` when something goes wrong.

* I want to implement the `std::set` and `std::multiset` interfaces.

* It would be nice to let the user specify their own random number generating function. In this iteration, I switched from `rand()` to `mt19937_64`. That’s an improvement, but Mersenne Twister’s weaknesses have been coming into the limelight lately.

* The C++11 standard quietly changed the specification for the hinted-insert function `std::map::insert(const_iterator hint, const value_type& value)`. In C++11, the hint should point to the element after the one to that will be inserted. Before C++11, the hint should point to the element before the one to be inserted. In Skiplist, this function is implemented to the C++11 spec, but I would like to extend it to also keep the C++03 guarantee.

* The next version will probably be in C++17, though I will probably keep a C++11 compatible version.

### Who are you?

I’m a CS grad student at Portland State University. I take classes part-time. I work full time as a software developer of test-and-measurement instrument software. I enjoy programming in my spare time (as you can see). My favorite topic is concurrent programming.

### Can I use your skiplist?

We should talk about it. I haven’t decided how I’m going to license this yet. For now, it will be on a case-by-case basis. Feel free to test it out and try it in your own programs, though. Suggestions on how to improve it are always welcome.


---
README.md

Language: Markdown. CommonMark 0.28 compatible

This file contains the write-up for Skiplist. It is written for Bitbucket's version of CommonMark, a variant of Markdown. Unfortunately, internal links are not part of CommonMark, and Bitbucket suppresses most HTML, so the internal links on this page are broken.

Copyright &copy; 2018 Alexander Freed. ALL RIGHTS RESERVED.
