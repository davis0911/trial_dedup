#include "FileInfo.hpp"
#include <system_error>  // for std::error_code
#include <sys/stat.h>

/**
 * @brief Reads the size of the file at the stored path.
 *
 * This function uses std::filesystem to read the size of the file pointed to by `m_path`.
 * It stores the result in `m_size`. If an error occurs during size retrieval,
 * the function returns false.
 *
 * @return true if the file size was read successfully, false otherwise.
 */
bool FileInfo::readFileSize() {
    std::error_code ec;
    m_size = std::filesystem::file_size(m_path, ec);
    if (ec) return false;
    return true;
}

/**
 * @brief Reads the first few bytes of the file and stores them in `m_somebytes`.
 *
 * This function opens the file in binary mode and reads up to fixed size of bytes returned by getBufferSize().
 * into the internal buffer `m_somebytes`. The buffer is initialized with null characters.
 *
 * @return 0 if bytes were successfully read, -1 if the file could not be opened.
 */
int FileInfo::readFirstBytes() {
  m_somebytes.fill('\0');
  std::ifstream file(m_path, std::ios::in | std::ios::binary);
  if (!file.is_open()) return -1;

  file.read(m_somebytes.data(), getBufferSize());
  return 0;
}
