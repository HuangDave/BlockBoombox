#pragma once

#include <cstdint>

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
