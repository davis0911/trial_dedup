#include <iostream>

#include "Manager.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: Not enough arguments.\n";
        std::cerr << "Expected usage:\n"
                << "  " << argv[0] << " dedup <directory> [follow_symlinks]   # Deduplicate files\n"
                << "  " << argv[0] << " img <directory>   [follow_symlinks]   # Filter image files\n"
                << "  " << argv[0] << " vid <directory>   [follow_symlinks]   # Filter video files\n"
                << "   [follow_symlinks] by default set to false.\n";

        return 1;
    }
    std::string mode=std::string(argv[1]);
    bool follow_symlinks=false;
    if(argc==4){
        std::string check=std::string(argv[3]);
        if(check=="true"){
            follow_symlinks=true;
        }
        else if(check!="false"){
            std::cerr<<"follow_symlinks parameter should be either true or false. Found "<<check<<"\n";
            return 0;
        }
    }
    if(mode=="dedup"){
        Manager::findExactDuplicates(argv[2], follow_symlinks);
    }
    else if(mode=="img"){
        Manager::findSimilarImages(argv[2], follow_symlinks);
    }
    else if(mode=="vid"){
        Manager::findSimilarVideos(argv[2], follow_symlinks);
    }
    else{
        std::cout<<"Invalid input"<<"\n";
    }
    return 0;
}

/*
To compile use
g++ main.cpp FileTree.cpp FileInfo.cpp Utility.cpp Checksum.cpp BKTree.cpp Manager.cpp 
$(pkg-config --cflags --libs opencv4) -lblake3 -o output
*/