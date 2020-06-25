#pragma once

#include <cstddef>
#include <cstdint>

namespace mp3
{
/// ID3v1 metadata of a MP3 file.
///
/// @see https://en.wikipedia.org/wiki/ID3
struct Id3v1_t
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

/// @see https://id3.org/id3v2.3.0?highlight=(ID3%20tag%20version%202.3.0)
/// @see https://id3.org/id3v2.4.0-structure?highlight=(id3v2.4.0-structure.txt)
struct Id3v2
{
  /// @returns The formatted tag size in bytes.
  static uint32_t GetSize(uint8_t size[4])
  {
    return sjsu::bit::Value()
        .Insert(size[0], sjsu::bit::MaskFromRange(24, 30))
        .Insert(size[1], sjsu::bit::MaskFromRange(16, 22))
        .Insert(size[2], sjsu::bit::MaskFromRange(8, 14))
        .Insert(size[3], sjsu::bit::MaskFromRange(0, 6));
  }

  struct TagHeader_t
  {
    /// The first 3 bytes should always be "ID3".
    uint8_t identifier[3];
    /// 2 byte version with high byte representing the major version and the low
    /// byte the revision number.
    ///
    /// @note Major versions are not backwards compatible. Revision numbers are
    ///       backwards compatible.
    uint8_t major_version;
    uint8_t revision_number;
    uint8_t flags;
    /// Size of the tag in bytes.
    ///
    /// @note The 7th bit for each byte is always cleared.
    /// @note size[0] is the most significant byte.
    uint8_t size[4];
  };

  struct FrameHeader_t
  {
    char identifier[4];
    uint32_t size;
    uint16_t flags;
  };
};

/// An object used to store a MP3 file's file path, size, and metadata.
class Mp3File
{
 public:
  /// @param file_path The MP3 file's file path.
  /// @param file_size The size of the MP3 file in bytes.
  constexpr Mp3File(const char * file_path = nullptr, size_t file_size = 0)
      : file_size_(file_size)
  {
    strcpy(file_path_, file_path);
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
  const MetadataType * GetMetadata(bool force_fetch = false)
  {
    if (force_fetch)
    {
      FIL file;
      size_t read_count = 0;

      if (f_open(&file, file_path_, FA_READ) == FR_OK)
      {
        // ID3v1 tag is at the last 128 bytes
        // f_lseek(&file, file_size_ - 256);
        // f_read(&file, meta_data_, sizeof meta_data_, &read_count);

        // read and check header
        f_read(&file, meta_data_, sizeof(Id3v2::TagHeader_t), &read_count);
        auto * header = reinterpret_cast<Id3v2::TagHeader_t *>(meta_data_);
        printf("ID: %c%c%c\n", header->identifier[0], header->identifier[1],
               header->identifier[2]);
        printf("version: 2.%d.%d\n", header->major_version,
               header->revision_number);
        printf("size: %ld bytes\n", Id3v2::GetSize(header->size));
      }
      f_close(&file);
    }

    return reinterpret_cast<MetadataType *>(meta_data_);
  }

 private:
  /// The MP3 file path.
  char file_path_[256] = { '\0' };
  /// Size of the MP3 file in bytes.
  size_t file_size_;
  /// Buffer to hold the retrieved metadata.
  BYTE meta_data_[256] = {};
};
}  // namespace mp3
