#include "FileInfo.hpp"
#include <system_error>  // for std::error_code
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include "Checksum.hpp"
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

void FileInfo::setVideoHashes(){
  cv::VideoCapture cap(m_path.string());
  if(!cap.isOpened()){
    std::cerr<<"Failed to open video file"<<m_path<<"\n";
    return ;
  }

  double totalFrames=cap.get(cv::CAP_PROP_FRAME_COUNT);
  const int numSamples=10;
  for(int i=0; i<numSamples; ++i){
    //starts from 0 to totalFrames-1;
    int frameIndex=(int)((i*totalFrames)/numSamples);
    cap.set(cv::CAP_PROP_POS_FRAMES, frameIndex);
    
    cv::Mat frame;
    if(!cap.read(frame) || frame.empty()){
      //std::cerr<<"Failed to read frame "<<frameIndex<<" frame of "<<m_path<<"\n";
      //If even the first frame could not be read then mark it for removal.
      if(i==0){
        m_remove_unique_flag=true;
      }
      return;
    }

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    uint64_t hashval=Checksum::phashFromMat(gray);
    m_video_hashes.push_back(hashval);
  }

}