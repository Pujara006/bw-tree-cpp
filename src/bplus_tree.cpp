#include "bplus_tree.hpp"

BPlusTree::BPlusTree(int order) : maxKeys(order -1){
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