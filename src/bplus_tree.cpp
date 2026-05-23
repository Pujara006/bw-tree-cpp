#include "bplus_tree.hpp"

BPlusTree::BPlusTree(int order) : maxKeys(order -1){
    // Tree should be a valid one
    if (order < 3) {
        throw std::invalid_argument("B+ Tree order must be at least 3");
    }
    root = std::make_shared<Node>(true);
}

BPlusTree::Node* BPlusTree::findTargetLeaf(int key){
    Node* current = root.get();
    if(false == current->isLeaf){
        size_t pos = std::lower_bound(current->keys.begin(),
                    current->keys.end(), key) - current->keys.begin();
        if(pos==current->keys.size()||key<current->keys[pos])
            current = current->children[pos].get();
        else{
            current = current->children[pos+1].get();
        }
    }
    return current;
}

const BPlusTree::Node* BPlusTree::findTargetLeaf(int key) const{
    const Node* current = root.get();
    if(false == current->isLeaf){
        size_t pos = std::lower_bound(current->keys.begin(),
                    current->keys.end(), key) - current->keys.begin();
        if(pos==current->keys.size()||key<current->keys[pos])
            current = current->children[pos].get();
        else{
            current = current->children[pos+1].get();
        }
    }
    return current;
}

bool BPlusTree::search(int key,int& value) const{
    const Node *current = findTargetLeaf(key);
    for (size_t i = 0; i < current->keys.size(); i++)
    {
        if(current->keys[i]==key){
            value = current->values[i];
            return true;
        }
    }
    return false;
}

void BPlusTree::insert(int key,int value){
    // current should not be constant here because we will modify the Node
    Node *current = findTargetLeaf(key);
    size_t pos = std::lower_bound(current->keys.begin(),
                    current->keys.end(), key) - current->keys.begin();
    if (pos < current->keys.size() && key == current->keys[pos]) {
        std::cout << "Pair with key " << key << " already exists so doing nothing" << std::endl;
        return;
    }
    current->keys.insert(current->keys.begin() + pos, key);
    current->values.insert(current->values.begin() + pos, value);
    if (current->keys.size() > maxKeys){
        std::cout << "Leaf overflow detected" << std::endl;
        if (root->isLeaf) {
            splitRootLeaf();
        }
    }
}

void BPlusTree::printTree() const {
    std::cout << "Root Keys: ";
    for (int key : root->keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl;

    if (!root->isLeaf) {
        for (size_t i = 0; i < root->children.size(); ++i) {
            std::cout << "Child " << i << " Keys: ";
            for (int k : root->children[i]->keys) std::cout << k << " ";
            std::cout << std::endl;
        }
    }
}

void BPlusTree::splitRootLeaf(){
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