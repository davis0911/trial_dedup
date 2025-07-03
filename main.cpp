#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

#include "FileTree.hpp"
#include "FileInfo.hpp"
#include "Utility.hpp"

/// @brief Global container to store discovered FileInfo objects.
std::vector<FileInfo> fileList;

/**
 * @brief Callback function used during directory traversal.
 */
int report(const std::string& path, const std::string& name, int depth) {
    std::filesystem::path fullPath = std::filesystem::path(path) / name;

    FileInfo fi(fullPath);
    if (fi.readFileInfo() && fi.isRegularFile()) {
        fileList.push_back(fi);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory>\n";
        return 1;
    }

    std::filesystem::path dir(argv[1]);
    std::cout << "Searching for files in directory: " << dir << "\n";

    // Traverse directory and populate fileList
    FileTree walker(false);
    walker.setCallback(&report);
    walker.walk(dir.string());

    std::cout << "Total files before filtering: " << fileList.size() << "\n";

    // Run removeUniqueSizes
    Utility deduper(fileList);
    std::size_t removed = deduper.removeUniqueSizes();

    std::cout << "Removed " << removed << " files with unique sizes.\n";
    std::cout << "Files remaining: " << fileList.size() << "\n\n";

    // Print final filtered file list
    for (const auto& file : fileList) {
        std::cout << file.path() << " - " << file.size() << " bytes\n";
    }

    return 0;
}
