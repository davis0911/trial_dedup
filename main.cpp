#include <iostream>

#include "Manager.hpp"

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
        Manager::findExactDuplicates(argv[2]);
    }
    else if(mode=="img"){
        Manager::findSimilarImages(argv[2]);
    }
    else if(mode=="vid"){
        Manager::findSimilarVideos(argv[2]);
    }
    else{
        std::cout<<"Invalid input"<<"\n";
    }
    return 0;
}
