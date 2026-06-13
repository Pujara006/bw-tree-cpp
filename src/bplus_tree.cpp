#include "bplus_tree.hpp"
#include <climits>

BPlusTree::BPlusTree(size_t order) : maxKeys(order - 1),minKeys((order + 1) / 2 - 1)
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
        auto pos = std::lower_bound(current->keys.begin(),
                                      current->keys.end(), key) -
                     current->keys.begin();
        const size_t posIndex = static_cast<size_t>(pos);
        if (posIndex == current->keys.size() || key < current->keys[posIndex])
            current = current->children[posIndex].get();
        else
        {
            current = current->children[posIndex + 1].get();
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
        auto pos = std::lower_bound(current->keys.begin(),
                                      current->keys.end(), key) -
                     current->keys.begin();
        const size_t posIndex = static_cast<size_t>(pos);
        if (posIndex == current->keys.size() || key < current->keys[posIndex])
            current = current->children[posIndex].get();
        else
        {
            current = current->children[posIndex + 1].get();
        }
    }
    pathVec.push_back(current);
    return pathVec;
}

BPlusTree::TraversalResult BPlusTree::findTargetLeafWithSharedLock(int key) const{
    TraversalResult result;
    if (root == nullptr){
        return result;
    }
    const Node *current = root.get();
    std::shared_lock<std::shared_mutex> lock1(current->nodeLock);
    while(!current->isLeaf){
        auto pos = std::upper_bound(current->keys.begin(),

                            current->keys.end(),

                            key) - current->keys.begin();

        const Node * child = current->children[pos].get();
        std::shared_lock<std::shared_mutex> lock2(child->nodeLock);
        lock1 = std::move(lock2);
        current = child;
    }
    result.leaf = current;
    result.lock = std::move(lock1);
    return result;
}

