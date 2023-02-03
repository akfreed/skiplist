cmake_minimum_required (VERSION 3.5)

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Skiplist
add_library(Skiplist INTERFACE)
target_include_directories(Skiplist INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_sources(Skiplist INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Skiplist.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SkiplistPair.hpp
)

# Skiplist Debug
add_library(SkiplistDebug INTERFACE)
target_include_directories(SkiplistDebug INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_sources(SkiplistDebug INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/SkiplistDebug.hpp
)
target_link_libraries(SkiplistDebug INTERFACE
    Skiplist
)
