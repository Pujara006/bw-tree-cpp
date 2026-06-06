#include "tests.hpp"
#include "bplus_tree.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <climits>

static void test_range_search_normal()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10){
        tree.insert(i, i * 100);
    }
    auto result = tree.rangeSearch(35, 85);
    std::vector<int> expectedKeys = {40, 50, 60, 70, 80};
    assert(result.size() == expectedKeys.size());
    for (size_t i = 0; i < result.size(); i++){
        assert(result[i].first == expectedKeys[i]);
        assert(result[i].second == expectedKeys[i] * 100);
    }
    std::cout << "[PASS] test_range_search_normal\n";
}

static void test_range_search_full_range()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10){
        tree.insert(i, i * 100);
    }
    auto result = tree.rangeSearch(-100, 1000);
    assert(result.size() == 10);
    for (size_t i = 0; i < result.size(); i++){
        int expectedKey = static_cast<int>((i + 1) * 10);

        assert(result[i].first == expectedKey);
        assert(result[i].second == expectedKey * 100);
    }
    std::cout << "[PASS] test_range_search_full_range\n";
}

static void test_range_search_single_key()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10){
        tree.insert(i, i * 100);
    }
    auto result = tree.rangeSearch(50, 50);
    assert(result.size() == 1);
    assert(result[0].first == 50);
    assert(result[0].second == 5000);
    std::cout << "[PASS] test_range_search_single_key\n";
}

static void test_range_search_empty()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10){
        tree.insert(i, i * 100);
    }
    auto result = tree.rangeSearch(45, 45);
    assert(result.empty());
    std::cout << "[PASS] test_range_search_empty\n";
}

static void test_range_search_large_tree()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++){
        tree.insert(i, i * 10);
    }
    auto result = tree.rangeSearch(INT_MIN, INT_MAX);
    assert(result.size() == 1000);
    for (int i = 1; i <= 1000; i++){
        assert(result[i - 1].first == i);
        assert(result[i - 1].second == i * 10);
    }
    std::cout << "[PASS] test_range_search_large_tree\n";
}

static void test_random_insert_large_range_search()
{
    BPlusTree tree(4);
    std::vector<int> keys;
    for (int i = 1; i <= 1000; i++){
        keys.push_back(i);
    }
    std::mt19937 rng(42);
    std::shuffle(keys.begin(), keys.end(), rng);
    for (int key : keys){
        tree.insert(key, key * 10);
    }
    auto result = tree.rangeSearch(INT_MIN, INT_MAX);
    assert(result.size() == 1000);
    for (int i = 1; i <= 1000; i++)
    {
        assert(result[i - 1].first == i);
        assert(result[i - 1].second == i * 10);
    }
    std::cout << "[PASS] test_random_insert_large_range_search\n";
}

static void test_range_search_single_missing_key()
{
    BPlusTree tree(4);

    for (int i = 10; i <= 100; i += 10)
    {
        tree.insert(i, i * 100);
    }

    auto result = tree.rangeSearch(51, 51);

    assert(result.empty());

    std::cout << "[PASS] test_range_search_single_missing_key\n";
}

void runRangeSearchTests()
{
    test_range_search_normal();
    test_range_search_full_range();
    test_range_search_single_key();
    test_range_search_empty();
    test_range_search_large_tree();
    test_random_insert_large_range_search();
    test_range_search_single_missing_key();
}