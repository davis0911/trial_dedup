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
            std::cerr << "Error reading directory '" << dirPath << "': " << ec.message() << '\n';
            continue;
        }

        //entry is of type std::filesystem::directory_entry.
        //This is an object that represents a directory entry (file, dir, symlink, etc.).
        // But entry is not a path itself — it's an object that contains a path along with cached metadata (like file type, permissions, etc.).
        const auto& path = entry.path();
        const std::string name = path.filename().string();

        std::error_code status_ec;
        fs::file_status stat = entry.symlink_status(status_ec);
        if (status_ec) {
            std::cerr << "Error: Cannot get file status for " << path << ": " << status_ec.message() << "\n";
            continue;
        }

        if (fs::is_symlink(stat)) {
            if (m_followsymlinks && m_callback) {
                m_callback(dirPath.string(), name, recursionLevel);
                if (fs::is_directory(path, ec)) {
                    walk(path.string(), recursionLevel + 1);
                }
            }
        } else if (fs::is_directory(stat)) {
            walk(path.string(), recursionLevel + 1);
        } else if (fs::is_regular_file(stat)) {
            if (m_callback) {
                m_callback(dirPath.string(), name, recursionLevel);
            }
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

    //File doesn't exist in the first place. Wrong name passed.
    if (!fs::exists(possibleFile, ec)) {
        return -1;
    }

    const std::string path = possibleFile.parent_path().string() + "/";
    const std::string filename = possibleFile.filename().string();

    fs::file_status stat = fs::symlink_status(possibleFile, ec);
    if (ec) {
        std::cerr << "Error getting status for file: " << possibleFile << ": " << ec.message() << "\n";
        return -1;
    }

    if (fs::is_symlink(stat)) {
        if (m_followsymlinks && m_callback) {
            //m_callback(path, filename, recursionLevel);
            if (fs::is_directory(possibleFile, ec)) {
                walk(possibleFile.string(), recursionLevel + 1);
            }
        }
        return 0;
    }

    /**
     * This is just a safety net because there could be a case
     * where file states change. Paul Dreik included this part in
     * his code. Would be interesting to see if this every happens.
     */
    if (fs::is_directory(stat)) {
        std::cerr << "Dirlist::handlePossibleFile: Unexpected directory.\n"
                  << "File: \"" << possibleFile.string() << "\"\n";
        return -2;
    }

    if (fs::is_regular_file(stat)) {
        if (m_callback) {
            m_callback(path, filename, recursionLevel);
        }
        return 0;
    }

    std::cout << "Dirlist::handlePossibleFile: Unrecognized file type.\n";
    return -1;
}
