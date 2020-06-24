#pragma once

#include <cstddef>
#include <cstdint>

namespace mp3
{
/// ID3v1 metadata of a MP3 file.
///
/// @see https://en.wikipedia.org/wiki/ID3
struct Idv3Tag_t
{
  /// The ID3v1 tag size in bytes.
  static constexpr size_t kSize = 128;

  ///  3 bytes - should contain "TAG"
  char header[3];
  /// 30 bytes - 30 characters of title
  char title[30];
  /// 30 bytes - 30 characters of artist name
  char artist[30];
  /// 30 bytes - 30 characters of album name
  char album[30];
  ///  4 bytes - 4 digit year
  uint32_t year;
  /// 30 bytes - Comment
  char comment[30];
  ///  1 byte  - Index of the track's genre (0-255)
  uint8_t genre;
};

/// An object used to store a MP3 file's file path, size, and metadata.
class Mp3File
{
 public:
  /// @param file_path The MP3 file's file path.
  /// @param file_size The size of the MP3 file in bytes.
  constexpr Mp3File(const char * file_path, size_t file_size)
      : file_path_(file_path), file_size_(file_size)
  {
  }

  /// @returns The MP3 file's file path.
  const char * GetFilePath() const
  {
    return file_path_;
  }

  /// @returns The size of the MP3 file in bytes.
  size_t GetFileSize() const
  {
    return file_size_;
  }

  /// Fetches the MP3 file's metadata if necessary and returns a pointer
  /// reference to the metadata.
  ///
  /// @tparam MetadataType The metadata/tag type.
  /// @param force_fetch True if the metadata should or needs to be retrieved.
  ///
  /// @returns Pointer reference of the metadata as teh specified type.
  template <typename MetadataType>
  const MetadataType * GetMetadata(bool force_fetch = false) const
  {
    return reinterpret_cast<T *>(meta_data_);
  }

 private:
  /// The MP3 file path.
  const char * file_path_;
  /// Size of the MP3 file in bytes.
  size_t file_size_;
  /// Buffer to hold the retrieved metadata.
  uint8_t meta_data_[256] = {};
};
}  // namespace mp3
