// Checksum.cpp
#include "Checksum.hpp"
#include "blake3.h"

#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>

std::string Checksum::compute(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file: " + filePath);

    blake3_hasher hasher;
    blake3_hasher_init(&hasher);

    std::vector<char> buffer(16384);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        blake3_hasher_update(&hasher, buffer.data(), file.gcount());
    }

    //0-255 range.
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);

    std::ostringstream oss;
    for (int i = 0; i < BLAKE3_OUT_LEN; ++i)
        //00-ff range. if ony single letter like a then make it 0a with setfill.
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)output[i];

    return oss.str();
}
