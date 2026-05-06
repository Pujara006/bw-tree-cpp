#pragma once
#include <vector>
#include <memory>

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
    public:
        BPlusTree(int order);
        bool search(int key, int &value) const;
        void insert(int key, int value);
};