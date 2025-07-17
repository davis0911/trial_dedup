#ifndef UTILITY_HH
#define UTILITY_HH

#include <vector>
#include "FileInfo.hpp"

/**
 * @class Utility
 * @brief Provides functions to filter and process files during deduplication.
 * 
 * This class holds a reference to a list of FileInfo objects and offers
 * utility functions to sort, analyze, and remove files that are unique
 * in size, content, or hash — which are not considered duplicates.
 */
class Utility {
public:
    /**
     * @brief Constructs a Utility object with a reference to a file list.
     * 
     * @param list A vector of FileInfo objects representing scanned files.
     */
    explicit Utility(std::vector<FileInfo>& list)
        : m_list(list) {}

    /**
     * @brief Removes files that have unique file sizes.
     * 
     * Files that do not share their size with any other file are not considered
     * duplicates and are removed from the list.
     * 
     * Internally:
     * - Sorts the file list by size.
     * - Identifies and marks files with unique sizes.
     * - Calls cleanup() to remove them.
     * 
     * @return The number of files removed.
     */
    std::size_t removeUniqueSizes();

    /**
     * @brief Removes files that have unique binary buffers.
     * 
     * After sorting by buffer content files that do not share
     * their buffer content with any other file are removed.
     * 
     * @return The number of files removed.
     */
    std::size_t removeUniqueBuffer();

    /**
     * @brief Removes files with unique hash values.
     *
     * Files whose hashes are not shared with any other are removed.
     * 
     * @return The number of files removed.
     */
    std::size_t removeUniqueHashes();

    /**
     * @brief Sorts the list of files by their size in ascending order.
     * 
     */
    void sortFilesBySize();
    
    static void findExactDuplicates(char* filename);
    
    static void similarImages(char* filename);

private:
    std::vector<FileInfo>& m_list;

    /**
     * @brief Removes all files marked for deletion from the file list.
     * 
     * This function physically removes files marked for removal from the list.
     * 
     * @return The number of files removed.
     */
    std::size_t cleanup();
};

#endif // UTILITY_HH
