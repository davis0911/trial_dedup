#ifndef FILETREE_HH
#define FILETREE_HH

#include <string>
#include <filesystem>
#include <unordered_set>

/**
 * @class FileTree
 * @brief Recursively traverses a directory and reports files.
 * 
 * This class uses (`std::filesystem`) to perform
 * recursive directory traversal and reports discovered files or symbolic links
 * via a user-defined callback.
 */
class FileTree {
public:

  /**
   * @brief Constructor
   * @param followsymlinks Whether to follow symbolic links during traversal
   */
  explicit FileTree(bool followsymlinks)
      : m_followsymlinks(followsymlinks), 
        m_callback(nullptr) 
        {}

  /**
   * @brief Callback function type for reporting files.
   * 
   * The callback receives:
   * - the full directory path
   */
  using ReportFcnType = int (*)(const std::filesystem::path&);

  /**
   * @brief Set the callback function to be invoked for each discovered file.
   * @param reportFcn Pointer to the callback function
   */
  void setCallback(ReportFcnType reportFcn) { m_callback = reportFcn; }

  /**
   * @brief Recursively walk through a directory tree and report files/symlinks.
   * 
   * @param dir Starting directory path
   * @param recursionLevel Current depth of the recursion (default = 0)
   * @return 
   *          : -1 if canonical path cannot be resolved.  
   *          :  0 incase the directory/file is already visited.
   *          :  1 if the given path is not a directory  
   *          :  2 if the directory was processed successfully  
   */
  int walk(const std::string& dir, int recursionLevel = 0);

private:
  bool m_followsymlinks;      // Whether to follow symbolic links.
  ReportFcnType m_callback;   // Callback to invoke for each discovered file.
  std::unordered_set<std::filesystem::path> visitedDirs;

  /**
   * @brief Handles a file that was expected to be a directory but isn't.
   * 
   * This function checks if the path is a regular file or symlink and reports it.
   * 
   * @param possibleFile File path that could not be opened as a directory
   * @param recursionLevel Current recursion depth
   * @return 
   *         : -1 If any kind of error occurs or the file/symlink doesn't exist.
   *         : 0 if it was a valid symlink or regular file
   */
  int handlePossibleFile(const std::filesystem::path& possibleFile, int recursionLevel);
};

#endif // FILETREE_HH
