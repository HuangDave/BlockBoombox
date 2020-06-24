#pragma once

#include <string.h>

#include "L3_Application/task.hpp"

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
  virtual void Pause() = 0;
  virtual void Play()  = 0;
};

class Mp3PlayerTask final : public sjsu::rtos::Task<512>,
                            public virtual Mp3PlayerInterface
{
 public:
  explicit Mp3PlayerTask(Mp3Decoder & decoder)
      : Task("MP3_Player_Task", sjsu::rtos::Priority::kLow),
        decoder_(decoder),
        song_list_count_(0)
  {
  }

  bool Setup() override
  {
    memset(song_list_, '\0',
           sizeof(char) * kMaxSongListCount * kMaxSongPathLength);
    return true;
  }

  bool Run() override
  {
    return true;
  }

  void Pause() override
  {
    decoder_.PausePlayback();
  }

  void Play() override
  {
    decoder_.ResumePlayback();
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
        strcpy(song_list_[song_list_count_++], fno.fname);
      }

      // TODO: should track position and fetch more when needed instead of
      //       just stopping
      if (song_list_count_ < kMaxSongListCount)
      {
        break;
      }
      res = f_findnext(&dir, &fno);
    }
    f_closedir(&dir);

    sjsu::LogDebug("Current Song List:");
    for (size_t i = 0; i < kMaxSongListCount; i++)
    {
      if (song_list_[i][0] != '\0')
      {
        sjsu::LogDebug("%s", song_list_[i]);
      }
    }
    return res;
  }

 private:
  /// TODO: using max song count of 28 for now, should increase the number of
  ///       paths from 28 to ??
  static constexpr size_t kMaxSongListCount  = 28;
  static constexpr size_t kMaxSongPathLength = 256;

  const Mp3Decoder & decoder_;
  char song_list_[kMaxSongListCount][kMaxSongPathLength];
  size_t song_list_count_;
};
