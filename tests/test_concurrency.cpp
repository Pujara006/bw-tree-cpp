// tests/test_concurrency.cpp
#include "tests.hpp"
#include "bplus_tree.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <set>
#include <mutex>
#include <atomic>
#include <chrono>

static void test_concurrent_search()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++)
    {
        tree.insert(i, i * 10);
    }
    const int threadCount = 8;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++)
    {
        threads.emplace_back([&tree]()
                             {
            int value = 0;
            for (int i = 1; i <= 1000; i++){
                assert(tree.search(i, value));
                assert(value == i * 10);
            } });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    assert(tree.validateTree());
    std::cout << "[PASS] test_concurrent_search\n";
}

static void test_concurrent_insert()
{
    BPlusTree tree(4);
    const int threadCount = 4;
    const int keysPerThread = 250;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++)
    {
        threads.emplace_back([&tree, t]()
                             {
            int start = t * keysPerThread + 1;
            int end = start + keysPerThread - 1;
            for (int key = start; key <= end; key++){
                tree.insert(key, key * 10);
            } });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= threadCount * keysPerThread; i++)
    {
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_insert\n";
}

static void test_concurrent_search_and_insert()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 500; i++)
    {
        tree.insert(i, i * 10);
    }
    std::thread inserter([&tree]()
                         {
        for (int i = 501; i <= 1000; i++){
            tree.insert(i, i * 10);
        } });
    std::thread searcher([&tree]()
                         {
        int value = 0;
        for (int i = 1; i <= 500; i++){
            assert(tree.search(i, value));
            assert(value == i * 10);
        } });
    inserter.join();
    searcher.join();
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 1000; i++)
    {
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_search_and_insert\n";
}

static void test_concurrent_delete()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++)
    {
        tree.insert(i, i * 10);
    }
    const int threadCount = 4;
    const int keysPerThread = 250;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++)
    {
        threads.emplace_back([&tree, t]()
                             {
            int start = t * keysPerThread + 1;
            int end = start + keysPerThread - 1;
            for (int key = start; key <= end; key++){
                assert(tree.deleteKey(key));
            } });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 1000; i++)
    {
        assert(!tree.search(i, value));
    }
    auto result = tree.rangeSearch(-10000, 10000);
    assert(result.empty());
    std::cout << "[PASS] test_concurrent_delete\n";
}

static void test_concurrent_mixed_workload()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 500; i++)
    {
        tree.insert(i, i * 10);
    }
    std::thread inserter([&tree]()
                         {
        for (int i = 501; i <= 1000; i++){
            tree.insert(i, i * 10);
        } });
    std::thread deleter([&tree]()
                        {
        for (int i = 1; i <= 250; i++){
            assert(tree.deleteKey(i));
        } });
    std::thread searcher([&tree]()
                         {
        int value = 0;
        for (int i = 251; i <= 500; i++){
            assert(tree.search(i, value));
            assert(value == i * 10);
        } });
    inserter.join();
    deleter.join();
    searcher.join();
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 250; i++)
    {
        assert(!tree.search(i, value));
    }
    for (int i = 251; i <= 1000; i++)
    {
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_mixed_workload\n";
}

