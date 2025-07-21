#include <algorithm> 
#include <cstring> //used for memcmp
#include <iostream>
#include <unordered_set>

#include "Utility.hpp"

/// Function template
/// Helper: apply callback on equal subranges
/// Go through the list, group files according to a specific criteria, and for each group, run the passed callback function.
template <typename Iterator, typename Comparator, typename Callback>
void apply_on_range(Iterator first, Iterator last, Comparator comp, Callback cb) {
    while (first != last) {
        auto [range_first, range_last] = std::equal_range(first, last, *first, comp);
        cb(range_first, range_last);
        first = range_last;
    }
}

// compares file duration
bool cmpDuration(const FileInfo& a, const FileInfo& b){
  return a.getDuration() < b.getDuration();
}

// compares file size
bool cmpSize(const FileInfo& a, const FileInfo& b){
  return a.getSize() < b.getSize();
}

//Lexicographical comparator on raw binary content.
bool cmpBuffers(const FileInfo& a, const FileInfo& b){
  return std::memcmp(a.getbyteptr(), b.getbyteptr(), a.getBufferSize()) < 0;
}

//Lexicographical comparator on the blake3 hashes of the files.
bool cmpHash(const FileInfo &a, const FileInfo &b){
    return a.getBlake3()<b.getBlake3();
}

//Sort all the functions in the given list according to size.
void Utility::sortFilesBySize(){
    std::sort(m_list.begin(), m_list.end(), cmpSize);
}

//The callback function which is going to be passed to apply_on_range which is going to process every group.
auto handle_size_group = [](auto first, auto last) {
    if(std::distance(first, last) == 1) {
        first->setRemoveUniqueFlag(true);  //Set removeunique flag if the size of the group is only 1.
    } 
};

/// Remove all entries marked for removal
std::size_t Utility::cleanup() {
    const auto old_size = m_list.size();

    //erase takes a pointer and removes all elements from that pointer to the end of the vector.
    m_list.erase(std::remove_if(m_list.begin(), m_list.end(),
        [](const FileInfo& f) { return f.checkRemoveUniqueFlag(); }),
        m_list.end());

    //Return how many elements were removed after cleanup.
    return old_size - m_list.size();
}

/// Remove files with unique sizes
std::size_t Utility::removeUniqueSizes() {
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpSize);

    // Step 2: Apply removal marking logic to the groups.
    apply_on_range(m_list.begin(), m_list.end(), cmpSize, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
    //return 0;
}

//Remove files with unique buffers.
std::size_t Utility::removeUniqueBuffer() {
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpBuffers);

    // Step 2: Apply removal marking logic to the groups.
    apply_on_range(m_list.begin(), m_list.end(), cmpBuffers, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
}

//Remove files with unique hashes.
std::size_t Utility::removeUniqueHashes(){
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpHash);

    // Step 2: Apply removal marking logic to the groups.
    apply_on_range(m_list.begin(), m_list.end(), cmpHash, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
}

std::size_t Utility::removeUniqueDuration(){
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpDuration);

    // Step 2: Apply removal marking logic to the groups.
    apply_on_range(m_list.begin(), m_list.end(), cmpDuration, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
}

std::size_t Utility::removeMarkedFiles(){
    return cleanup();
}