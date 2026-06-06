#include "tests.hpp"
#include "bplus_tree.hpp"

#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

static void test_validate_tree_small_tree()
{
    BPlusTree tree(4);
    assert(tree.validateTree());
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    assert(tree.validateTree());
    std::cout << "[PASS] test_validate_tree_small_tree\n";
}

static void test_validate_tree_large_sequential_insert()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++){
        tree.insert(i, i * 10);
    }
    assert(tree.validateTree());
    std::cout << "[PASS] test_validate_tree_large_sequential_insert\n";
}

static void test_validate_tree_large_random_insert()
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
    assert(tree.validateTree());
    std::cout << "[PASS] test_validate_tree_large_random_insert\n";
}

void runValidationTests()
{
    test_validate_tree_small_tree();
    test_validate_tree_large_sequential_insert();
    test_validate_tree_large_random_insert();
}