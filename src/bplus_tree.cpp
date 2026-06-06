#include "bplus_tree.hpp"
#include <climits>

BPlusTree::BPlusTree(int order) : maxKeys(order - 1),minKeys((order + 1) / 2 - 1)
{
    // Tree should be a valid one
    if (order < 3)
    {
        throw std::invalid_argument("B+ Tree order must be at least 3");
    }
    root = std::make_shared<Node>(true);
}

std::vector<BPlusTree::Node*> BPlusTree::findTargetLeaf(int key){
    if(root == nullptr)
        return {};
    Node* current = root.get();
    std::vector<BPlusTree::Node *> pathVec;
    while (!current->isLeaf)
    {
        pathVec.push_back(current);
        size_t pos = std::lower_bound(current->keys.begin(),
                                      current->keys.end(), key) -
                     current->keys.begin();
        if (pos == current->keys.size() || key < current->keys[pos])
            current = current->children[pos].get();
        else
        {
            current = current->children[pos + 1].get();
        }
    }
    pathVec.push_back(current);
    return pathVec;
}

std::vector<const BPlusTree::Node*> BPlusTree::findTargetLeaf(int key) const{
    if(root == nullptr)
        return {};
    const Node *current = root.get();
    std::vector<const BPlusTree::Node *> pathVec;
    while (!current->isLeaf)
    {
        pathVec.push_back(current);
        size_t pos = std::lower_bound(current->keys.begin(),
                                      current->keys.end(), key) -
                     current->keys.begin();
        if (pos == current->keys.size() || key < current->keys[pos])
            current = current->children[pos].get();
        else
        {
            current = current->children[pos + 1].get();
        }
    }
    pathVec.push_back(current);
    return pathVec;
}

bool BPlusTree::search(int key, int &value) const
{
    std::vector<const Node*> pathVec = findTargetLeaf(key);
    const Node *current = pathVec.back();
    for (size_t i = 0; i < current->keys.size(); i++)
    {
        if (current->keys[i] == key)
        {
            value = current->values[i];
            return true;
        }
    }
    return false;
}

void BPlusTree::insert(int key, int value)
{
    // current should not be constant here because we will modify the Node
    std::vector<Node*> pathVec = findTargetLeaf(key);
    Node *current = pathVec.back();
    pathVec.pop_back();
    size_t pos = std::lower_bound(current->keys.begin(),
                                  current->keys.end(), key) -
                 current->keys.begin();
    if (pos < current->keys.size() && key == current->keys[pos])
    {
        std::cout << "Pair with key " << key << " already exists so doing nothing" << std::endl;
        return;
    }
    current->keys.insert(current->keys.begin() + pos, key);
    current->values.insert(current->values.begin() + pos, value);
    if (current->keys.size() > maxKeys)
    {
        if (root->isLeaf)
        {
            splitRootLeaf();
        }
        else if (current->isLeaf)
        {
            splitLeaf(pathVec,current);
        }
    }
}

void BPlusTree::printTree() const
{
    if(root==nullptr){
        return;
    }
    int level = 0;
    std::queue<const Node*> traversalQueue;
    traversalQueue.push(root.get());
    while(!traversalQueue.empty()){
        size_t queueSize = traversalQueue.size();
        std::cout << "Level " << level << ": ";
        while (queueSize > 0)
        {
            const Node* current = traversalQueue.front();
            traversalQueue.pop();
            std::cout << "[ ";
            for (size_t i = 0; i < current->keys.size();i++){
                std::cout << current->keys[i] << " ";
            }
            std::cout << "] ";
            for (size_t i = 0; i < current->children.size();i++){
                traversalQueue.push(current->children[i].get());
            }
            queueSize--;
        }
        std::cout << std::endl;
        level++;
    }
}

void BPlusTree::splitRootLeaf()
{
    std::shared_ptr<Node> newRoot = std::make_shared<Node>(false);
    size_t splitIndex = (root->keys.size()) / 2;
    std::shared_ptr<Node> rightLeaf = std::make_shared<Node>(true);
    newRoot->children.push_back(root);
    newRoot->children.push_back(rightLeaf);
    rightLeaf->keys.assign(root->keys.begin() + splitIndex,
                           root->keys.end());
    rightLeaf->values.assign(root->values.begin() + splitIndex,
                             root->values.end());
    root->keys.erase(root->keys.begin() + splitIndex,
                     root->keys.end());
    root->values.erase(root->values.begin() + splitIndex,
                       root->values.end());
    int separatorKey = rightLeaf->keys[0];
    newRoot->keys.push_back(separatorKey);
    rightLeaf->next = root->next;
    root->next = rightLeaf;
    root = newRoot;
}