static void test_many_concurrent_search_threads()
{
    BPlusTree tree(4);

    for (int i = 1; i <= 1000; i++)
    {
        tree.insert(i, i * 10);
    }

    constexpr int threadCount = 32;

    std::vector<std::thread> threads;

    for (int t = 0; t < threadCount; t++)
    {
        threads.emplace_back([&tree]()
                             {
            int value = 0;

            for (int round = 0; round < 100; round++)
            {
                for (int i = 1; i <= 1000; i++)
                {
                    assert(tree.search(i, value));
                    assert(value == i * 10);
                }
            } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    assert(tree.validateTree());

    std::cout << "[PASS] test_many_concurrent_search_threads\n";
}

static void test_concurrent_search_during_insert()
{
    BPlusTree tree(4);

    for (int i = 1; i <= 1000; i++)
    {
        tree.insert(i, i * 10);
    }

    std::thread inserter([&tree]()
                         {
        for (int i = 1001; i <= 5000; i++)
        {
            tree.insert(i, i * 10);
        } });

    constexpr int searchThreadCount = 8;

    std::vector<std::thread> searchThreads;

    for (int t = 0; t < searchThreadCount; t++)
    {
        searchThreads.emplace_back([&tree]()
                                   {
            int value = 0;

            for (int round = 0; round < 50; round++)
            {
                for (int i = 1; i <= 1000; i++)
                {
                    assert(tree.search(i, value));
                    assert(value == i * 10);
                }
            } });
    }

    inserter.join();

    for (auto &thread : searchThreads)
    {
        thread.join();
    }

    assert(tree.validateTree());

    int value = 0;

    for (int i = 1; i <= 5000; i++)
    {
        assert(tree.search(i, value));
        assert(value == i * 10);
    }

    std::cout << "[PASS] test_concurrent_search_during_insert\n";
}

static void test_concurrent_non_split_duplicate_insert()
{
    BPlusTree tree(5000);
    constexpr int threadCount = 8;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++)
    {
        threads.emplace_back([&tree]()
                             {
                                 for (int key = 1; key <= 200; key++)
                                 {
                                     tree.insert(key, key * 10);
                                 }
                             });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int key = 1; key <= 200; key++)
    {
        assert(tree.search(key, value));
        assert(value == key * 10);
    }
    std::cout << "[PASS] test_concurrent_non_split_duplicate_insert\n";
}

static void test_search_during_non_split_insert()
{
    BPlusTree tree(5000);
    for (int i = 1; i <= 1000; i++)
    {
        tree.insert(i, i * 10);
    }
    std::thread inserter([&tree]()
                         {
        for (int i = 1001; i <= 2000; i++){
            tree.insert(i, i * 10);
        } });
    std::thread searcher([&tree]()
                         {
        int value = 0;
        for (int round = 0; round < 100; round++){
            for (int i = 1; i <= 1000; i++){
                assert(tree.search(i, value));
                assert(value == i * 10);
            }
        } });
    inserter.join();
    searcher.join();
    assert(tree.validateTree());
    std::cout << "[PASS] test_search_during_non_split_insert\n";
}

static void test_root_leaf_split(){
    BPlusTree tree(4);
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    // next insert should trigger root leaf split
    tree.insert(40, 400);
    assert(tree.validateTree());
    int value = 0;
    for (int key : {10, 20, 30, 40}){
        assert(tree.search(key, value));
        assert(value == key * 10);
    }
    std::cout << "[PASS] test_root_leaf_split\n";
}

static void test_concurrent_leaf_split_only()
{
    BPlusTree tree(4);
    std::vector<std::thread> threads;
    threads.emplace_back([&tree]() {
        for (int i = 1; i <= 10; i++){
            tree.insert(i, i * 10);
        }
    });
    threads.emplace_back([&tree]() {
        for (int i = 11; i <= 20; i++){
            tree.insert(i, i * 10);
        }
    });
    for (auto &thread : threads){
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 20; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    auto result = tree.rangeSearch(1, 20);
    assert(result.size() == 20);
    std::cout << "[PASS] test_concurrent_leaf_split_only\n";
}

static void test_concurrent_non_split_insert(){
    BPlusTree tree(5000);
    constexpr int threadCount = 8;
    constexpr int keysPerThread = 100;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++){
        threads.emplace_back([&tree, t]() {
            int start = t * keysPerThread + 1;
            int end = start + keysPerThread - 1;
            for (int key = start; key <= end; key++){
                tree.insert(key, key * 10);
            }
        });
    }
    for (auto &thread : threads){
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int key = 1; key <= threadCount * keysPerThread; key++){
        assert(tree.search(key, value));
        assert(value == key * 10);
    }
    std::cout << "[PASS] test_concurrent_non_split_insert\n";
}

static void test_insert_split_fallback_leaf_split()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 50; i++){
        tree.insert(i, i * 10);
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 50; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_insert_split_fallback_leaf_split\n";
}

static void test_concurrent_insert_split_fallback()
{
    BPlusTree tree(4);
    constexpr int threadCount = 4;
    constexpr int keysPerThread = 100;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++){
        threads.emplace_back([&tree, t]() {
            int start = t * keysPerThread + 1;
            int end = start + keysPerThread - 1;

            for (int key = start; key <= end; key++)
            {
                tree.insert(key, key * 10);
            }
        });
    }
    for (auto &thread : threads){
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= threadCount * keysPerThread; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_insert_split_fallback\n";
}

static void test_concurrent_insert_search_split_fallback()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 100; i++){
        tree.insert(i, i * 10);
    }
    std::thread inserter([&tree]() {
        for (int i = 101; i <= 500; i++){
            tree.insert(i, i * 10);
        }
    });
    std::thread searcher([&tree]() {
        int value = 0;
        for (int round = 0; round < 50; round++){
            for (int i = 1; i <= 100; i++)
            {
                assert(tree.search(i, value));
                assert(value == i * 10);
            }
        }
    });
    inserter.join();
    searcher.join();
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 500; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_insert_search_split_fallback\n";
}

static void test_concurrent_random_insert_stress()
{
    BPlusTree tree(4);
    constexpr int threadCount = 32;
    constexpr int insertsPerThread = 5000;
    constexpr int keyRange = 50000;
    std::vector<std::thread> threads;
    std::set<int> expectedKeys;
    std::mutex expectedMutex;
    for (int t = 0; t < threadCount; t++){
        threads.emplace_back([&tree, &expectedKeys, &expectedMutex, t]() {
            std::mt19937 rng(1000 + t);
            std::uniform_int_distribution<int> dist(1, keyRange);
            for (int i = 0; i < insertsPerThread; i++){
                int key = dist(rng);
                tree.insert(key, key * 10);
                std::lock_guard<std::mutex> guard(expectedMutex);
                expectedKeys.insert(key);
            }
        });
    }
    for (auto &thread : threads){
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int key : expectedKeys){
        assert(tree.search(key, value));
        assert(value == key * 10);
    }
    std::cout << "[PASS] test_concurrent_random_insert_stress\n";
}

void runConcurrencyTests()
{
    test_concurrent_search();
    test_concurrent_insert();
    test_concurrent_search_and_insert();
    test_concurrent_delete();
    test_concurrent_mixed_workload();
    test_many_concurrent_search_threads();
    test_concurrent_search_during_insert();
    test_concurrent_non_split_insert();
    test_concurrent_non_split_duplicate_insert();
    test_search_during_non_split_insert();
    test_root_leaf_split();
    test_concurrent_leaf_split_only();
    test_insert_split_fallback_leaf_split();
    test_concurrent_insert_split_fallback();
    test_concurrent_insert_search_split_fallback();
    test_concurrent_random_insert_stress();
}