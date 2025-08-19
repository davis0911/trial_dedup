// #include <filesystem>
// #include <iostream>
// #include <string>
// #include "FileTree.hpp"

// namespace fs = std::filesystem;

// int FileTree::walk(const std::string& dir, int recursionLevel) {
//     //It converts the string dir to std::filesystem::path on its own.
//     fs::path dirPath(dir);

//     //This is used in order to avoid writing try catch blocks(no exceptions).
//     //It's common to use it with filesystem functions.
//     std::error_code ec;

//     //Get the path resolved to know its exact form.
//     //If it is a symlink then it will resolved to where it points.
//     if (!fs::is_directory(dirPath, ec)) {
//         return handlePossibleFile(dirPath, recursionLevel);
//         //return 1;
//     }

//     fs::path canonicalPath = fs::weakly_canonical(dirPath, ec);
//     if(ec){
//         std::cerr<<"Error resolving canonical path for "<<dirPath<<"\n";
//         return -1;
//     }
//     //If given path has been visited then return.
//     if(visitedDirs.count(canonicalPath)){
//         return 0;
//     }
//     //If the path is being visited for the first time then add it to the set.
//     visitedDirs.insert(canonicalPath);

//     for (const auto& entry : fs::directory_iterator(dirPath, fs::directory_options::skip_permission_denied, ec)) {
//         if (ec) {
//             //When system's broken symlinks are read.
//             std::cerr << "Error reading path " << entry.path() << "': " << ec.message() << '\n';
//             continue;
//         }

//         //entry is of type std::filesystem::directory_entry.
//         //This is an object that represents a directory entry (file, dir, symlink, etc.).
//         // But entry is not a path itself — it's an object that contains a path along with cached metadata (like file type, permissions, etc.).
//         const auto& path = entry.path();
//         const std::string name = path.filename().string();

//         std::error_code status_ec;
//         fs::file_status stat = entry.symlink_status(status_ec);
//         if (status_ec) {
//             std::cerr << "Error: Cannot get file status for " << path << ": " << status_ec.message() << "\n";
//             continue;
//         }

//         if (fs::is_symlink(stat)) {
//             if (m_followsymlinks) {
//                 //std::cout<<"YES\n";
//                 if (fs::is_directory(path, ec)) {
//                     walk(path.string(), recursionLevel + 1);
//                 }
//                 // else if(fs::is_regular_file(path, ec)){
//                 //     m_callback(path);
//                 // }
//             }
//             // else{
//             //     continue;
//             // }
//         } else if (fs::is_directory(stat)) {
//             walk(path.string(), recursionLevel + 1);
//         } else if (fs::is_regular_file(stat)) {
//             if (m_callback) {
//                 m_callback(path);
//             }
//         }
//     }

//     return 2;
// }

// int FileTree::handlePossibleFile(const fs::path& possibleFile, int recursionLevel) {
//     std::error_code ec;

//     //File doesn't exist in the first place. Wrong name passed.
//     if(!fs::exists(possibleFile, ec)){
//         std::cerr<<"Arguement passed doesn't exist\n";
//         return -1;
//     }

//     // The use of / ensures that based on the system(windows or linux) the correct seperator is added.(\\ or /).
//     //Operator overloading of / in std::filesystem.
//     // const std::string path=(possibleFile.parent_path() / "").string();
//     // const std::string filename = possibleFile.filename().string();

//     //The two available functions were
//     //fs::status(dirPath) and fs::symlink_status(dirPath)
//     //status() If symlink is passed then status follows the symlink and gives status of the target.
//     //symlink_status() If symlink is passed then it doesn't follow it. Marks the symlink status of the file as true.
//     fs::file_status stat = fs::symlink_status(possibleFile, ec);
//     if (ec) {
//         std::cerr << "Error getting status for file: " << possibleFile << ": " << ec.message() << "\n";
//         return -1;
//     }

//     if (fs::is_symlink(stat)) {
//         if (m_followsymlinks) {
//             std::cout<<"HEREEE\n";
//             fs::path resolvedPath = fs::weakly_canonical(possibleFile, ec);
//             if (ec) {
//                 std::cerr << "Error resolving symlink target: " << possibleFile << ": " << ec.message() << "\n";
//                 return -1;
//             }

