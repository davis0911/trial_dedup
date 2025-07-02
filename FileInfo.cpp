#include "FileInfo.hpp"
#include <system_error>  // for std::error_code

/**
 * @brief Constructs a FileInfo object from a filesystem path.
 * @param path Full path to a file or directory.
 */
FileInfo::FileInfo(std::filesystem::path path)
    : m_path(std::move(path)) {}

/**
 * @brief Reads metadata about the file at the stored path.
 *
 * Uses `symlink_status()` to check file type (regular file, directory),
 * and `file_size()` to determine the size if it's a regular file.
 * Errors such as permission denied are handled via `std::error_code`.
 * 
 * @return true if metadata was successfully read, false otherwise.
 */
bool FileInfo::readFileInfo() {
    std::error_code ec;

    // Get file type information without following symlinks
    auto status = std::filesystem::symlink_status(m_path, ec);
    if (ec) return false;

    // Set file type flags
    m_is_regular = std::filesystem::is_regular_file(status);
    m_is_directory = std::filesystem::is_directory(status);

    // If it's a regular file, get its size
    if (m_is_regular) {
        m_size = std::filesystem::file_size(m_path, ec);
        if (ec) return false;
    } else {
        m_size = 0;
    }

    return true;
}

/**
 * @brief Gets the size of the file.
 * @return File size in bytes, or 0 if not a regular file.
 */
FileInfo::filesizetype FileInfo::size() const noexcept {
    return m_size;
}

/**
 * @brief Gets the original filesystem path of the file.
 * @return Reference to the stored path.
 */
const std::filesystem::path& FileInfo::path() const noexcept {
    return m_path;
}

/**
 * @brief Checks if the file is a regular file.
 * @return true if regular, false otherwise.
 */
bool FileInfo::isRegularFile() const noexcept {
    return m_is_regular;
}

/**
 * @brief Checks if the file is a directory.
 * @return true if directory, false otherwise.
 */
bool FileInfo::isDirectory() const noexcept {
    return m_is_directory;
}
