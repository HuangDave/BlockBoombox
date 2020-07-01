#pragma once

#include <cstddef>
#include <cstdint>

/// An interface containing controls for an audio decoder.
class AudioDecoder
{
 public:
  /// Initialize the device for use.
  virtual void Initialize() const = 0;

  /// Reset the device.
  virtual void Reset() const = 0;

  /// Enable the device to be ready to decode audio data.
  virtual void Enable() const = 0;

  /// Pause audio decoding.
  virtual void Pause() const = 0;

  /// Attempts to resume audio decoding.
  virtual void Resume() const = 0;

  /// Reset decode time back to 00:00.
  virtual void ClearDecodeTime() const = 0;

  /// Buffer audio data for decoding.
  ///
  /// @param data Pointer to the array containing the data bytes to buffer.
  /// @param length The number of data bytes in the array.
  virtual void Buffer(const uint8_t * data, size_t length) const = 0;

  /// @param percentage Volume percentage ranging from 0.0 to 1.0, where 1.0 is
  ///                   100 percent.
  virtual void SetVolume(float percentage) const = 0;
};
