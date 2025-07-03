#ifndef FILEINFO_HPP
#define FILEINFO_HPP

#include <string>
#include <filesystem>

/**
 * @class FileInfo
 * @brief Stores metadata about a file using modern C++17 filesystem API.
 * 
 * This class encapsulates file attributes such as size, path, and type (regular file or directory),
 * using the `std::filesystem` library for efficient and portable file information gathering.
 */
class FileInfo {
public:
    /**
     * @brief Type alias for file size type.
     * Uses `std::uintmax_t`, which matches `std::filesystem::file_size()` return type.
     */
    using filesizetype = std::uintmax_t;

    /**
     * @brief Constructs a FileInfo object with the given path.
     * @param path Full file or directory path.
     */
    explicit FileInfo(std::filesystem::path path);

    /**
     * @brief Reads file metadata using `std::filesystem::symlink_status()` and `file_size()`.
     * 
     * Determines if the path points to a regular file or directory, and sets internal flags and size.
     * @return true if metadata was successfully read, false otherwise (e.g., permission denied).
     */
    bool readFileInfo();

    /**
     * @brief Sets the delete flag for this file.
     * @param flag true to mark for removal, false otherwise.
     */
    void set_remove_unique_flag(bool flag);

    /**
     * @brief Checks if the file is marked for removal.
     * @return true if marked, false otherwise.
     */
    bool remove_unique_flag() const;

    /**
     * @brief Returns the file size in bytes.
     * @return File size as `filesizetype`.
     */
    filesizetype size() const;

    /**
     * @brief Returns the original path of the file.
     * @return Path object referencing the full file path.
     */
    const std::filesystem::path& path() const;

    /**
     * @brief Checks whether the file is a regular file.
     * @return true if it's a regular file, false otherwise.
     */
    bool isRegularFile() const;

    /**
     * @brief Checks whether the path points to a directory.
     * @return true if it's a directory, false otherwise.
     */
    bool isDirectory() const;

private:
    std::filesystem::path m_path;     ///< Full file or directory path.
    filesizetype m_size = 0;          ///< File size in bytes (0 if not a regular file).
    bool m_is_regular = false;        ///< True if path is a regular file.
    bool m_is_directory = false;      ///< True if path is a directory.
    bool m_remove_unique_flag = false;        ///< True if file should be removed during cleanup.

};

#endif // FILEINFO_HPP
