cmake_minimum_required(VERSION 3.16)
project(BwTreeCpp VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

add_library(
    bplus_tree
    src/bplus_tree.cpp
)

target_include_directories(
    bplus_tree
    PUBLIC
    include
)

add_executable(
    bplus_tree_tests
    tests/test_main.cpp
    tests/test_insert_search.cpp
    tests/test_range_search.cpp
    tests/test_validation.cpp
    tests/test_delete.cpp
    tests/test_concurrency.cpp
)

target_link_libraries(
    bplus_tree_tests
    PRIVATE
    bplus_tree
)

if(MSVC)
    target_compile_options(bplus_tree PRIVATE /W4)
    target_compile_options(bplus_tree_tests PRIVATE /W4)
else()
    target_compile_options(bplus_tree PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(bplus_tree_tests PRIVATE -Wall -Wextra -Wpedantic)

    if(ENABLE_ASAN)
        target_compile_options(bplus_tree PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(bplus_tree PRIVATE -fsanitize=address)

        target_compile_options(bplus_tree_tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(bplus_tree_tests PRIVATE -fsanitize=address)
    endif()
endif()