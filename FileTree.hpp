#ifndef FILETREE_HH
#define FILETREE_HH

#include <string>
#include <filesystem>

/**
 * @class FileTree
 * @brief Recursively traverses a directory and reports files.
 * 
 * This class uses modern C++17 features (`std::filesystem`) to perform
 * recursive directory traversal and reports discovered files or symbolic links
 * via a user-defined callback.
 */
class FileTree {
public:
  /**
   * @brief Callback function type for reporting files.
   * 
   * The callback receives:
   * - the parent directory path
   * - the file or symlink name
   * - the current recursion depth
   */
  using ReportFcnType = int (*)(const std::string&, const std::string&, int);

  /**
   * @brief Constructor
   * @param followsymlinks Whether to follow symbolic links during traversal
   */
  explicit FileTree(bool followsymlinks)
      : m_followsymlinks(followsymlinks), m_callback(nullptr) {}

  /**
   * @brief Set the callback function to be invoked for each discovered file or symlink.
   * @param reportFcn Pointer to the callback function
   */
  void setCallback(ReportFcnType reportFcn) { m_callback = reportFcn; }

  /**
   * @brief Recursively walk through a directory tree and report files/symlinks.
   * 
   * @param dir Starting directory path
   * @param recursionLevel Current depth of the recursion (default = 0)
   * @return 
   * - -1 if maximum recursion depth is exceeded  
   * - 1 if the given path is not a directory  
   * - 2 if the directory was processed successfully  
   */
  int walk(const std::string& dir, int recursionLevel = 0);

private:
  bool m_followsymlinks;      ///< Whether to follow symbolic links
  ReportFcnType m_callback;   ///< Callback to invoke for each discovered file/symlink

  /**
   * @brief Handles a file that was expected to be a directory but isn't.
   * 
   * This function checks if the path is a regular file or symlink and reports it.
   * 
   * @param possibleFile File path that could not be opened as a directory
   * @param recursionLevel Current recursion depth
   * @return 
   * - -1 if stat fails or file type is unrecognized  
   * - -2 if the file unexpectedly turns out to be a directory  
   * - 0 if a regular file or symlink was handled correctly  
   */
  int handlePossibleFile(const std::filesystem::path& possibleFile, int recursionLevel);
};

#endif // FILETREE_HH
