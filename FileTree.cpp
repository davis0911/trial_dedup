#include <filesystem>
#include <iostream>
#include <string>
#include "FileTree.hpp"

constexpr int MAX_RECURSION_DEPTH = 50;

namespace fs = std::filesystem;

int FileTree::walk(const std::string& dir, int recursionLevel) {
    if (recursionLevel >= MAX_RECURSION_DEPTH) {
        std::cerr << "Error: Maximum recursion depth exceeded\n";
        return -1;
    }
    //It converts the string dir to std::filesystem::path on its own.
    fs::path dirPath(dir);

    //This is used in order to avoid writing try catch blocks(no exceptions).
    //It's common to use it with filesystem functions.
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
            if (m_followsymlinks) {
                if (fs::is_directory(path, ec)) {
                    walk(path.string(), recursionLevel + 1);
                }
            }
        } else if (fs::is_directory(stat)) {
            walk(path.string(), recursionLevel + 1);
        } else if (fs::is_regular_file(stat)) {
            if (m_callback) {
                m_callback(path, recursionLevel);
            }
        }
    }

    return 2;
}

int FileTree::handlePossibleFile(const fs::path& possibleFile, int recursionLevel) {
    std::error_code ec;

    //File doesn't exist in the first place. Wrong name passed.
    if(!fs::exists(possibleFile, ec)){
        std::cerr<<"Arguement passed doesn't exist"<<"\n";
        return -1;
    }

    // The use of / ensures that based on the system(windows or linux) the correct seperator is added.(\\ or /).
    //Operator overloading of / in std::filesystem.
    const std::string path=(possibleFile.parent_path() / "").string();
    const std::string filename = possibleFile.filename().string();

    //The two available functions were
    //fs::status(dirPath) and fs::symlink_status(dirPath)
    //status() If symlink is passed then status follows the symlink and gives status of the target.
    //symlink_status() If symlink is passed then it doesn't follow it. Marks the symlink status of the file as true.
    fs::file_status stat = fs::symlink_status(possibleFile, ec);
    if (ec) {
        std::cerr << "Error getting status for file: " << possibleFile << ": " << ec.message() << "\n";
        return -1;
    }

    if (fs::is_symlink(stat)) {
        //In C++, private member functions can freely access private member variables of the same class.
        //That is why m_followsymlinks can be directly accessed here. 
        if (m_followsymlinks) {
            if (fs::is_directory(stat)) {
                walk(possibleFile.string(), recursionLevel + 1);
            }
        }
        return 0;
    }

    if (fs::is_regular_file(stat)) {
        std::cout<<"You passed a file as the arguement. No duplicates to check"<<"\n";
        return 0;
    }

    std::cout << "Dirlist::handlePossibleFile: Unrecognized file type.\n";
    return -1;
}