//             if (fs::is_directory(resolvedPath, ec)) {
//                 if (ec) {
//                     std::cerr << "Error checking if resolved path is directory: " << resolvedPath << ": " << ec.message() << "\n";
//                     return -1;
//                 }
//                 return walk(resolvedPath.string(), recursionLevel + 1);
//             }  
//         }
//         else {
//                 std::cout << "Cannot follow symlink. Set parameter to true in input to follow\n";
//                 return -1;
//             }
//     }
//     if (fs::is_regular_file(stat)) {
//         std::cout<<"You passed a file as the arguement. No duplicates to check"<<"\n";
//         return 0;
//     }

//     std::cout << "Dirlist::handlePossibleFile: Unrecognized file type.\n";
//     return -1;
// }

#include <filesystem>
#include <iostream>
#include <string>
#include "FileTree.hpp"

namespace fs = std::filesystem;

int FileTree::walk(const std::string& dir, int recursionLevel) {
    fs::path dirPath(dir);
    std::error_code ec;

    fs::file_status stat = fs::symlink_status(dirPath, ec);
    if (ec) {
        std::cerr << "Error getting status of " << dirPath << ": " << ec.message() << "\n";
        return -1;
    }

    if (fs::is_symlink(stat)) {
        return handlePossibleFile(dirPath, recursionLevel);
    }

    if (!fs::is_directory(stat)) {
        return handlePossibleFile(dirPath, recursionLevel);
    }

    fs::path canonicalPath = fs::weakly_canonical(dirPath, ec);
    if (ec) {
        std::cerr << "Error resolving canonical path for " << dirPath << "\n";
        return -1;
    }

    if (visitedDirs.count(canonicalPath)) {
        return 0;
    }

    visitedDirs.insert(canonicalPath);

    for (const auto& entry : fs::directory_iterator(dirPath, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) {
            std::cerr << "Error reading path " << entry.path() << ": " << ec.message() << '\n';
            continue;
        }

        const auto& path = entry.path();
        std::error_code status_ec;
        fs::file_status entryStat = entry.symlink_status(status_ec);
        if (status_ec) {
            std::cerr << "Error: Cannot get file status for " << path << ": " << status_ec.message() << "\n";
            continue;
        }

        if (fs::is_symlink(entryStat)) {
            if (m_followsymlinks) {
                if (fs::is_directory(path, ec)) {
                    if (!ec) {
                        int res = walk(path.string(), recursionLevel + 1);
                        if (res < 0) return res;
                    }
                }
            }
        } else if (fs::is_directory(entryStat)) {
            int res = walk(path.string(), recursionLevel + 1);
            if (res < 0) return res;
        } else if (fs::is_regular_file(entryStat)) {
            if (m_callback) {
                m_callback(path);
            }
        }
    }

    return 2;
}


int FileTree::handlePossibleFile(const fs::path& possibleFile, int recursionLevel) {
    std::error_code ec;

    if (!fs::exists(possibleFile, ec)) {
        std::cerr << "Argument passed doesn't exist\n";
        return -1;
    }

    fs::file_status stat = fs::symlink_status(possibleFile, ec);
    if (ec) {
        std::cerr << "Error getting status for file: " << possibleFile << ": " << ec.message() << "\n";
        return -1;
    }

    if (fs::is_symlink(stat)) {
        if (m_followsymlinks) {
            fs::path resolvedPath = fs::weakly_canonical(possibleFile, ec);
            if (ec) {
                std::cerr << "Error resolving symlink target: " << possibleFile << ": " << ec.message() << "\n";
                return -1;
            }

            if (fs::is_directory(resolvedPath, ec)) {
                if (ec) {
                    std::cerr << "Error checking if resolved path is directory: " << resolvedPath << ": " << ec.message() << "\n";
                    return -1;
                }
                return walk(resolvedPath.string(), recursionLevel + 1);
            }
        } else {
            std::cout << "Cannot follow symlink. Set parameter to true in input to follow\n";
            return -1;
        }
    }

    if (fs::is_regular_file(stat)) {
        std::cout << "You passed a file as the argument. No duplicates to check\n";
        return 0;
    }

    std::cout << "Dirlist::handlePossibleFile: Unrecognized file type.\n";
    return -1;
}
