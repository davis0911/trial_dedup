// Checksum.cpp

#include "Checksum.hpp"
#include "blake3.h"           // BLAKE3 hash function API

#include <fstream>            // For std::ifstream
#include <sstream>            // For output string formatting
#include <iomanip>            // For hex formatting (setw, setfill)

#include <opencv2/opencv.hpp>     // OpenCV core functionality (e.g., cv::Mat, imread, resize, dct, etc.)
#include <cstdint>                // For fixed-width integer types like uint64_t
#include <vector>                 // To store DCT coefficients in a std::vector / For buffer storage.
#include <algorithm>              // For std::nth_element (to find the median)
#include <stdexcept>              // For std::runtime_error when image loading fails / For throwing file read exceptions.
#include <string>                 // For std::string in function parameter


std::string Checksum::compute(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary); // Open file in binary mode
    if (!file)
        throw std::runtime_error("Failed to open file: " + filePath);

    // Initialize the BLAKE3 hasher context
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);

    // Buffer to read file chunks (16 KB)
    std::vector<char> buffer(16384);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        // Feed read bytes to the hasher (even if less than full buffer)
        //buffer.data() input pointer.
        //file.gcount() input length().
        //.gcount() returns how many bytes were read by the .read function in its last attempt.
        blake3_hasher_update(&hasher, buffer.data(), file.gcount());
    }

    // Finalize the hash computation and get the 32-byte digest
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);

    // Convert hash bytes to hex string (2 characters per byte)
    std::ostringstream oss;
    for (int i = 0; i < BLAKE3_OUT_LEN; ++i)
        oss << std::hex                // Use hexadecimal output
            << std::setw(2)            // Always print 2 characters
            << std::setfill('0')       // Pad with '0' if needed (e.g., 0a instead of a)
            << static_cast<int>(output[i]); // Cast byte to int for correct formatting

    //64 character long hash.
    return oss.str();
}

uint64_t Checksum::computeImagePHash64(const std::string& imagePath) {
    // Load image in grayscale (works for color images too)
    cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        throw std::runtime_error("Failed to load image: " + imagePath);
    }

    // Resize to 32x32 for DCT
    cv::resize(img, img, cv::Size(32, 32));
    img.convertTo(img, CV_32F);

    // Apply Discrete Cosine Transform
    cv::Mat dctImg;
    cv::dct(img, dctImg);

    // Take top-left 8x8 block of low frequencies
    cv::Mat dctLowFreq = dctImg(cv::Rect(0, 0, 8, 8));

    // Flatten into a 1D vector safely
    std::vector<float> vals;
    vals.reserve(64);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            vals.push_back(dctLowFreq.at<float>(i, j));
        }
    }

    // Optionally skip DC component (first term) — comment out if you want 64-bit
    // vals.erase(vals.begin());

    // Compute median
    std::vector<float> sortedVals = vals;  // Copy for median calculation
    std::nth_element(sortedVals.begin(), sortedVals.begin() + sortedVals.size() / 2, sortedVals.end());
    float median = sortedVals[sortedVals.size() / 2];

    // Generate hash
    uint64_t hash = 0;
    for (size_t i = 0; i < vals.size(); ++i) {
        if (vals[i] > median) {
            hash |= (1ULL << (63 - i));  // MSB first
        }
    }

    return hash;
}

int Checksum::hammingDistance(uint64_t hash1, uint64_t hash2) {
    return __builtin_popcountll(hash1 ^ hash2);  // GCC/Clang only
}