void BPlusTree::insertIntoParent(std::vector<Node*>& pathVec,std::shared_ptr<Node> rightNode,int separatorKey){
    if(pathVec.size()>0){
        auto parent = pathVec.back();
        size_t pos = std::lower_bound(parent->keys.begin(), parent->keys.end(), separatorKey) -
                     parent->keys.begin();
        parent->keys.insert(parent->keys.begin() + pos, separatorKey);
        parent->children.insert(parent->children.begin() + pos + 1, rightNode);
    }
    else{
        std::shared_ptr<Node> parent = std::make_shared<Node>(false);
        parent->keys.push_back(separatorKey);
        parent->children.push_back(root);
        parent->children.push_back(rightNode);
        root = parent;
        pathVec.push_back(root.get());
    }
}

void BPlusTree::splitLeaf(std::vector<Node*>& pathVec,Node *leaf)
{
    size_t splitIndex = (leaf->keys.size()) / 2;
    std::shared_ptr<Node> rightLeaf = std::make_shared<Node>(true);
    rightLeaf->keys.assign(leaf->keys.begin() + splitIndex,
                           leaf->keys.end());
    rightLeaf->values.assign(leaf->values.begin() + splitIndex,
                             leaf->values.end());
    leaf->keys.erase(leaf->keys.begin() + splitIndex,
                     leaf->keys.end());
    leaf->values.erase(leaf->values.begin() + splitIndex,
                       leaf->values.end());
    int separatorKey = rightLeaf->keys[0];
    insertIntoParent(pathVec, rightLeaf, separatorKey);
    rightLeaf->next = leaf->next;
    leaf->next = rightLeaf;
    auto parent = pathVec.back();
    pathVec.pop_back();
    if (parent->keys.size() > maxKeys)
        splitInternal(parent,pathVec);
}

void BPlusTree::splitInternal(Node* internalNode, std::vector<Node*>& pathVec){
    while(internalNode->keys.size()>maxKeys)
    {
        std::shared_ptr<Node> rightNode = std::make_shared<Node>(false);
        size_t splitIndex = internalNode->keys.size() / 2;
        rightNode->keys.assign(internalNode->keys.begin() + splitIndex + 1,
                            internalNode->keys.end());
        rightNode->children.assign(internalNode->children.begin() + splitIndex + 1,
                                internalNode->children.end());
        int separatorKey = internalNode->keys[splitIndex];
        internalNode->keys.erase(internalNode->keys.begin() + splitIndex,
                                 internalNode->keys.end());
        internalNode->children.erase(internalNode->children.begin() + splitIndex + 1, 
                            internalNode->children.end());
        insertIntoParent(pathVec,rightNode, separatorKey);
        internalNode = pathVec.back();
        pathVec.pop_back();
    }
}

void BPlusTree::printLeaves() const{
    const Node* current = root.get();
    while(!current->isLeaf){
        current = current->children[0].get();
    }
    while(current){
        std::cout << "[ ";
        for (auto key : current->keys)
            std::cout << key << " ";
        std::cout << "]";
        if (current->next)
        {
            std::cout << " -> ";
        }
        current = current->next.get();
    }
    std::cout << std::endl;
}

std::vector<std::pair<int,int>> BPlusTree::rangeSearch(int startKey,int endKey) const{
    std::vector<std::pair<int, int>> keyValues;
    std::vector<const Node*> pathVec = findTargetLeaf(startKey);
    const Node *current = pathVec.back();
    while(current){
        if(current->keys[0]>endKey)
            break;
        for (size_t i = 0; i < current->keys.size(); i++)
        {
            if(current->keys[i]>=startKey && current->keys[i]<=endKey){
                keyValues.push_back({current->keys[i], current->values[i]});
            }
            else if(current->keys[i]>endKey)
                break;
        }
        current = current->next.get();
    }
    return keyValues;
}

bool BPlusTree::validateTree() const
{
    return BPlusTree::validateNode(root.get()) &&
           BPlusTree::validateLeafDepth() &&
           BPlusTree::validateLeafChain();
}

