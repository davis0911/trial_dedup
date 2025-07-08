#ifndef UTILITY_HH
#define UTILITY_HH

#include <vector>
#include "FileInfo.hpp" // Required for Fileinfo class

/// @brief Class that holds utilities to process and filter files for deduplication.
class Utility{
public:
  /// @brief Constructor that accepts a reference to a file list.
  /// @param list A vector of Fileinfo objects representing scanned files.
  explicit Utility(std::vector<FileInfo>& list)
    : m_list(list) {}

  /**
   * @brief Removes files with unique sizes.
   *
   * Files that do not share their size with any other file are removed.
   * Internally:
   * - Sorts m_list by file size.
   * - Applies a filter: keeps files with duplicate sizes, marks others.
   * - Calls cleanup() to remove marked entries.
   *
   * @return Number of files removed.
   */
  std::size_t removeUniqueSizes();
  std::size_t removeUniqueBuffer();
  std::size_t removeUniqueHashes();

  void sortFilesBySize();

  void setHash();

  void findExactDuplicates();
  
private:
  std::vector<FileInfo>& m_list; ///< The list of files being processed.
  /**
   * @brief Removes all files from m_list that have the delete flag set.
   *
   * This should be called after marking files for deletion.
   * @return Number of items removed.
   */
  std::size_t cleanup();
  //int sortOnDeviceAndInode();
};

#endif // UTILITY_HH
