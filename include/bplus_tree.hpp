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
        void splitRootLeaf();
        void splitLeaf(std::vector<Node*>& pathVec,Node* leaf);
        void splitInternal(Node* internalNode, std::vector<Node*>& path);
        std::vector<Node*> findTargetLeaf(int key);
        std::vector<const Node*> findTargetLeaf(int key) const;
        void insertIntoParent(std::vector<Node *> &pathVec,
                                         std::shared_ptr<Node> rightNode, int separatorKey);
    public:
        BPlusTree(int order);
        bool search(int key, int &value) const;
        void insert(int key, int value);
        void printTree() const;
        void printLeaves() const;
        std::vector<std::pair<int, int>> rangeSearch(int startKey, int endKey) const;
};