bool BPlusTree::validateNode(const Node* node) const
{
    if (node == nullptr){
        return true;
    }
    std::queue<const Node*> traversalQueue;
    traversalQueue.push(node);
    while (!traversalQueue.empty()){
        const Node* current = traversalQueue.front();
        traversalQueue.pop();
        for (size_t i = 1; i < current->keys.size(); i++){
            if (current->keys[i] < current->keys[i - 1])
            {
                return false;
            }
        }
        if (current->keys.size() > static_cast<size_t>(maxKeys)){
            return false;
        }
        if (current->isLeaf){
            if (current->values.size() != current->keys.size()){
                return false;
            }
        }
        else{
            if (current->children.size() != current->keys.size() + 1){
                return false;
            }
            for (const auto& child : current->children){
                if (child == nullptr)
                {
                    return false;
                }
                traversalQueue.push(child.get());
            }
        }
    }
    return true;
}

bool BPlusTree::validateLeafDepth() const{
    int leafLevel = -1;
    int level = 0;
    if (nullptr == root)
        return true;
    std::queue<const Node*> traversalQueue;
    traversalQueue.push(root.get());
    while(!traversalQueue.empty()){
        size_t queueSize = traversalQueue.size();
        while(0<queueSize){
            const Node *current = traversalQueue.front();
            traversalQueue.pop();
            if(current->isLeaf){
                if (leafLevel == -1) {
                    leafLevel = level;
                } else if (leafLevel != level) {
                    return false;
                }
            }
            for (size_t i = 0; i < current->children.size();i++){
                traversalQueue.push(current->children[i].get());
            }
            queueSize--;
        }
        level++;
    }
    return true;
}

bool BPlusTree::validateLeafChain() const{
    if (root == nullptr){
        return true;
    }
    const Node* current = root.get();
    while (!current->isLeaf){
        current = current->children[0].get();
    }
    int previousKey = INT_MIN;
    while (current){
        for (size_t i = 0; i < current->keys.size(); i++){
            if (current->keys[i] <= previousKey){
                return false;
            }
            previousKey = current->keys[i];
        }
        current = current->next.get();
    }
    return true;
}

void BPlusTree::borrowFromLeftSibling(Node* leaf,Node* leftSibling){
    int key = leftSibling->keys[leftSibling->keys.size() - 1];
    int value = leftSibling->values[leftSibling->values.size() - 1];
    leaf->keys.insert(leaf->keys.end(), key);
    leaf->values.insert(leaf->values.end(), value);
    leftSibling->keys.erase(leftSibling->keys.end() - 1);
    leftSibling->values.erase(leftSibling->values.end() - 1);
}

void BPlusTree::borrowFromRightSibling(Node* leaf,Node* rightSibling){
    int key = rightSibling->keys[0];
    int value = rightSibling->values[0];
    leaf->keys.insert(leaf->keys.begin(), key);
    leaf->values.insert(leaf->values.begin(), value);
    rightSibling->keys.erase(rightSibling->keys.begin());
    rightSibling->values.erase(rightSibling->values.begin());
}

void BPlusTree::handleLeafUnderflow(std::vector<Node*> pathVec,Node* leaf){
    Node *parent = pathVec.back();
    size_t childIndex =0;
    for (size_t i = 0; i < parent->children.size();i++){
        if(parent->children[i].get() == leaf){
            childIndex = i;
            break;
        }
    }
    Node *leftSibling = childIndex > 0 ? parent->children[childIndex - 1].get() : nullptr;
    Node* rightSibling = childIndex < parent->children.size()-1 ? parent->children[childIndex+1].get() : nullptr;
    if(leftSibling && leftSibling->keys.size()>minKeys){
        borrowFromLeftSibling(leaf, leftSibling);
        parent->keys[childIndex - 1] = leaf->keys[0];
    }
    else if(rightSibling && rightSibling->keys.size()>minKeys){
        borrowFromRightSibling(leaf, rightSibling);
        parent->keys[childIndex] = rightSibling->keys[0];
    }
    else{
        std::cout << "Merge Needed" << std::endl;
    }
}

bool BPlusTree::deleteKey(int key){
    std::vector<Node*> pathVec = findTargetLeaf(key);
    if (pathVec.empty()) return false;
    Node *current = pathVec.back();
    pathVec.pop_back();
    size_t pos = std::lower_bound(current->keys.begin(),
                                  current->keys.end(), key) -
                 current->keys.begin();
    if(pos>=current->keys.size())
        return false;
    if(current->keys[pos]!= key)
        return false;
    current->keys.erase(current->keys.begin() + pos);
    current->values.erase(current->values.begin() + pos);
    if(current == root.get()){
        std::cout << "root underflow \n";
        return true;
    }
    if(current->keys.size()<minKeys){
        handleLeafUnderflow(pathVec, current);
    }
    return true;
}