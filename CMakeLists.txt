cmake_minimum_required (VERSION 3.5)

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Solution
project(Skiplist CXX)

# Libraries
find_package(Threads)

# Include directories
include_directories(
    tests/
)

# Warning flags
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  if(NOT CMAKE_CXX_FLAGS MATCHES "/WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
endif()


# -------------------------------------------------------------------
# Projects

# Skiplist
add_subdirectory(skiplist/)


# Unit Tests
add_executable(UnitTest
    tests/Movable.h
    tests/UnitMain.cpp
)
target_link_libraries(UnitTest
    Skiplist
    SkiplistDebug
    ${CMAKE_THREAD_LIBS_INIT}
)
# set Visual Studio working directory
set_target_properties(UnitTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")


# Profiling Test
add_executable(ProfilingTest
    tests/Movable.h
    tests/ProfilingMain.cpp
)
target_link_libraries(ProfilingTest
    Skiplist
)
# set Visual Studio working directory
set_target_properties(ProfilingTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")


# Set UnitTest as the start-up project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UnitTest)
