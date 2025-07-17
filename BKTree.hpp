#ifndef BKTREE_HH
#define BKTREE_HH

#include "FileInfo.hpp"
#include <unordered_map>
#include <memory> //For the unique_ptr
#include <vector>
#include <set>
#include <string>

class BKTreeNode{
    public:
    FileInfo m_data;
    
    //The key will be the hamming distance and value the node at that distance.
    std::unordered_map<int, std::unique_ptr<BKTreeNode>> children;

    explicit BKTreeNode(const FileInfo &f)
        :m_data(f)
        {};
};

class BKTree{

    private:
        std::unique_ptr<BKTreeNode> m_root=nullptr;
    public:
        void insert(const FileInfo &f);
        void findSimilar(uint64_t targetHash, int maxDistance,
                        std::vector<FileInfo>& result,
                        std::set<std::filesystem::path>& visited,
                    BKTreeNode *node=nullptr) const;
        //void printSimilarGroups(const std::vector<FileInfo>& fileList, const BKTree &tree, int threshold=10);

};

#endif