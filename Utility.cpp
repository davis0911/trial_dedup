#include <algorithm> 
#include <cassert>    // assert
#include "Utility.hpp"
#include "FileInfo.hpp"
#include "Checksum.hpp"

#include <cstring>
/// Function template
/// Helper: apply callback on equal subranges
/// Go through the list, group files that have the same size, and for each group, run this callback.
template <typename Iterator, typename Comparator, typename Callback>
void apply_on_range(Iterator first, Iterator last, Comparator comp, Callback cb) {
    while (first != last) {
        auto [range_first, range_last] = std::equal_range(first, last, *first, comp);
        cb(range_first, range_last);
        first = range_last;
    }
}

// compares file size
bool cmpSize(const FileInfo& a, const FileInfo& b){
  return a.size() < b.size();
}

/// Remove files with unique sizes (only one file with that size)
std::size_t Utility::removeUniqueSizes() {
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpSize);

    // Step 2: Apply removal logic to size groups
    //The lambda function which is going to be passed and which is going to process every group.
    auto handle_size_group = [](auto first, auto last) {
        if(std::distance(first, last) == 1) {
            first->set_remove_unique_flag(true);  // Unique size remove
        } 
    };
    apply_on_range(m_list.begin(), m_list.end(), cmpSize, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
    //return 0;
}

/// Remove all entries marked for removal
std::size_t Utility::cleanup() {
    const auto old_size = m_list.size();

    m_list.erase(std::remove_if(m_list.begin(), m_list.end(),
        [](const FileInfo& f) { return f.remove_unique_flag(); }),
        m_list.end());

    return old_size - m_list.size();
}

// bool cmpDeviceInode(const FileInfo& a, const FileInfo& b){
//   return std::make_tuple(a.device(), a.inode()) <
//          std::make_tuple(b.device(), b.inode());
// }


// int Utility::sortOnDeviceAndInode(){
//   std::sort(m_list.begin(), m_list.end(), cmpDeviceInode);
//   return 0;
// }

// bool hasEqualBuffers(const FileInfo& a, const FileInfo& b){
//   return std::memcmp(a.getbyteptr(), b.getbyteptr(), a.getBufferSize()) == 0;
// }

//lexicographical comparator on raw binary content.
bool cmpBuffers(const FileInfo& a, const FileInfo& b){
  return std::memcmp(a.getbyteptr(), b.getbyteptr(), a.getBufferSize()) < 0;
}

std::size_t Utility::removeUniqueBuffer() {
    std::sort(m_list.begin(), m_list.end(), cmpBuffers);

    auto handle_size_group = [](auto first, auto last) {
        if(std::distance(first, last) == 1) {
            first->set_remove_unique_flag(true);  // Unique size remove
        } 
    };

    apply_on_range(m_list.begin(), m_list.end(), cmpBuffers, handle_size_group);
    return cleanup();
}

void Utility::sortFilesBySize(){
    std::sort(m_list.begin(), m_list.end(), cmpSize);
}

Checksum obj;
void Utility::setHash(){
    for(auto &it: m_list){
        it.setBlake3(obj.compute(it.path().string()));
    }
}

bool cmpHash(const FileInfo &a, const FileInfo &b){
    return a.getblake3()<b.getblake3();
}
std::size_t Utility::removeUniqueHashes(){
    std::sort(m_list.begin(), m_list.end(), cmpHash);
    auto handle_size_group = [](auto first, auto last) {
        if(std::distance(first, last) == 1) {
            first->set_remove_unique_flag(true);  // Unique size remove
        } 
    };
    apply_on_range(m_list.begin(), m_list.end(), cmpHash, handle_size_group);
    return cleanup();
}