bool BPlusTree::search(int key, int &value) const
{
    TraversalResult result = findTargetLeafWithSharedLock(key);
    if (result.leaf == nullptr){
        return false;
    }
    const Node *current = result.leaf;
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

BPlusTree::InsertTraversalResult BPlusTree::findTargetLeafForInsert(int key){
    InsertTraversalResult result;
    std::unique_lock<std::shared_mutex> treeLk(treeLock);   // pin the root pointer
    if (root == nullptr) return result;
    Node* current = root.get();
    std::unique_lock<std::shared_mutex> lock1(current->nodeLock);
    // Root won't split (so won't be replaced) iff it has room. Then drop treeLk.
    if (current->keys.size() < maxKeys)
        treeLk.unlock();
    else
        result.treeLk = std::move(treeLk);   // keep until the split finishes
    while (!current->isLeaf) {
        auto pos = std::upper_bound(current->keys.begin(),
                                    current->keys.end(), key)
                   - current->keys.begin();
        Node* child = current->children[pos].get();
        std::unique_lock<std::shared_mutex> lock2(child->nodeLock);
        if (child->keys.size() < maxKeys) {
            // Nothing at/above this child can split -> drop all ancestors,
            // and the root can no longer be replaced -> drop treeLk too.
            result.pathVec.clear();
            result.locks.clear();
            if (result.treeLk.owns_lock()) result.treeLk.unlock();
        } else {
            result.pathVec.push_back(current);
            result.locks.push_back(std::move(lock1));
        }
        lock1 = std::move(lock2);
        current = child;
    }
    result.leaf = current;
    result.locks.push_back(std::move(lock1));
    return result;
}

// Caller (insert) already holds treeLock via result.treeLk, because this is
// only reached when the leaf-root was unsafe. Do NOT lock treeLock again.
// root.get() == leaf is guaranteed here, so reading the global root is safe.
void BPlusTree::splitRootLeaf()
{
    std::shared_ptr<Node> newRoot = std::make_shared<Node>(false);
    size_t splitIndex = root->keys.size() / 2;
    std::shared_ptr<Node> rightLeaf = std::make_shared<Node>(true);
    newRoot->children.push_back(root);
    newRoot->children.push_back(rightLeaf);
    rightLeaf->keys.assign(root->keys.begin() + splitIndex, root->keys.end());
    rightLeaf->values.assign(root->values.begin() + splitIndex, root->values.end());
    root->keys.erase(root->keys.begin() + splitIndex, root->keys.end());
    root->values.erase(root->values.begin() + splitIndex, root->values.end());
    int separatorKey = rightLeaf->keys[0];
    newRoot->keys.push_back(separatorKey);
    rightLeaf->next = root->next;
    root->next = rightLeaf;
    root = newRoot;
}

void BPlusTree::insertIntoParent(
        std::vector<Node*>& pathVec,
        std::vector<std::unique_lock<std::shared_mutex>>& locks,
        std::shared_ptr<Node> rightNode,int separatorKey){
    if (!pathVec.empty()) {
        auto parent = pathVec.back();
        auto pos = std::lower_bound(parent->keys.begin(),
                                    parent->keys.end(), separatorKey)
                   - parent->keys.begin();
        parent->keys.insert(parent->keys.begin() + pos, separatorKey);
        parent->children.insert(parent->children.begin() + pos + 1, rightNode);
    } else {
        // Root replacement. Reached only when the old root was unsafe, so the
        // calling thread already holds treeLock via result.treeLk — do NOT relock.
        std::shared_ptr<Node> newRoot = std::make_shared<Node>(false);
        newRoot->keys.push_back(separatorKey);
        newRoot->children.push_back(root);
        newRoot->children.push_back(rightNode);
        root = newRoot;
        // Lock the new root before anyone can traverse it.
        std::unique_lock<std::shared_mutex> newRootLock(root->nodeLock);
        pathVec.push_back(root.get());
        locks.push_back(std::move(newRootLock));
    }
}

void BPlusTree::splitLeaf(
    std::vector<Node*>& pathVec,
    std::vector<std::unique_lock<std::shared_mutex>>& locks,Node* leaf){
    size_t splitIndex = leaf->keys.size() / 2;
    std::shared_ptr<Node> rightLeaf = std::make_shared<Node>(true);
    rightLeaf->keys.assign(leaf->keys.begin() + splitIndex, leaf->keys.end());
    rightLeaf->values.assign(leaf->values.begin() + splitIndex, leaf->values.end());
    leaf->keys.erase(leaf->keys.begin() + splitIndex, leaf->keys.end());
    leaf->values.erase(leaf->values.begin() + splitIndex, leaf->values.end());
    int separatorKey = rightLeaf->keys[0];
    rightLeaf->next = leaf->next;
    leaf->next = rightLeaf;
    insertIntoParent(pathVec, locks, rightLeaf, separatorKey);
    Node* parent = pathVec.back();
    pathVec.pop_back();
    if (!locks.empty()) locks.pop_back();  // releases the leaf lock; leaf is done
    if (parent->keys.size() > maxKeys)
        splitInternal(parent, pathVec, locks);
}

void BPlusTree::splitInternal(
    Node* internalNode,
    std::vector<Node*>& pathVec,
    std::vector<std::unique_lock<std::shared_mutex>>& locks){
    while (internalNode->keys.size() > maxKeys) {
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

        insertIntoParent(pathVec, locks, rightNode, separatorKey);

        if (pathVec.empty()) break;

        internalNode = pathVec.back();
        pathVec.pop_back();
        if (!locks.empty()) locks.pop_back();
    }
}

void BPlusTree::insert(int key, int value)
{
    InsertTraversalResult result = findTargetLeafForInsert(key);
    Node* current = result.leaf;
    if (current == nullptr) return;   // empty tree: handle root creation separately

    auto pos = std::lower_bound(current->keys.begin(),
                                current->keys.end(), key)
               - current->keys.begin();
    const size_t posIndex = static_cast<size_t>(pos);

    if (posIndex < current->keys.size() && key == current->keys[posIndex]) {
        std::cout << "Pair with key " << key << " already exists\n";
        return;
    }

    current->keys.insert(current->keys.begin() + pos, key);
    current->values.insert(current->values.begin() + pos, value);

    if (current->keys.size() > maxKeys) {
        if (result.pathVec.empty()) {
            // No parent was retained -> the overflowing leaf IS the root.
            splitRootLeaf();
        } else {
            splitLeaf(result.pathVec, result.locks, current);
        }
    }
}

void BPlusTree::printTree() const{
    std::shared_lock<std::shared_mutex> lock(treeLock);
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


void BPlusTree::printLeaves() const{
    std::shared_lock<std::shared_mutex> lock(treeLock);
    if(root==nullptr){
        return;
    }
    const Node* current = root.get();
    while(!current->isLeaf){
        if (current->children.empty()){
            return;
        }
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
    std::shared_lock<std::shared_mutex> lock(treeLock);
    std::vector<std::pair<int, int>> keyValues;
    std::vector<const Node*> pathVec = findTargetLeaf(startKey);
    if(pathVec.empty()){
        return {};
    }
    const Node *current = pathVec.back();
    while(current){
        if(!current->keys.empty() && current->keys[0]>endKey)
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
    std::shared_lock<std::shared_mutex> lock(treeLock);
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

void BPlusTree::shrinkRoot(){
    if(root->isLeaf)
        return;
    if(root->keys.empty() &&root->children.size() == 1){
        root = root->children[0];
    }
}

void BPlusTree::borrowFromLeftLeaf(Node* leaf,Node* leftSibling){
    int key = leftSibling->keys.back();
    int value = leftSibling->values.back();
    leaf->keys.insert(leaf->keys.begin(), key);
    leaf->values.insert(leaf->values.begin(), value);
    leftSibling->keys.pop_back();
    leftSibling->values.pop_back();
}

void BPlusTree::borrowFromRightLeaf(Node* leaf,Node* rightSibling){
    int key = rightSibling->keys[0];
    int value = rightSibling->values[0];
    leaf->keys.insert(leaf->keys.end(), key);
    leaf->values.insert(leaf->values.end(), value);
    rightSibling->keys.erase(rightSibling->keys.begin());
    rightSibling->values.erase(rightSibling->values.begin());
}

void BPlusTree::mergeWithLeftLeaf(Node *leaf, Node* leftSibling){
    leftSibling->keys.insert(leftSibling->keys.end(),leaf->keys.begin(), leaf->keys.end());
    leftSibling->values.insert(leftSibling->values.end(),leaf->values.begin(), leaf->values.end());
    leftSibling->next = leaf->next;
}

void BPlusTree::mergeWithRightLeaf(Node* leaf, Node* rightSibling){
    leaf->keys.insert(leaf->keys.end(),rightSibling->keys.begin(), rightSibling->keys.end());
    leaf->values.insert(leaf->values.end(),rightSibling->values.begin(), rightSibling->values.end());
    leaf->next = rightSibling->next;
}

void BPlusTree::borrowFromLeftInternal(Node *node, Node *leftSibling,
                                       Node* parent,size_t childIndex){
    size_t separatorIndex = childIndex - 1;
    node->keys.insert(node->keys.begin(), parent->keys[separatorIndex]);
    node->children.insert(node->children.begin(), 
                        leftSibling->children.back());
    parent->keys[separatorIndex] = leftSibling->keys.back();
    leftSibling->keys.pop_back();
    leftSibling->children.pop_back();
}

void BPlusTree::borrowFromRightInternal(Node *node, Node *rightSibling,
                                        Node* parent,size_t childIndex){
    // Move parent separator down into node.
    // Promote rightSibling's first key to parent.
    node->keys.push_back(parent->keys[childIndex]);
    node->children.push_back(rightSibling->children[0]);
    parent->keys[childIndex] = rightSibling->keys[0];
    rightSibling->keys.erase(rightSibling->keys.begin());
    rightSibling->children.erase(rightSibling->children.begin());
}

void BPlusTree::mergeWithLeftInternal(Node *node, Node *leftSibling, 
                                    Node *parent, size_t childIndex){
    size_t separatorIndex = childIndex - 1;
    leftSibling->keys.push_back(parent->keys[separatorIndex]);
    parent->keys.erase(parent->keys.begin()+separatorIndex);
    leftSibling->keys.insert(leftSibling->keys.end(),node->keys.begin(),node->keys.end());
    leftSibling->children.insert(leftSibling->children.end(), node->children.begin(), node->children.end());
    parent->children.erase(parent->children.begin()+childIndex);
}

void BPlusTree::mergeWithRightInternal(Node *node, Node *rightSibling, Node *parent, size_t childIndex){
    node->keys.push_back(parent->keys[childIndex]);
    node->keys.insert(node->keys.end(), rightSibling->keys.begin(), rightSibling->keys.end());
    node->children.insert(node->children.end(), rightSibling->children.begin(), rightSibling->children.end());
    parent->keys.erase(parent->keys.begin() + childIndex);
    parent->children.erase(parent->children.begin() + childIndex + 1);
}

void BPlusTree::handleInternalUnderflow(std::vector<Node*>& pathVec){
    if (pathVec.size() <= 1)
    {
        return;
    }
    Node *node = pathVec.back();
    pathVec.pop_back();
    Node *parent = pathVec.back();
    size_t childIndex = 0;
    for (size_t i = 0; i < parent->children.size();i++){
        if(parent->children[i].get() == node){
            childIndex = i;
            break;
        }
    }
    Node *leftSibling = childIndex > 0 ? parent->children[childIndex - 1].get() : nullptr;
    Node* rightSibling = childIndex < parent->children.size()-1 ? parent->children[childIndex+1].get() : nullptr;
    if(leftSibling && leftSibling->keys.size()>minKeys){
        borrowFromLeftInternal(node, leftSibling, parent, childIndex);
    }
    else if(rightSibling && rightSibling->keys.size()>minKeys){
        borrowFromRightInternal(node, rightSibling, parent, childIndex);
    }
    else{
        if(leftSibling){
            mergeWithLeftInternal(node, leftSibling, parent, childIndex);
        }
        else if(rightSibling){
            mergeWithRightInternal(node,rightSibling,parent,childIndex);
        }
        if(parent->keys.size()<minKeys){
            handleInternalUnderflow(pathVec);
        }
    }
}

void BPlusTree::handleLeafUnderflow(std::vector<Node*>& pathVec,Node* leaf){
    if(pathVec.empty()){
        return;
    }
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
        borrowFromLeftLeaf(leaf, leftSibling);
        parent->keys[childIndex - 1] = leaf->keys[0];
    }
    else if(rightSibling && rightSibling->keys.size()>minKeys){
        borrowFromRightLeaf(leaf, rightSibling);
        parent->keys[childIndex] = rightSibling->keys[0];
    }
    else{
        if(leftSibling){
            mergeWithLeftLeaf(leaf, leftSibling);
            parent->children.erase(parent->children.begin() + childIndex);
            parent->keys.erase(parent->keys.begin() + childIndex-1);
        }
        else{
            mergeWithRightLeaf(leaf, rightSibling);
            parent->children.erase(parent->children.begin() + childIndex+1);
            parent->keys.erase(parent->keys.begin() + childIndex);
        }
        if(parent->keys.size()<minKeys){
            handleInternalUnderflow(pathVec);
        }
    }
}

bool BPlusTree::deleteKey(int key){
    std::unique_lock<std::shared_mutex> lock(treeLock);
    std::vector<Node*> pathVec = findTargetLeaf(key);
    if (pathVec.empty()) return false;
    Node *current = pathVec.back();
    pathVec.pop_back();
    auto pos = std::lower_bound(current->keys.begin(),
                                  current->keys.end(), key) -
                 current->keys.begin();
    const size_t posIndex = static_cast<size_t>(pos);
    if(posIndex >=current->keys.size())
        return false;
    if(current->keys[posIndex]!= key)
        return false;
    current->keys.erase(current->keys.begin() + pos);
    current->values.erase(current->values.begin() + pos);
    if(current == root.get()){
        return true;
    }
    if(current->keys.size()<minKeys){
        handleLeafUnderflow(pathVec, current);
    }
    shrinkRoot();
    return true;
}