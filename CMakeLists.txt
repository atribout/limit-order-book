cmake_minimum_required(VERSION 3.14)
project(LimitOrderBook VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(lob_core INTERFACE)
target_include_directories(lob_core INTERFACE include)

add_executable(LOB_Demo src/main.cpp)
target_link_libraries(LOB_Demo PRIVATE lob_core)

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lob_tests tests/OrderBookTest.cpp)

target_link_libraries(lob_tests PRIVATE lob_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lob_tests)