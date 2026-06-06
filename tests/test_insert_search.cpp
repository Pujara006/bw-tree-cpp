#include "tests.hpp"
#include "bplus_tree.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

static void test_insert_and_search_single_key()
{
    BPlusTree tree(4);
    tree.insert(10, 100);
    int value = 0;
    assert(tree.search(10, value));
    assert(value == 100);
    std::cout << "[PASS] test_insert_and_search_single_key\n";
}

static void test_search_missing_key()
{
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    int value = 0;
    assert(!tree.search(25, value));
    std::cout << "[PASS] test_search_missing_key\n";
}

static void test_duplicate_insert()
{
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(10, 999);
    int value = 0;
    assert(tree.search(10, value));
    assert(value == 100);
    std::cout << "[PASS] test_duplicate_insert\n";
}

static void test_sequential_insert()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10){
        tree.insert(i, i * 100);
    }
    int value = 0;
    assert(tree.search(10, value));
    assert(value == 1000);
    assert(tree.search(50, value));
    assert(value == 5000);
    assert(tree.search(100, value));
    assert(value == 10000);
    std::cout << "[PASS] test_sequential_insert\n";
}

static void test_reverse_insert()
{
    BPlusTree tree(4);
    for (int i = 100; i >= 1; i--){
        tree.insert(i, i * 100);
    }
    for (int i = 100; i >= 1; i--){
        int value = 0;
        assert(tree.search(i, value));
        assert(value == i * 100);
    }
    std::cout << "[PASS] test_reverse_insert\n";
}

static void test_random_insert()
{
    BPlusTree tree(4);
    std::vector<int> keys = {
        50, 10, 90, 30, 70,
        20, 40, 60, 80, 100
    };
    for (int key : keys){
        tree.insert(key, key * 100);
    }
    for (int key : keys){
        int value = 0;
        assert(tree.search(key, value));
        assert(value == key * 100);
    }
    std::cout << "[PASS] test_random_insert\n";
}

static void test_negative_keys(){
    BPlusTree tree(4);
    tree.insert(-100, 100);
    tree.insert(-50, 200);
    tree.insert(0, 300);
    tree.insert(50, 400);
    int value = 0;
    assert(tree.search(-100, value));
    assert(value == 100);
    assert(tree.search(-50, value));
    assert(value == 200);
    assert(tree.search(0, value));
    assert(value == 300);
    assert(tree.search(50, value));
    assert(value == 400);
    assert(tree.validateTree());
    std::cout << "[PASS] test_negative_keys\n";
}

static void test_search_after_large_random_insert()
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
    for (int i = 1; i <= 1000; i++){
        int value = 0;
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    assert(tree.validateTree());
    std::cout << "[PASS] test_search_after_large_random_insert\n";
}

void runInsertSearchTests()
{
    test_insert_and_search_single_key();
    test_search_missing_key();
    test_duplicate_insert();
    test_sequential_insert();
    test_reverse_insert();
    test_random_insert();
    test_negative_keys();
    test_search_after_large_random_insert();
}