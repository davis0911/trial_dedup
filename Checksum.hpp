// Checksum.hpp
#pragma once
#include <string>

class Checksum {
public:
    // Computes BLAKE3 hash of a file and returns it as a hex string
    static std::string compute(const std::string& filePath);
};
