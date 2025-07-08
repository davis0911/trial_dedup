#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <unordered_set>

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
    const std::unordered_set<std::string> skipDuplication={
        ".git", ".config", ".cache", ".vscode", ".local", ".venv", ".mozilla", ".thunderbird"
    };
    for(const auto &part: fullPath){
        if(skipDuplication.contains(part.string())){
            return 0;
        }
    }
    FileInfo fi(fullPath);

    if (fi.readFileInfo() && fi.isRegularFile() && fi.size()>=1024) {
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
    // for(auto &it: fileList){
    //     for(const auto &part: it.path()){
    //         std::cout<<part<<" ";
    //     }
    //     std::cout<<"\n";
    // }
    // Run removeUniqueSizes
    // Utility deduper(fileList);
    // std::size_t removed = deduper.removeUniqueSizes();
    // for(auto &it: fileList){
    //     std::cout<<it.path()<<" "<<it.size()<<"\n";
    // }

    //Run removeUniqueSizes
    Utility deduper(fileList);
    std::size_t removed = deduper.removeUniqueSizes();

    std::cout << "Removed " << removed << " files with unique sizes.\n";
    std::cout << "Files remaining: " << fileList.size() << "\n\n";

    // Print final filtered file list
    // for (const auto& file : fileList) {
    //     std::cout << file.path() << " - " << file.size() << " bytes\n";
    // }

    for(auto &file: fileList){
        file.readFirstBytes();
    }
    removed=deduper.removeUniqueBuffer();
    std::cout<<"Removed "<<removed<<" files with different first bytes.\n";
    std::cout<<"Files remaining "<<fileList.size()<<"\n\n";

    for(auto &file: fileList){
        file.readLastBytes();
    }
    removed=deduper.removeUniqueBuffer();
    std::cout<<"Removed "<<removed<<" files with different last bytes.\n";
    std::cout<<"Files remaining "<<fileList.size()<<"\n\n";

    deduper.setHash();
    removed=deduper.removeUniqueHashes();
    std::cout<<"Removed "<<removed<<" files with unique hashes"<<"\n";
    std::cout<<"Files remaining "<<fileList.size()<<"\n\n";

    deduper.sortFilesBySize();
    int count=1;
    int beg=0;
    for(int i=1; i<fileList.size(); ++i){
        if(fileList[i].size()==fileList[i-1].size()){
            count++;
        }
        else{
            int interval=beg+count;
            std::cout<<"Found "<<count<<" files of size "<<fileList[beg].size()<<"\n";
            for(int j=beg; j<interval; j++){
                std::cout<<fileList[j].path()<<"\n";
            }
            std::cout<<"\n\n";
            beg=i;
            count=1;
        }
    }
    //This will always get executed.
    if(count!=1){
        int interval=beg+count;
        //std::cout<<beg<<" "<<count<<"\n";
        std::cout<<"Found "<<count<<" files of size "<<fileList[beg].size()<<"\n";
        for(int j=beg; j<(interval); j++){
            std::cout<<fileList[j].path()<<"\n";
        }
    }
    std::cout<<"\n\n";
    //deduper.sortFilesBySize();
    // for (const auto& file : fileList) {
    //     std::cout << file.path() << " - " << file.size() << " bytes\n";
    // }
    return 0;
}
