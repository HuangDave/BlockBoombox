#pragma once

#include <string.h>

#include "L3_Application/task_scheduler.hpp"

#include "drivers/mp3_decoder.hpp"
#include "utility/mp3_file.hpp"
#include "utility/queue.hpp"

namespace
{
constexpr size_t kSongQueueLength = 2;
freertos::Queue<mp3::Mp3File *, kSongQueueLength> song_queue;

constexpr size_t kStreamQueueLength = 1024;
freertos::Queue<uint8_t, kStreamQueueLength> stream_queue;
}  // namespace

class Mp3PlayerInterface
{
 public:
  virtual void Pause()              = 0;
  virtual void Play(uint32_t index) = 0;
};

class Mp3PlayerTask final : public sjsu::rtos::Task<1024>,
                            public virtual Mp3PlayerInterface
{
 public:
  explicit Mp3PlayerTask(Mp3Decoder & decoder)
      : Task("MP3_Player_Task", sjsu::rtos::Priority::kLow),
        decoder_(decoder),
        song_list_count_(0)
  {
  }

  bool Run() override
  {
    return true;
  }

  void Pause() override
  {
    decoder_.PausePlayback();
  }

  void Play(uint32_t index) override
  {
    constexpr size_t kBufferSize = 2048;
    mp3::Mp3File & song          = song_list_[index];

    decoder_.EnablePlayback();

    FIL fil;
    printf("Playing: %s\n", song.GetFilePath());

    f_open(&fil, song.GetFilePath(), FA_READ);

    for (size_t i = 0; i < song.GetFileSize() / kBufferSize; i++)
    {
      uint8_t data[kBufferSize];
      UINT bytes_read = 0;
      // f_lseek(&fil, i * kBufferSize);
      f_read(&fil, data, kBufferSize, &bytes_read);

      decoder_.Buffer(data, bytes_read);
      // printf("Bytes read: %d\n", bytes_read);
    }

    f_close(&fil);
  }

  FRESULT FetchSongs()
  {
    static FILINFO fno;
    FRESULT res;
    DIR dir;

    song_list_count_ = 0;
    res              = f_findfirst(&dir, &fno, "", "*.mp3");
    while (res == FR_OK && fno.fname[0])
    {
      if (fno.fname[0] != '.')
      {
        sjsu::LogDebug("Adding: %s\n", fno.fname);
        mp3::Mp3File mp3_file(fno.fname, fno.fsize);
        song_list_[song_list_count_++] = mp3_file;
      }

      // TODO: should track position and fetch more when needed instead of
      //       just stopping
      if (song_list_count_ >= kMaxSongListCount)
      {
        break;
      }
      res = f_findnext(&dir, &fno);
    }
    f_closedir(&dir);

    // TODO: remove this
    sjsu::LogDebug("Current Song List:");
    for (size_t i = 0; i < song_list_count_; i++)
    {
      sjsu::LogInfo("%s", song_list_[i].GetFilePath());
    }
    return res;
  }

 private:
  /// TODO: using max song count of 28 for now, should increase the number of
  ///       paths from 28 to ??
  static constexpr size_t kMaxSongListCount = 28;

  const Mp3Decoder & decoder_;
  mp3::Mp3File song_list_[kMaxSongListCount];
  size_t song_list_count_;
};
