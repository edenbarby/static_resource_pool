cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(PROJECT_LIB static_resource_pool)
set(PROJECT_TEST static_resource_pool_test)

project(${PROJECT_LIB})

add_executable(
    ${PROJECT_TEST}
    test/Main.cpp
)

target_include_directories(
    ${PROJECT_TEST}
    PRIVATE
    src
    inc
)

target_link_libraries(
    ${PROJECT_TEST}
    GTest::gtest_main
)

add_subdirectory(googletest)
include(GoogleTest)
enable_testing()
gtest_discover_tests(${PROJECT_TEST})