#ifndef FILEINFO_HPP
#define FILEINFO_HPP

#include <array>
#include <string>
#include <filesystem>
#include <fstream>
#include "Checksum.hpp"

/**
 * @class FileInfo
 * @brief Stores metadata about a file using std::filesystem.
 * 
 * This class encapsulates file attributes such as size, path, etc
 * using the `std::filesystem` library for efficient and portable file information gathering.
 */
class FileInfo {
public:
    
    using filesizetype = std::uintmax_t;

    /**
     * @brief Constructs a FileInfo object with the given path.
     * @param path Filesystem path of the file.
     */
    explicit FileInfo(const std::filesystem::path& path)
        :m_path(path)
        {}

    /**
     * @brief Reads the size of the file and stores it in m_size.
     * @return true if successful, false otherwise.
     */
    bool readFileSize();

    /**
     * @brief Sets the delete flag for this file.
     * @param flag true to mark for removal, false otherwise.
     */
    void setRemoveUniqueFlag(bool flag){m_remove_unique_flag=flag;}

    /**
     * @brief Checks if the file is marked for removal.
     * @return true if marked, false otherwise.
     */
    bool checkRemoveUniqueFlag() const{return m_remove_unique_flag;}

    /**
     * @brief Returns the file size in bytes.
     * @return File size as `filesizetype`.
     */
    filesizetype getSize() const {return m_size;}

    /**
     * @brief Returns the original path of the file.
     * @return Path object referencing the full file path.
     */
    const std::filesystem::path& getPath() const {return m_path;}
    
    /**
     * @brief Reads a fixed amount of first bytes from the file and stores them in a buffer.
     * @return 0 if successful, -1 if the file couldn't be opened.
     */
    int readFirstBytes();

    /**
     * @brief Returns the fixed number of bytes read from the file.
     * @return Buffer size in bytes.
     */
    std::size_t getBufferSize() const{return m_FixedReadSize;}

    //I use this in memcmp in utility.cpp.
    /// get a pointer to the bytes read from the file
    //The .data function returns a pointer to m_somebytes.
    const char* getbyteptr() const {return m_somebytes.data();}

    /**
     * @brief Computes and sets the BLAKE3 hash for this file.
     * 
     * This function computes the BLAKE3 hash of the file located at the path 
     * stored in this FileInfo object and assigns the result to m_blake3_val.
     */
    void setBlake3() {
        m_blake3_val = Checksum::compute(m_path.string());
    }

    void setImgHash(){
        m_phash_val=Checksum::computeImagePHash64(m_path.string());
    }

    uint64_t getImgHash() const{
        return m_phash_val;
    }

    void setDuration(int duration){
        m_duration=duration;
    }
    int getDuration() const{
        return m_duration;
    }
    void setVideoHashes(){
        m_video_hashes=Checksum::setVideoHashes(m_path.string());
    }

    const std::vector<uint64_t>& getVideoHashVector() const{
        return m_video_hashes;
    }
    /**
     * @brief Gets the BLAKE3 hash string for this file.
     * @return A string representing the BLAKE3 hash.
     */
    const std::string getBlake3() const{return m_blake3_val;}


private:
    std::filesystem::path m_path;               // Full file or directory path.
    filesizetype m_size = 0;                    // File size in bytes.(Setting 0 as default.)
    bool m_remove_unique_flag = false;          // True if file should be removed during cleanup.
    
    //constexpr within class must be static.
    //For it to be shared across all instances as a single copy in memory
    //it must be made static.
    static constexpr std::size_t m_FixedReadSize=4096;
    std::array<char, m_FixedReadSize> m_somebytes;
    std::string m_blake3_val;
    uint64_t m_phash_val=0;
    int m_duration=0;
    std::vector<uint64_t> m_video_hashes;
};

#endif // FILEINFO_HPP
