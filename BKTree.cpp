#include "BKTree.hpp"
#include "FileInfo.hpp"

#include <set>
#include <iostream>

int hammingDistance(uint64_t a, uint64_t b){
    return __builtin_popcountll(a^b);
}


void BKTree::insert(const FileInfo &f){
    if(!m_root){
        m_root=std::make_unique<BKTreeNode>(f);
        return;
    }
    //You are going to insert based on the position of the root.
    //And what content is present in the root node.
    BKTreeNode *current=m_root.get();
    
    int dist=hammingDistance(current->m_data.getImgHash(), f.getImgHash());
    while(current->children.count(dist)){
        //Ownership still belongs to the unique_ptr
        //current is just an observer.
        current=current->children[dist].get();
        dist=hammingDistance(f.getImgHash(), current->m_data.getImgHash());
    }
    current->children[dist]=std::make_unique<BKTreeNode>(f);

}

void BKTree::findSimilar(uint64_t targetHash, int maxDistance, 
    std::vector<FileInfo>& result, 
    std::set<std::filesystem::path> &visited, 
    BKTreeNode* node) const{
    if(!node){
        node=m_root.get();
    }
    if(!node){
        std::cout<<"Tree is empty. No files to filter"<<"\n";
        return ;
    }
    int dist=hammingDistance(targetHash, node->m_data.getImgHash());
    if(dist<=maxDistance && visited.find(node->m_data.getPath())==visited.end()){
        //std::cout<<dist<<"\n";
        result.push_back(node->m_data);
    }
    for(int i=std::max(0, dist-maxDistance); i<=dist+maxDistance; ++i){
        auto it=node->children.find(i);
        if(it!=node->children.end()){
            findSimilar(targetHash, maxDistance, result, visited, it->second.get());
        }
    }

}