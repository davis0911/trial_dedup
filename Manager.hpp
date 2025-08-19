#ifndef MANAGER_HPP
#define MANAGER_HPP
class Manager{
    public:
        static void findExactDuplicates(char* filename, bool follow_symlinks);

        static void findSimilarImages(char* filename, bool follow_symlinks);

        static void findSimilarVideos(char* filename, bool follow_symlinks);
        
};

#endif