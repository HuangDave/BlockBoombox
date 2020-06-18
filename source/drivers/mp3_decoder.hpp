#pragma once

#include <cstddef>
#include <cstdint>

struct Mp3File_t
{
  /// File path.
  const char * path;
  /// Size of the file in bytes.
  size_t size;
  /// ID3v1 metadata of a MP3 file.
  ///
  /// @see https://en.wikipedia.org/wiki/ID3
  struct
  {
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
  } metadata = {};
};

/// An interface containing controls for a MP3 audio decoder.
class Mp3Decoder
{
 public:
  virtual void Initialize() const = 0;

  virtual void Reset() const = 0;

  virtual void EnablePlayback() const = 0;

  /// Pause the music player.
  virtual void PausePlayback() const = 0;

  /// Resume the music player.
  virtual void ResumePlayback() const = 0;

  virtual void Buffer(const uint8_t * data, size_t length) const = 0;

  /// @param percentage Volume percentage ranging from 0.0 to 1.0, where 1.0 is
  ///                   100 percent.
  virtual void SetVolume(float percentage) const = 0;
};
