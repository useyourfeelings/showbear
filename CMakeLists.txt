# Set project name
project (showbear)
message(STATUS cmake-showbear)

# Set minimum version
cmake_minimum_required (VERSION 2.8)

if(MSVC)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if( WIN32 )
    message(STATUS cmake-WIN32)
    add_definitions(-DWIN32)
else()
    message(STATUS cmake-notWIN32)
endif()

add_subdirectory (source)
