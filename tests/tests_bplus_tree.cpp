#include "bplus_tree.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <climits>

// Test 1
void test_insert_and_search_single_key() {
    BPlusTree tree(4);
    tree.insert(10, 100);
    int value = 0;
    bool found = tree.search(10, value);
    assert(found == true);
    assert(value == 100);
    std::cout << "[PASS] test_insert_and_search_single_key\n";
}

// Test 2
void test_search_missing_key(){
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20,200);
    tree.insert(30, 300);
    int value = 0;
    bool found = tree.search(25, value);
    assert(found == false);
    std::cout << "[PASS] test_search_missing_key\n";
}

// Test 3
void test_duplicate_insert(){
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(10, 999);
    int value = 0;
    assert(tree.search(10, value));
    assert(value == 100);
    std::cout << "[PASS] test_duplicate_insert\n";
}

// Test 4
void test_sequential_insert(){
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10)
    {
        tree.insert(i, i * 100);
    }
    int value = 0;
    bool found = tree.search(10, value);
    assert(found == true);
    assert(value == 1000);
    found = tree.search(50, value);
    assert(found == true);
    assert(value == 5000);
    found = tree.search(100, value);
    assert(found == true);
    assert(value == 10000);
    std::cout << "[PASS] test_sequential_insert\n";
}

// Test 5
void test_reverse_insert(){
    BPlusTree tree(4);
    for (int i = 100; i >= 1; i -= 1)
    {
        tree.insert(i, i * 100);
    }
    for (int i = 100; i >= 1; i -= 1)
    {
        int value = 0;
        bool found = tree.search(i, value);
        assert(found == true);
        assert(value == i * 100);
    }
    std::cout << "[PASS] test_reverse_insert\n";
}

// Test 6
void test_random_insert(){
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
        bool found = tree.search(key, value);
        assert(found == true);
        assert(value == key * 100);
    }
    std::cout << "[PASS] test_random_insert\n";
}

// Test 7
void test_range_search_normal(){
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

// Test 8
void test_range_search_full_range(){
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

// Test 9
void test_range_search_single_key(){
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

// Test 10
void test_range_search_empty(){
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10){
        tree.insert(i, i * 100);
    }
    auto result = tree.rangeSearch(45, 45);
    assert(result.empty());
    std::cout << "[PASS] test_range_search_empty\n";
}

// Test 11
void test_range_search_large_tree()
{
    BPlusTree tree(4);
    for(int i = 1; i <= 1000; i++){
        tree.insert(i, i * 10);
    }
    auto result = tree.rangeSearch(INT_MIN, INT_MAX);
    assert(result.size() == 1000);
    for(int i = 1; i <= 1000; i++){
        assert(result[i - 1].first == i);
        assert(result[i - 1].second == i * 10);
    }
    std::cout << "[PASS] test_range_search_large_tree\n";
}

// Test 12
void test_random_insert_large_range_search()
{
    BPlusTree tree(4);
    std::vector<int> keys;
    for (int i = 1; i <= 1000; i++){
        keys.push_back(i);
    }
    std::mt19937 rng(42);   // fixed seed for reproducible tests
    std::shuffle(keys.begin(), keys.end(), rng);
    for (int key : keys){
        tree.insert(key, key * 10);
    }
    auto result = tree.rangeSearch(INT_MIN, INT_MAX);
    assert(result.size() == 1000);
    for (int i = 1; i <= 1000; i++){
        assert(result[i - 1].first == i);
        assert(result[i - 1].second == i * 10);
    }
    std::cout << "[PASS] test_random_insert_large_range_search\n";
}

// Test 13
void test_validate_tree_small_tree()
{
    BPlusTree tree(4);
    assert(tree.validateTree());
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    assert(tree.validateTree());
    std::cout << "[PASS] test_validate_tree_small_tree\n";
}

// Test 14
void test_validate_tree_large_sequential_insert()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++){
        tree.insert(i, i * 10);
    }
    assert(tree.validateTree());
    std::cout << "[PASS] test_validate_tree_large_sequential_insert\n";
}

// Test 15
void test_validate_tree_large_random_insert()
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

// Test 16
void test_negative_keys()
{
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

// Test 17
void test_range_search_single_missing_key()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 100; i += 10){
        tree.insert(i, i * 100);
    }
    auto result = tree.rangeSearch(51, 51);
    assert(result.empty());
    std::cout << "[PASS] test_range_search_single_missing_key\n";
}

// Test 18
void test_search_after_large_random_insert()
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
        bool found = tree.search(i, value);
        assert(found);
        assert(value == i * 10);
    }
    assert(tree.validateTree());
    std::cout << "[PASS] test_search_after_large_random_insert\n";
}

// Test case 19
void test_delete_existing_key(){
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    bool deleted = tree.deleteKey(20);
    assert(deleted);
    int value = 0;
    assert(tree.search(10, value));
    assert(value == 100);
    assert(!tree.search(20, value));
    assert(tree.search(30, value));
    assert(value == 300);
    std::cout << "[PASS] test_delete_existing_key\n";
}

// Test case 20
void test_delete_missing_key(){
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    bool deleted = tree.deleteKey(25);
    assert(!deleted);
    int value = 0;
    assert(tree.search(10, value));
    assert(tree.search(20, value));
    assert(tree.search(30, value));
    std::cout << "[PASS] test_delete_missing_key\n";
}

// Test 21
void test_multiple_deletes(){
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    tree.insert(40, 400);
    assert(tree.deleteKey(20));
    assert(tree.deleteKey(30));
    int value = 0;
    assert(tree.search(10, value));
    assert(value == 100);
    assert(tree.search(40, value));
    assert(value == 400);
    assert(!tree.search(20, value));
    assert(!tree.search(30, value));
    std::cout << "[PASS] test_multiple_deletes\n";
}

// Test 22
void test_delete_from_large_tree()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 100; i++){
        tree.insert(i, i * 10);
    }
    assert(tree.deleteKey(50));
    int value = 0;
    assert(!tree.search(50, value));
    assert(tree.search(49, value));
    assert(value == 490);
    assert(tree.search(51, value));
    assert(value == 510);
    std::cout << "[PASS] test_delete_from_large_tree\n";
}

// Test 23
void test_delete_all_keys_without_rebalancing()
{
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    assert(tree.deleteKey(10));
    assert(tree.deleteKey(20));
    assert(tree.deleteKey(30));
    int value = 0;
    assert(!tree.search(10, value));
    assert(!tree.search(20, value));
    assert(!tree.search(30, value));
    std::cout << "[PASS] test_delete_all_keys_without_rebalancing\n";
}

int main() {
    test_insert_and_search_single_key();
    test_search_missing_key();
    test_duplicate_insert();
    test_sequential_insert();
    test_reverse_insert();
    test_random_insert();
    test_range_search_normal();
    test_range_search_full_range();
    test_range_search_single_key();
    test_range_search_empty();
    test_range_search_large_tree();
    test_random_insert_large_range_search();
    test_validate_tree_small_tree();
    test_validate_tree_large_sequential_insert();
    test_validate_tree_large_random_insert();
    test_negative_keys();
    test_range_search_single_missing_key();
    test_search_after_large_random_insert();
    test_delete_existing_key();
    test_delete_missing_key();
    test_multiple_deletes();
    test_delete_from_large_tree();
    test_delete_all_keys_without_rebalancing();
    return 0;
}