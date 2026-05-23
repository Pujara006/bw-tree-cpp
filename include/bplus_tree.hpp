#pragma once
#include <vector>
#include <memory>
#include <iostream>

class BPlusTree
{
    private:
        struct Node{
            bool isLeaf;
            std::vector<int> keys;
            std::vector<int> values; // only applicable for leaf nodes
            std::vector<std::shared_ptr<Node>> children;
            std::shared_ptr<Node> next;
            Node(bool leaf) : isLeaf(leaf) {}
        };
        std::shared_ptr<Node> root;
        int maxKeys;
        void rootSplit(std::shared_ptr<Node>& root);
    public:
        BPlusTree(int order);
        bool search(int key, int &value) const;
        void insert(int key, int value);
        void printTree() const;
};