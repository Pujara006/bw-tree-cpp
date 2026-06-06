#include "tests.hpp"
#include "bplus_tree.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>

static void test_delete_existing_key()
{
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

static void test_delete_missing_key()
{
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

static void test_multiple_deletes(){
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

static void test_delete_from_large_tree()
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

static void test_delete_all_keys_without_rebalancing()
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


static void test_borrow_from_left_sibling()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 50; i += 10)
    {
        tree.insert(i, i * 100);
    }
    assert(tree.deleteKey(50));
    int value = 0;
    assert(!tree.search(50, value));
    assert(tree.search(10, value));
    assert(tree.search(20, value));
    assert(tree.search(30, value));
    assert(tree.search(40, value));
    assert(tree.validateTree());
    std::cout << "[PASS] test_borrow_from_left_sibling\n";
}

static void test_borrow_from_right_sibling()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 50; i += 10)
    {
        tree.insert(i, i * 100);
    }
    assert(tree.deleteKey(10));
    int value = 0;
    assert(!tree.search(10, value));
    assert(tree.search(20, value));
    assert(tree.search(30, value));
    assert(tree.search(40, value));
    assert(tree.search(50, value));
    assert(tree.validateTree());
    std::cout << "[PASS] test_borrow_from_right_sibling\n";
}

static void test_multiple_borrows_stress()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 100; i++)
    {
        tree.insert(i, i * 10);
    }
    std::vector<int> deleteKeys = {
        10, 20, 30, 40, 50,
        60, 70, 80, 90
    };
    for (int key : deleteKeys)
    {
        assert(tree.deleteKey(key));
        assert(tree.validateTree());
    }
    int value = 0;
    for (int key : deleteKeys)
    {
        assert(!tree.search(key, value));
    }
    for (int key = 1; key <= 100; key++)
    {
        if (std::find(deleteKeys.begin(), deleteKeys.end(), key) ==
            deleteKeys.end())
        {
            assert(tree.search(key, value));
            assert(value == key * 10);
        }
    }
    assert(tree.validateTree());
    std::cout << "[PASS] test_multiple_borrows_stress\n";
}

void runDeleteTests()
{
    test_delete_existing_key();
    test_delete_missing_key();
    test_multiple_deletes();
    test_delete_from_large_tree();
    test_delete_all_keys_without_rebalancing();
    test_borrow_from_left_sibling();
    test_borrow_from_right_sibling();
    test_multiple_borrows_stress();
}