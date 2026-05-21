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
    for(size_t i=0;i<current->keys.size();i++){
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
    }
}

void BPlusTree::printTree() const{
    const Node *current = root.get();
    std::cout << "Keys: ";
    for (size_t pos = 0; pos < current->keys.size();pos++)
        std::cout << current->keys[pos] << " ";
    std::cout << std::endl;
}