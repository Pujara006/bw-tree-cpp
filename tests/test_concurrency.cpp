// tests/test_concurrency.cpp

#include "tests.hpp"
#include "bplus_tree.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

static void test_concurrent_search()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++){
        tree.insert(i, i * 10);
    }
    const int threadCount = 8;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++){
        threads.emplace_back([&tree]() {
            int value = 0;
            for (int i = 1; i <= 1000; i++){
                assert(tree.search(i, value));
                assert(value == i * 10);
            }
        });
    }
    for (auto &thread : threads){
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
    for (int i = 1; i <= threadCount * keysPerThread; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_insert\n";
}

static void test_concurrent_search_and_insert()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 500; i++){
        tree.insert(i, i * 10);
    }
    std::thread inserter([&tree]() {
        for (int i = 501; i <= 1000; i++){
            tree.insert(i, i * 10);
        }
    });
    std::thread searcher([&tree]() {
        int value = 0;
        for (int i = 1; i <= 500; i++){
            assert(tree.search(i, value));
            assert(value == i * 10);
        }
    });
    inserter.join();
    searcher.join();
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 1000; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_search_and_insert\n";
}

static void test_concurrent_delete()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 1000; i++){
        tree.insert(i, i * 10);
    }
    const int threadCount = 4;
    const int keysPerThread = 250;
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; t++){
        threads.emplace_back([&tree, t]() {
            int start = t * keysPerThread + 1;
            int end = start + keysPerThread - 1;
            for (int key = start; key <= end; key++){
                assert(tree.deleteKey(key));
            }
        });
    }
    for (auto &thread : threads){
        thread.join();
    }
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 1000; i++){
        assert(!tree.search(i, value));
    }
    auto result = tree.rangeSearch(-10000, 10000);
    assert(result.empty());
    std::cout << "[PASS] test_concurrent_delete\n";
}

static void test_concurrent_mixed_workload()
{
    BPlusTree tree(4);
    for (int i = 1; i <= 500; i++){
        tree.insert(i, i * 10);
    }
    std::thread inserter([&tree]() {
        for (int i = 501; i <= 1000; i++){
            tree.insert(i, i * 10);
        }
    });
    std::thread deleter([&tree]() {
        for (int i = 1; i <= 250; i++){
            assert(tree.deleteKey(i));
        }
    });
    std::thread searcher([&tree]() {
        int value = 0;
        for (int i = 251; i <= 500; i++){
            assert(tree.search(i, value));
            assert(value == i * 10);
        }
    });
    inserter.join();
    deleter.join();
    searcher.join();
    assert(tree.validateTree());
    int value = 0;
    for (int i = 1; i <= 250; i++){
        assert(!tree.search(i, value));
    }
    for (int i = 251; i <= 1000; i++){
        assert(tree.search(i, value));
        assert(value == i * 10);
    }
    std::cout << "[PASS] test_concurrent_mixed_workload\n";
}

void runConcurrencyTests()
{
    test_concurrent_search();
    test_concurrent_insert();
    test_concurrent_search_and_insert();
    test_concurrent_delete();
    test_concurrent_mixed_workload();
}