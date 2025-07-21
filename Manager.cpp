#include "Manager.hpp"
#include "Utility.hpp"
#include "FileTree.hpp"
#include "BKTree.hpp"

#include <unordered_set>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Code for exact deduplication.

//Global Usage of fileList.
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
void Manager::findExactDuplicates(char* filename) {

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
bool is_image_file(const std::filesystem::path& path) {
    static const std::unordered_set<std::string> image_extensions = {
        ".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".gif", ".webp"
    };

    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return image_extensions.count(ext) > 0;
}

bool is_in_skipped_dir(const std::filesystem::path& path) {
    static const std::unordered_set<std::string> skipDuplication = {
        ".git", ".config", ".cache", ".vscode", ".local", ".venv", ".mozilla", ".thunderbird"
    };
    for (const auto& part : path) {
        if (skipDuplication.count(part.string())) return true;
    }
    return false;
}

int img_report(const std::filesystem::path& path_name) {
    if (!std::filesystem::is_regular_file(path_name)) {
        return -1;
    }

    if (is_in_skipped_dir(path_name)) {
        return -1;
    }

    if (!is_image_file(path_name)) {
        return -1;
    }

    cv::Mat img = cv::imread(path_name.string(), cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        return -1;
    }

    FileInfo obj(path_name);
    fileList.emplace_back(obj);
    return 0;
}


void printSimilarGroups(const std::vector<FileInfo>& fileList, const BKTree& tree, int& ifvideo, int threshold=10){
    //std::cout<<"FileList size "<<fileList.size()<<"\n";
    std::set<std::filesystem::path> visited;
    int count=0;
    // std::cout<<"Function called"<<"\n";
    for(const auto &file: fileList){
        if(visited.count(file.getPath())) continue;

        std::vector<FileInfo> similar;
        tree.findSimilar(file.getImgHash(), threshold, similar, visited);


        if(similar.size()>1){
            if(ifvideo){
                std::cout<<"Group "<<(ifvideo)<<"\n";
                ifvideo++;
            }
            else{
                std::cout<<"Group "<<(++count)<<"\n";
            }
            for(const auto& img: similar){
                std::cout<<" - "<<img.getPath()<<"\n";
                visited.insert(img.getPath());
            }
            std::cout<<"\n";
        }
    }
}

void Manager::findSimilarImages(char* filename){
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
    int ifvideo=0;
    printSimilarGroups(fileList, tree, ifvideo);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//For finding different vidoes which differ only in terms of quality.

int vid_report(const std::filesystem::path& path_name) {
    const std::unordered_set<std::string> skipDuplication = {
        ".git", ".config", ".cache", ".vscode", ".local", ".venv", ".mozilla", ".thunderbird"
    };

    for (const auto& part : path_name) {
        if (skipDuplication.find(part.string())!=skipDuplication.end()) {
            return -1;
        }
    }

    static const std::unordered_set<std::string> videoExtensions = {
        ".mp4", ".mkv", ".avi", ".mov", ".flv", ".wmv", ".webm"
    };

    if (videoExtensions.count(path_name.extension().string()) == 0) {
        return -1; // Skip non-video files
    }


    cv::VideoCapture cap(path_name.string());
    if(!cap.isOpened()){
        return -1; //Not a video file.
    }

    double totalFrames=cap.get(cv::CAP_PROP_FRAME_COUNT);
    double fps=cap.get(cv::CAP_PROP_FPS);

    if(fps<=0 || totalFrames<=0){
        return -1;
    }
    FileInfo file(path_name);
    int duration=(int)(totalFrames/fps);
    file.setDuration(duration);
    fileList.emplace_back(file);

    return 0;
}

bool cmpDurationValFile(const int& a, const FileInfo& b) {
    return a < b.getDuration();
}

void Manager::findSimilarVideos(char* filename){
    std::filesystem::path dir(filename);
    std::cout << "Searching for files in directory: " << dir << "\n";

    // This function is used to walk the specified directory.
    //The false here indicates that the program should not follow symlinks.
    FileTree walker(false);
    walker.setCallback(&vid_report);
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
    
    std::cout<<"Found "<<fileList.size()<<" video files in "<<dir<<" directory\n";

    for(auto &it: fileList){
        it.setVideoHashes();
    }
    
    Utility deduper(fileList);
    std::cout<<deduper.removeMarkedFiles()<<" video files couldn't be read\n";
    std::size_t removed = deduper.removeUniqueDuration();
    std::cout << "Removed " << removed << " files with unique duration.\n";
    std::cout << "Files remaining: " << fileList.size() << "\n\n";

    if(fileList.size()==0){
        return ;
    }
    int start=0, end;
    int ifvideo=1;
    while(start!=fileList.size()){
        end=std::upper_bound(fileList.begin()+start, fileList.end(), fileList[start].getDuration(), cmpDurationValFile)-fileList.begin();
        BKTree tree;
        std::vector<FileInfo> temp;
        while(start!=end){
            tree.insertVideoHashes(fileList[start]);
            temp.push_back(fileList[start]);
            start++;
        }

        printSimilarGroups(temp, tree, ifvideo);
    }
}