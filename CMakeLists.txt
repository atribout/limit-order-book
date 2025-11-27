cmake_minimum_required(VERSION 3.10)
project(LimitOrderBook VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(lob_core INTERFACE)

target_include_directories(lob_core INTERFACE include)

add_executable(LOB_Demo src/main.cpp)
target_link_libraries(LOB_Demo PRIVATE lob_core)