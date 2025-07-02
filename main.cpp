#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

#include "FileTree.hpp"
#include "FileInfo.hpp"

/// @brief Global container to store discovered FileInfo objects.
std::vector<FileInfo> fileList;

/**
 * @brief Callback function used during directory traversal.
 * 
 * Constructs a full path from directory and filename, reads metadata using FileInfo,
 * and stores the object in a global list if it's a regular file.
 * 
 * @param path The parent directory path.
 * @param name The name of the file or subdirectory.
 * @param depth The current depth of the traversal (unused here).
 * @return Always returns 0.
 */
int report(const std::string& path, const std::string& name, int depth) {
    std::filesystem::path fullPath = std::filesystem::path(path) / name;

    FileInfo fi(fullPath);
    if (fi.readFileInfo() && fi.isRegularFile()) {
        fileList.push_back(fi);
    }

    return 0;
}

/**
 * @brief Entry point for the file discovery and reporting program.
 * 
 * Validates command-line arguments, sets up a FileTree walker, invokes
 * recursive traversal, and prints sorted file info based on file size.
 * 
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory>\n";
        return 1;
    }

    std::filesystem::path dir(argv[1]);
    std::cout << "Searching for files in directory: " << dir << "\n";

    // Setup and execute file traversal
    FileTree walker(/*followsymlinks=*/false);
    walker.setCallback(&report);
    walker.walk(dir.string());

    // Sort files by size in ascending order
    std::sort(fileList.begin(), fileList.end(),
        [](const FileInfo& a, const FileInfo& b) {
            return a.size() < b.size();
        });

    // Print results
    for (const auto& file : fileList) {
        std::cout << file.path() << " - " << file.size() << " bytes\n";
    }

    return 0;
}
