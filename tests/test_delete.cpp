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

static void test_merge_with_left_sibling()
{
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    tree.insert(40, 400);
    assert(tree.deleteKey(40));
    int value = 0;
    assert(!tree.search(40, value));
    assert(tree.search(10, value));
    assert(tree.search(20, value));
    assert(tree.search(30, value));
    std::cout << "[PASS] test_merge_with_left_sibling\n";
}

static void test_merge_with_right_sibling()
{
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    tree.insert(40, 400);
    assert(tree.deleteKey(10));
    int value = 0;
    assert(!tree.search(10, value));
    assert(tree.search(20, value));
    assert(tree.search(30, value));
    assert(tree.search(40, value));
    std::cout << "[PASS] test_merge_with_right_sibling\n";
}

static void test_leaf_merge_range_search()
{
    BPlusTree tree(4);
    for (int i = 10; i <= 60; i += 10){
        tree.insert(i, i * 100);
    }
    assert(tree.deleteKey(60));
    auto result = tree.rangeSearch(-100, 1000);
    std::vector<int> expectedKeys = {10, 20, 30, 40, 50};
    assert(result.size() == expectedKeys.size());
    for (size_t i = 0; i < result.size(); i++){
        assert(result[i].first == expectedKeys[i]);
        assert(result[i].second == expectedKeys[i] * 100);
    }
    std::cout << "[PASS] test_leaf_merge_range_search\n";
}

static void test_internal_borrow_from_left(){
    BPlusTree tree(4);
    for (int i = 1; i <= 50; i++){
        tree.insert(i, i * 10);
    }
    for (int i = 40; i <= 50; i++){
        assert(tree.deleteKey(i));
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 39; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    for (int i = 40; i <= 50; i++){
        assert(!tree.search(i, value));
    }
    std::cout << "[PASS] test_internal_borrow_from_left\n";
}

static void test_internal_borrow_from_right(){
    BPlusTree tree(4);
    for (int i = 1; i <= 50; i++){
        tree.insert(i, i * 10);
    }
    for (int i = 1; i <= 10; i++)
    {
        assert(tree.deleteKey(i));
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 11; i <= 50; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    for (int i = 1; i <= 10; i++){
        assert(!tree.search(i, value));
    }
    std::cout << "[PASS] test_internal_borrow_from_right\n";
}

static void test_internal_merge(){
    BPlusTree tree(4);
    for (int i = 1; i <= 100; i++){
        tree.insert(i, i * 10);
    }
    for (int i = 1; i <= 70; i++){
        assert(tree.deleteKey(i));
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 71; i <= 100; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    for (int i = 1; i <= 70; i++){
        assert(!tree.search(i, value));
    }
    std::cout << "[PASS] test_internal_merge\n";
}

static void test_multilevel_delete_propagation(){
    BPlusTree tree(4);
    for (int i = 1; i <= 200; i++){
        tree.insert(i, i * 10);
    }
    for (int i = 1; i <= 180; i++){
        assert(tree.deleteKey(i));
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 181; i <= 200; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    for (int i = 1; i <= 180; i++){
        assert(!tree.search(i, value));
    }
    std::cout << "[PASS] test_multilevel_delete_propagation\n";
}

static void test_large_delete_stress(){
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++){
        tree.insert(i, i * 10);
    }
    for (int i = 1; i <= 900; i++){
        assert(tree.deleteKey(i));
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 901; i <= 1000; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    for (int i = 1; i <= 900; i++){
        assert(!tree.search(i, value));
    }
    std::cout << "[PASS] test_large_delete_stress\n";
}

static void test_root_shrink_single_level()
{
    BPlusTree tree(4);

    for (int i = 1; i <= 20; i++)
    {
        tree.insert(i, i * 10);
    }

    for (int i = 1; i <= 19; i++)
    {
        assert(tree.deleteKey(i));
    }

    assert(tree.validateTree());

    int value = 0;
    assert(tree.search(20, value));
    assert(value == 200);

    for (int i = 1; i <= 19; i++)
    {
        assert(!tree.search(i, value));
    }

    std::cout << "[PASS] test_root_shrink_single_level\n";
}

static void test_root_shrink_to_leaf()
{
    BPlusTree tree(4);

    for (int i = 1; i <= 10; i++)
    {
        tree.insert(i, i * 10);
    }

    for (int i = 1; i <= 9; i++)
    {
        assert(tree.deleteKey(i));
    }

    assert(tree.validateTree());

    int value = 0;
    assert(tree.search(10, value));
    assert(value == 100);

    std::cout << "[PASS] test_root_shrink_to_leaf\n";
}

static void test_root_shrink_multilevel()
{
    BPlusTree tree(4);

    for (int i = 1; i <= 200; i++)
    {
        tree.insert(i, i * 10);
    }

    for (int i = 1; i <= 195; i++)
    {
        assert(tree.deleteKey(i));
    }

    assert(tree.validateTree());

    int value = 0;

    for (int i = 196; i <= 200; i++)
    {
        assert(tree.search(i, value));
        assert(value == i * 10);
    }

    for (int i = 1; i <= 195; i++)
    {
        assert(!tree.search(i, value));
    }

    std::cout << "[PASS] test_root_shrink_multilevel\n";
}

static void test_root_shrink_large_stress()
{
    BPlusTree tree(4);

    for (int i = 1; i <= 1000; i++)
    {
        tree.insert(i, i * 10);
    }

    for (int i = 1; i <= 995; i++)
    {
        assert(tree.deleteKey(i));
    }

    assert(tree.validateTree());

    int value = 0;

    for (int i = 996; i <= 1000; i++)
    {
        assert(tree.search(i, value));
        assert(value == i * 10);
    }

    std::cout << "[PASS] test_root_shrink_large_stress\n";
}

static void test_delete_everything()
{
    BPlusTree tree(4);

    for(int i = 1; i <= 100; i++)
    {
        tree.insert(i, i * 10);
    }

    for(int i = 1; i <= 100; i++)
    {
        assert(tree.deleteKey(i));
    }

    assert(tree.validateTree());

    int value;
    for(int i = 1; i <= 100; i++)
    {
        assert(!tree.search(i, value));
    }

    std::cout << "[PASS] test_delete_everything\n";
}

static void test_delete_everything_and_validate_empty_tree()
{
    BPlusTree tree(4);

    for (int i = 1; i <= 1000; i++)
    {
        tree.insert(i, i * 10);
    }

    for (int i = 1; i <= 1000; i++)

{

    assert(tree.deleteKey(i));

    if (i % 100 == 0)

    {

        assert(tree.validateTree());

    }

}

    assert(tree.validateTree());

    int value = 0;

    for (int i = 1; i <= 1000; i++)
    {
        assert(!tree.search(i, value));
    }

    auto result = tree.rangeSearch(-10000, 10000);

    assert(result.empty());

    std::cout
        << "[PASS] test_delete_everything_and_validate_empty_tree\n";
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
    test_merge_with_left_sibling();
    test_merge_with_right_sibling();
    test_leaf_merge_range_search();
    test_internal_borrow_from_left();
    test_internal_borrow_from_right();
    test_internal_merge();
    test_multilevel_delete_propagation();
    test_large_delete_stress();
    test_root_shrink_single_level();
    test_root_shrink_to_leaf();
    test_root_shrink_multilevel();
    test_root_shrink_large_stress();
    test_delete_everything();
    test_delete_everything_and_validate_empty_tree();
}