#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <unordered_set>

#include "FileTree.hpp"
#include "FileInfo.hpp"
#include "Utility.hpp"

std::vector<FileInfo> fileList;

std::string beautify(uintmax_t size) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    if (size >= 1073741824) {
        oss << (double)size / 1073741824 << " GB";
    } else if (size >= 1048576) {
        oss << (double)size / 1048576 << " MB";
    } else if (size >= 1024) {
        oss << (double)size / 1024 << " KB";
    } else {
        oss << size << " B";
    }

    return oss.str();
}

/**
 * @brief Callback function to process a file or directory during traversal.
 *
 * This function checks if the given file should be skipped based on its path.
 * If it is not skipped and is a regular file of at least 1KB, it is added to the global file list.
 *
 * @param path The full path being scanned.
 * @param depth The depth of the traversal.
 * @return Always returns 0.
 */
int report(const std::filesystem::path& path_name, int depth) {
    /// Set of directory names to skip during traversal.
    const std::unordered_set<std::string> skipDuplication = {
        ".git", ".config", ".cache", ".vscode", ".local", ".venv", ".mozilla", ".thunderbird"
    };

    for (const auto& part : path_name) {
        if (skipDuplication.contains(part.string())) {
            return 0;
        }
    }

    FileInfo fi(path_name);

    if (fi.readFileSize() && fi.getSize() >= 1024) {
        fileList.push_back(fi);
    }

    return 0;
}

/**
 * @brief Finds and reports exact duplicate files within a given directory.
 *
 * This function performs several stages of duplicate detection:
 * - Walking the file tree and collecting eligible files
 * - Removing files with unique sizes
 * - Removing files with unique beginning byte patterns
 * - Removing files with unique hash values
 * - Grouping and printing remaining files by identical sizes
 *
 * @param filename Path to the directory in which to search for duplicate files.
 */
void findExactDuplicates(char* filename) {
    std::filesystem::path dir(filename);
    std::cout << "Searching for files in directory: " << dir << "\n";

    // This function is used to walk the specified directory.
    //The false here indicates that the program should not follow symlinks.
    FileTree walker(false);
    walker.setCallback(&report);
    int status=walker.walk(dir.string());

    if(status==1){
        return ;
    }
    else if(status==-1){
        return ;
    }

    if(fileList.size()==0){
        std::cout<<"File List is empty."<<"\n";
        return;
    }

    std::cout << "Total files before filtering: " << fileList.size() << "\n";

    //This object of Utility class is used to find duplicate files using various techniques.
    Utility deduper(fileList);

    //1.
    //removeUniqueSizes removes all the files with unique file size within the mentioned directory and 
    //following sub-directories and returns the number of removed files.
    std::size_t removed = deduper.removeUniqueSizes();
    std::cout << "Removed " << removed << " files with unique sizes.\n";
    std::cout << "Files remaining: " << fileList.size() << "\n\n";

    if(fileList.size()==0){
        return ;
    }

    //2.
    // This serves as a quick content-based pre-filter to eliminate files that differ early,
    // reducing the workload for full hashing.
    for (auto& file : fileList) {
        file.readFirstBytes();
    }
    //removeUniqueBuffer removes all the files with unique firstbytes(default buffer size set to 4kB)
    //and returns the total number of such removed files.
    removed = deduper.removeUniqueBuffer();
    std::cout << "Removed " << removed << " files with unique first bytes.\n";
    std::cout << "Files remaining " << fileList.size() << "\n\n";

    if(fileList.size()==0){
        return ;
    }

    //3.
    //The setHash function is used to hash the contents of the entire file and store it in the form 
    //of a string in the member-variable of the class FileInfo called m_blake3_val.
    for(auto &file: fileList){
        file.setBlake3();
    }

    //removeUniqueHashes removes all the files with unique hashes from the fileList and returns the number
    //of files which it removed.
    removed = deduper.removeUniqueHashes();
    std::cout << "Removed " << removed << " files with unique hashes\n";
    std::cout << "Files remaining " << fileList.size() << "\n\n";

    if(fileList.size()==0){
        return ;
    }

    //This is used to sort the FileList based on the size of the files.
    deduper.sortFilesBySize();

    //The code given below is to display all files which are duplicates and their regarding details.
    int count = 1;
    int beg = 0;
    for (int i = 1; i < fileList.size(); ++i) {
        if (fileList[i].getSize() == fileList[i - 1].getSize()) {
            count++;
        } else {
            int interval = beg + count;
            std::cout << "Found " << count << " files of size " << beautify(fileList[beg].getSize()) << "\n";
            for (int j = beg; j < interval; j++) {
                std::cout << fileList[j].getPath() << "\n";
            }
            std::cout << "\n\n";
            beg = i;
            count = 1;
        }
    }

    // This will always get executed.
    if (count != 1) {
        int interval = beg + count;
        std::cout << "Found " << count << " files of size " << beautify(fileList[beg].getSize()) << "\n";
        for (int j = beg; j < interval; j++) {
            std::cout << fileList[j].getPath() << "\n";
        }
    }

    std::cout << "\n\n";
}



int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: Not enough arguments.\n";
        std::cerr << "Expected usage:\n"
                << "  " << argv[0] << " dedup <directory>    # Deduplicate files\n"
                << "  " << argv[0] << " img <directory>      # Filter image files\n"
                << "  " << argv[0] << " vid <directory>      # Filter video files\n";

        return 1;
    }
    std::string mode=std::string(argv[1]);
    if(mode=="dedup"){
        findExactDuplicates(argv[2]);
    }
    else{
        std::cout<<"Invalid input"<<"\n";
    }
    return 0;
}
