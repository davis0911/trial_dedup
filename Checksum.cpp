// Checksum.cpp

#include "Checksum.hpp"
#include "blake3.h"           // BLAKE3 hash function API

#include <fstream>            // For std::ifstream
#include <vector>             // For buffer storage
#include <sstream>            // For output string formatting
#include <iomanip>            // For hex formatting (setw, setfill)
#include <stdexcept>          // For throwing file read exceptions


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
