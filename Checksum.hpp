#ifndef CHECKSUM_HPP
#define CHECKSUM_HPP
#include <string>
#include <cstdint>
#include <opencv2/opencv.hpp> 
class Checksum {
public:
    /**
     * @brief Computes the BLAKE3 hash of a file's contents.
     * 
     * This function reads the file in chunks, updates the BLAKE3 hasher incrementally,
     * and returns the final hash as a lowercase hexadecimal string.
     * 
     * @param filePath Path to the file to be hashed.
     * @return Hexadecimal string representation of the BLAKE3 hash.
     * 
     * @throws std::runtime_error if the file cannot be opened.
     * 
     * Computes BLAKE3 hash of a file and returns it as a hex string
     * Since it is static it belongs to the class and not an object.
     * No need to create an object to call this function.
     */

    static std::string compute(const std::string& filePath);

    static uint64_t computeImagePHash64(const std::string& imgPath);

    static uint64_t phashFromMat(cv::Mat& img);

    //static std::vector<uint64_t> setVideoHashes(const std::string& filePath);

};

#endif 