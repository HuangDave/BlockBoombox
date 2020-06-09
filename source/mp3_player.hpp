#pragma once

#include <cstddef>
#include <cstdint>

struct Mp3File_t
{
  /// File path.
  const char * path_;
  /// Size of the file in bytes.
  size_t size_;
  /// ID3v1 metadata of a MP3 file.
  ///
  /// @see https://en.wikipedia.org/wiki/ID3
  struct
  {
    char header[3];    ///  3 bytes - should contain "TAG"
    char title[30];    /// 30 bytes - 30 characters of title
    char artist[30];   /// 30 bytes - 30 characters of artist name
    char album[30];    /// 30 bytes - 30 characters of album name
    uint32_t year;     ///  4 bytes - 4 digit year
    char comment[30];  /// 30 bytes - Comment
    uint8_t genre;     ///  1 byte  - Index of the track's genre (0-255)
  } metadata = {};
};

/// An interface containing basic music player controls.
class Mp3Player
{
 public:
  /// @returns true if a song is currently playing.
  virtual bool IsPlaying() = 0;

  /// Queue a song to be played.
  ///
  /// @param song Pointer reference to the song to play.
  virtual void QueueSong(const Mp3File_t * song) = 0;

  /// Pause the music player.
  virtual void PauseSong() = 0;

  /// Resume the music player.
  virtual void ResumeSong() = 0;

  /// Play the previously played song.
  virtual void PlayPreviousSong() = 0;

  /// Play the next queued song.
  virtual void PlayNextSong() = 0;

  /// @param percentage Volume percentage ranging from 0.0 to 1.0, where 1.0 is
  ///                   100 percent.
  virtual void SetVolume(float percentage) = 0;
};
