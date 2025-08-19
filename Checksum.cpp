// Checksum.cpp

#include "Checksum.hpp"
#include "blake3.h"           // BLAKE3 hash function

#include <fstream>            // For std::ifstream
#include <sstream>            // For output string formatting
#include <iomanip>            // For hex formatting (setw, setfill)

#include <opencv2/opencv.hpp>     // OpenCV core functionality (e.g., cv::Mat, imread, resize, dct, etc.)
#include <cstdint>                // For fixed-width integer types like uint64_t
#include <vector>                 
#include <algorithm>              
#include <stdexcept>              // For std::runtime_error when image loading fails / For throwing file read exceptions.
#include <string>                 // For std::string in function parameter


std::string Checksum::compute(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary); // Open file in binary mode
    if (!file){
        std::cerr<<"Failed to open file "<<filePath<<". Removed it from the hashing process\n";
        return "";
    }

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
            << (int)(output[i]); // Cast byte to int for correct formatting

    //64 character long hash.
    return oss.str();
}

uint64_t Checksum::computeImagePHash64(const std::string& imagePath) {
    // Load image in grayscale (works for color images too)
    //Each value in tha matrix is an unsigned 8bit number(0-255).
    //The datatype of each pixel is uchar.
    //0-black 255-white.
    cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr<<"Failed to load image: "<<imagePath<<"\n";
        return 0;
    }

    // Resize to 32x32 for DCT
    //The resultant matrix has type CV_8UC1
    //8bit unsigned char with 1 channel(grayscale).
    cv::resize(img, img, cv::Size(32, 32));

    //Converting all the values from (0-255) from unsigned char to float.
    //For accuracy in dct.
    img.convertTo(img, CV_32F);

    // Apply Discrete Cosine Transform
    // DCT transforms the image from the spatial domain (pixel intensities) to the frequency domain.
    cv::Mat dctImg;
    cv::dct(img, dctImg);

    // Take top-left 8x8 block as it contains the lowest frequencies
    cv::Mat dctLowFreq = dctImg(cv::Rect(0, 0, 8, 8));

    // Flatten into a 1D vector safely
    std::vector<float> vals;
    vals.reserve(64);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            vals.push_back(dctLowFreq.at<float>(i, j));
        }
    }

    //The first value, top-left of DCT is removed because it represents
    //overall brightness, not structure.
    //Time complexity O(63). Will optimize later if needed.
    vals.erase(vals.begin());

    // Compute median
    std::vector<float> sortedVals = vals;  // Copy for median calculation
    std::nth_element(sortedVals.begin(), sortedVals.begin() + sortedVals.size() / 2, sortedVals.end());
    //If I were to use 64(even) I would have to take average of middle two values.
    float median = sortedVals[sortedVals.size() / 2];

    // Generate hash
    uint64_t hash = 0;
    for (size_t i = 0; i < vals.size(); ++i) {
        if (vals[i] > median) {
            hash |= (1ULL << (vals.size()-i-1));  // MSB first
        }
    }

    return hash;
}

uint64_t Checksum::phashFromMat(cv::Mat & img){
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

    vals.erase(vals.begin());

    // Compute median
    std::vector<float> sortedVals = vals;  // Copy for median calculation
    std::nth_element(sortedVals.begin(), sortedVals.begin() + sortedVals.size() / 2, sortedVals.end());
    float median = sortedVals[sortedVals.size() / 2];

    // Generate hash
    uint64_t hash = 0;
    for (size_t i = 0; i < vals.size(); ++i) {
        if (vals[i] > median) {
            hash |= (1ULL << (vals.size()-i-1));  // MSB first
        }
    }

    return hash;
}

std::vector<uint64_t> Checksum::setVideoHashes(const std::string& videoPath){
  cv::VideoCapture cap(videoPath);
  if(!cap.isOpened()){
    std::cerr<<"Failed to open video file"<<videoPath<<"\n";
    std::vector<uint64_t> vec;
    return vec;
  }

  double totalFrames=cap.get(cv::CAP_PROP_FRAME_COUNT);
  const int numSamples=10;
  std::vector<uint64_t> video_hashes;
  for(int i=0; i<numSamples; ++i){
    //starts from 0 to totalFrames-1;
    int frameIndex=(int)((i*totalFrames)/numSamples);
    cap.set(cv::CAP_PROP_POS_FRAMES, frameIndex);
    
    cv::Mat frame;
    if(!cap.read(frame) || frame.empty()){
      //std::cerr<<"Failed to read frame "<<frameIndex<<" frame of "<<m_path<<"\n";
      //If even the first frame could not be read then mark it for removal.
      return video_hashes;
    }

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    uint64_t hashval=Checksum::phashFromMat(gray);
    video_hashes.push_back(hashval);
  }
  return video_hashes;

}