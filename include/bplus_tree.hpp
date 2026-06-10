#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include <shared_mutex>

class BPlusTree
{
    private:
        struct Node{
            bool isLeaf;
            std::vector<int> keys;
            std::vector<int> values; // only applicable for leaf nodes
            std::vector<std::shared_ptr<Node>> children;
            std::shared_ptr<Node> next;
            mutable std::shared_mutex nodeLock;
            Node(bool leaf) : isLeaf(leaf) {}
        };
        std::shared_ptr<Node> root;
        mutable std::shared_mutex treeLock;
        size_t maxKeys;
        size_t minKeys;
        void splitRootLeaf();
        void splitLeaf(std::vector<Node*>& pathVec,Node* leaf);
        void splitInternal(Node* internalNode, std::vector<Node*>& path);
        std::vector<Node*> findTargetLeaf(int key);
        std::vector<const Node*> findTargetLeaf(int key) const;
        void insertIntoParent(std::vector<Node *> &pathVec,
                                         std::shared_ptr<Node> rightNode, int separatorKey);
        bool validateNode(const Node* node) const;
        bool validateLeafDepth() const;
        bool validateLeafChain() const;
        void handleLeafUnderflow(std::vector<Node *>& pathVec,Node* leaf);
        void borrowFromLeftLeaf(Node* leaf,Node* leftSibling);
        void borrowFromRightLeaf(Node *leaf, Node *rightSibling);
        void mergeWithLeftLeaf(Node *leaf, Node *leftSibling);
        void mergeWithRightLeaf(Node *leaf, Node *rightSibling);
        void handleInternalUnderflow(std::vector<Node *>& pathVec);
        void borrowFromLeftInternal(Node *node, Node *leftSibling,Node* parent,size_t childIndex);
        void borrowFromRightInternal(Node *node, Node *rightSibling,Node* parent,size_t childIndex);
        void mergeWithLeftInternal(Node *node, Node *leftSibling, Node *parent, size_t childIndex);
        void mergeWithRightInternal(Node *node, Node *rightSibling, Node *parent, size_t childIndex);
        void shrinkRoot();
    public:
        BPlusTree(size_t order);
        bool search(int key, int &value) const;
        void insert(int key, int value);
        void printTree() const;
        void printLeaves() const;
        std::vector<std::pair<int, int>> rangeSearch(int startKey, int endKey) const;
        bool validateTree() const;
        bool deleteKey(int key);
};