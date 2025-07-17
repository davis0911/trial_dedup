#include <algorithm> 
#include <cstring> //used for memcmp
#include <iostream>
#include <unordered_set>
#include <opencv2/opencv.hpp>     // OpenCV core functionality (e.g., cv::Mat, imread, resize, dct, etc.)

#include "Utility.hpp"
#include "FileInfo.hpp"
#include "Checksum.hpp"
#include "FileTree.hpp"
#include "BKTree.hpp"


/// Function template
/// Helper: apply callback on equal subranges
/// Go through the list, group files according to a specific criteria, and for each group, run the passed callback function.
template <typename Iterator, typename Comparator, typename Callback>
void apply_on_range(Iterator first, Iterator last, Comparator comp, Callback cb) {
    while (first != last) {
        auto [range_first, range_last] = std::equal_range(first, last, *first, comp);
        cb(range_first, range_last);
        first = range_last;
    }
}

// compares file size
bool cmpSize(const FileInfo& a, const FileInfo& b){
  return a.getSize() < b.getSize();
}

//Lexicographical comparator on raw binary content.
bool cmpBuffers(const FileInfo& a, const FileInfo& b){
  return std::memcmp(a.getbyteptr(), b.getbyteptr(), a.getBufferSize()) < 0;
}

//Lexicographical comparator on the blake3 hashes of the files.
bool cmpHash(const FileInfo &a, const FileInfo &b){
    return a.getBlake3()<b.getBlake3();
}

//Sort all the functions in the given list according to size.
void Utility::sortFilesBySize(){
    std::sort(m_list.begin(), m_list.end(), cmpSize);
}

//The callback function which is going to be passed to apply_on_range which is going to process every group.
auto handle_size_group = [](auto first, auto last) {
    if(std::distance(first, last) == 1) {
        first->setRemoveUniqueFlag(true);  //Set removeunique flag if the size of the group is only 1.
    } 
};

/// Remove all entries marked for removal
std::size_t Utility::cleanup() {
    const auto old_size = m_list.size();

    //erase takes a pointer and removes all elements from that pointer to the end of the vector.
    m_list.erase(std::remove_if(m_list.begin(), m_list.end(),
        [](const FileInfo& f) { return f.checkRemoveUniqueFlag(); }),
        m_list.end());

    //Return how many elements were removed after cleanup.
    return old_size - m_list.size();
}

/// Remove files with unique sizes
std::size_t Utility::removeUniqueSizes() {
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpSize);

    // Step 2: Apply removal marking logic to the groups.
    apply_on_range(m_list.begin(), m_list.end(), cmpSize, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
    //return 0;
}

//Remove files with unique buffers.
std::size_t Utility::removeUniqueBuffer() {
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpBuffers);

    // Step 2: Apply removal marking logic to the groups.
    apply_on_range(m_list.begin(), m_list.end(), cmpBuffers, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
}

//Remove files with unique hashes.
std::size_t Utility::removeUniqueHashes(){
    // Step 1: Sort by size
    std::sort(m_list.begin(), m_list.end(), cmpHash);

    // Step 2: Apply removal marking logic to the groups.
    apply_on_range(m_list.begin(), m_list.end(), cmpHash, handle_size_group);

    // Step 3: Remove marked files
    return cleanup();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Code for exact deduplication.

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
int dedup_report(const std::filesystem::path& path_name) {
    /// Set of directory names to skip during traversal.
    const std::unordered_set<std::string> skipDuplication = {
        ".git", ".config", ".cache", ".vscode", ".local", ".venv", ".mozilla", ".thunderbird"
    };

    for (const auto& part : path_name) {
        if (skipDuplication.find(part.string())!=skipDuplication.end()) {
            return -1;
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
void Utility::findExactDuplicates(char* filename) {
    std::filesystem::path dir(filename);
    std::cout << "Searching for files in directory: " << dir << "\n";

    // This function is used to walk the specified directory.
    //The false here indicates that the program should not follow symlinks.
    FileTree walker(false);
    walker.setCallback(&dedup_report);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//For detecting similar images.

int img_report(const std::filesystem::path& path_name) {
    const std::unordered_set<std::string> skipDuplication = {
        ".git", ".config", ".cache", ".vscode", ".local", ".venv", ".mozilla", ".thunderbird"
    };

    for (const auto& part : path_name) {
        if (skipDuplication.find(part.string())!=skipDuplication.end()) {
            return -1;
        }
    }

    cv::Mat img = cv::imread(path_name.string(), cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        return -1;
    }
    FileInfo obj(path_name);
    fileList.emplace_back(obj);  // Construct FileInfo in-place
    return 0;
}

void printSimilarGroups(const std::vector<FileInfo>& fileList, const BKTree& tree, int threshold=10){
    //std::cout<<"FileList size "<<fileList.size()<<"\n";
    std::set<std::filesystem::path> visited;
    int count=0;
    // std::cout<<"Function called"<<"\n";
    for(const auto &file: fileList){
        if(visited.count(file.getPath())) continue;

        std::vector<FileInfo> similar;
        tree.findSimilar(file.getImgHash(), threshold, similar, visited);


        if(similar.size()>1){
            std::cout<<"Group "<<(++count)<<"\n";
            for(const auto& img: similar){
                std::cout<<" - "<<img.getPath()<<"\n";
                visited.insert(img.getPath());
            }
            std::cout<<"\n";
        }
    }
}

void Utility::similarImages(char * filename){
    std::filesystem::path dir(filename);
    std::cout << "Searching for files in directory: " << dir << "\n";

    // This function is used to walk the specified directory.
    //The false here indicates that the program should not follow symlinks.
    FileTree walker(false);
    walker.setCallback(&img_report);
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
    for(auto &it: fileList){
        it.setImgHash();
    }
    BKTree tree;
    for(auto &file: fileList){
        tree.insert(file);
    }
    printSimilarGroups(fileList, tree);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////