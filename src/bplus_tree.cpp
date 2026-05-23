#include "bplus_tree.hpp"

BPlusTree::BPlusTree(int order) : maxKeys(order -1){
    // Tree should be a valid one
    if (order < 3) {
        throw std::invalid_argument("B+ Tree order must be at least 3");
    }
    root = std::make_shared<Node>(true);
}

bool BPlusTree::search(int key,int& value) const{
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
    Node *current = root.get();
    size_t pos = 0;
    while((pos<current->keys.size())&&key>current->keys[pos]) pos++;
    if (pos < current->keys.size() && key == current->keys[pos]) {
        std::cout << "Pair with key " << key << " already exists so doing nothing" << std::endl;
        return;
    }
    current->keys.insert(current->keys.begin() + pos, key);
    current->values.insert(current->values.begin() + pos, value);
    if (current->keys.size() > maxKeys){
        std::cout << "Leaf overflow detected" << std::endl;
        rootSplit(root);
    }
}

void BPlusTree::printTree() const{
    const Node *current = root.get();
    std::cout << "Root Keys: ";
    for (size_t pos = 0; pos < current->keys.size();pos++)
        std::cout << current->keys[pos] << " ";
    std::cout << std::endl;
    if(current->isLeaf == false){
        // for(const std::shared_ptr<Node>& child : current->children){
        for (size_t childPtr = 0; childPtr < current->children.size();childPtr++)
        {
            const auto& child = current->children[childPtr];
            std::cout << "Children "<<childPtr<< " Keys: ";
            for (size_t pos = 0; pos < child->keys.size();pos++)
                std::cout << child->keys[pos] << " ";
            std::cout << std::endl;
        }
    }
}

void BPlusTree::rootSplit(std::shared_ptr<Node>& oldRoot){
    std::shared_ptr<Node> newRoot = std::make_shared<Node>(false);
    size_t splitIndex = (oldRoot->keys.size()) / 2;
    std::shared_ptr<Node> rightLeaf = std::make_shared<Node>(true);
    newRoot->children.push_back(oldRoot);
    newRoot->children.push_back(rightLeaf);
    rightLeaf->keys.assign(oldRoot->keys.begin() + splitIndex, 
                           oldRoot->keys.end());
    rightLeaf->values.assign(oldRoot->values.begin() + splitIndex, 
                           oldRoot->values.end());
    oldRoot->keys.erase(oldRoot->keys.begin() + splitIndex, 
                           oldRoot->keys.end());
    oldRoot->values.erase(oldRoot->values.begin() + splitIndex, 
                           oldRoot->values.end());
    int separatorKey = rightLeaf->keys[0];
    newRoot->keys.push_back(separatorKey);
    rightLeaf->next = oldRoot->next;
    oldRoot->next = rightLeaf;
    oldRoot = newRoot;
}