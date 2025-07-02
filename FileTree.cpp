#include <filesystem>
#include <iostream>
#include <string>
#include "FileTree.hpp"

namespace {
/// @brief Maximum allowed depth for recursive directory traversal
constexpr int MAX_RECURSION_DEPTH = 50;
namespace fs = std::filesystem;
}

/**
 * @brief Recursively traverses a directory, reporting regular files and symlinks.
 * 
 * Uses `std::filesystem` to iterate through the directory structure. For each entry,
 * it checks if it's a regular file, symbolic link, or directory. Invokes a callback
 * for symlinks and regular files, and continues traversal into subdirectories.
 * 
 * @param dir The directory path to start traversal from.
 * @param recursionLevel The current depth in the recursive traversal.
 * @return int 
 *         - -1 if maximum recursion depth is exceeded
 *         - 1 if the given path is not a directory
 *         - 2 if the directory was processed successfully
 */
int FileTree::walk(const std::string& dir, int recursionLevel) {
    if (recursionLevel >= MAX_RECURSION_DEPTH) {
        std::cerr << "Error: Maximum recursion depth exceeded\n";
        return -1;
    }

    fs::path dirPath(dir);
    std::error_code ec;

    if (!fs::is_directory(dirPath, ec)) {
        handlePossibleFile(dirPath, recursionLevel);
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(dirPath, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) {
            std::cerr << "Error reading directory: " << ec.message() << '\n';
            continue;
        }

        const auto& path = entry.path();
        const std::string name = path.filename().string();

        bool shouldRecurse = false;

        if (fs::is_symlink(path, ec)) {
            if (m_followsymlinks && m_callback) {
                m_callback(dirPath.string(), name, recursionLevel);
                shouldRecurse = true;
            }
        } else if (fs::is_directory(path, ec)) {
            shouldRecurse = true;
        } else if (fs::is_regular_file(path, ec)) {
            if (m_callback) {
                m_callback(dirPath.string(), name, recursionLevel);
            }
        }

        if (shouldRecurse) {
            walk(path.string(), recursionLevel + 1);
        }
    }

    return 2;
}

/**
 * @brief Handles a file or path that could not be opened as a directory.
 * 
 * If the path is a symlink or regular file, and a callback is set, it is reported.
 * If it is a directory (unexpected), logs an error. Unknown types are ignored.
 * 
 * @param possibleFile The file path that failed to open as a directory.
 * @param recursionLevel The current recursion depth at which this path was encountered.
 * @return int 
 *         - -1 if file info couldn't be retrieved
 *         - -2 if the file is unexpectedly a directory
 *         - 0 if it was a valid symlink or regular file
 */
int FileTree::handlePossibleFile(const fs::path& possibleFile, int recursionLevel) {
    std::error_code ec;

    if (!fs::exists(possibleFile, ec)) {
        return -1;
    }

    const std::string path = possibleFile.parent_path().string() + "/";
    const std::string filename = possibleFile.filename().string();

    if (fs::is_symlink(possibleFile, ec)) {
        if (m_followsymlinks && m_callback) {
            m_callback(path, filename, recursionLevel);
        }
        return 0;
    }

    if (fs::is_directory(possibleFile, ec)) {
        std::cerr << "Dirlist::handlePossibleFile: Unexpected directory.\n"
                  << "File: \"" << possibleFile.string() << "\"\n";
        return -2;
    }

    if (fs::is_regular_file(possibleFile, ec)) {
        if (m_callback) {
            m_callback(path, filename, recursionLevel);
        }
        return 0;
    }

    std::cout << "Dirlist::handlePossibleFile: Unrecognized file type.\n";
    return -1;
}

/**
 * @brief Splits a full file path into directory and filename components.
 * 
 * This function uses `std::filesystem::path` to extract the parent path
 * and filename from a full path string.
 * 
 * @param[out] path The extracted directory path (including trailing '/').
 * @param[out] filename The extracted filename.
 * @param[in] input The full file path to split.
 * @return int 
 *         - 0 on success
 *         - -1 if no parent path was found
 */
int splitFilename(std::string& path, std::string& filename, const std::string& input) {
    fs::path fullPath(input);
    if (fullPath.has_parent_path()) {
        path = fullPath.parent_path().string() + "/";
        filename = fullPath.filename().string();
        return 0;
    } else {
        path.clear();
        filename = input;
        return -1;
    }
}
