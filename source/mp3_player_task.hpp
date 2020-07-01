#pragma once

#include <string.h>

#include "L3_Application/task_scheduler.hpp"

#include "drivers/audio_decoder.hpp"
#include "utility/mp3_file.hpp"
#include "utility/queue.hpp"

namespace
{
constexpr size_t kSongQueueLength = 2;
freertos::Queue<mp3::Mp3File *, kSongQueueLength> song_queue;

constexpr size_t kStreamQueueLength = 1024;
freertos::Queue<uint8_t, kStreamQueueLength> stream_queue;
}  // namespace

class Mp3PlayerTask final : public sjsu::rtos::Task<512>
{
 public:
  explicit Mp3PlayerTask(AudioDecoder & audio_decoder)
      : Task("MP3_Player_Task", sjsu::rtos::Priority::kLow),
        audio_decoder_(audio_decoder),
        song_list_count_(0)
  {
  }

  bool Run() override
  {
    return true;
  }

  void Play(uint32_t index)
  {
    constexpr size_t kBufferSize = 2048;
    mp3::Mp3File & song          = song_list_[index];
    uint8_t data[kBufferSize];
    UINT bytes_read = 0;

    audio_decoder_.Enable();

    FIL fil;
    printf("Playing: %s\n", song.GetFilePath());

    for (size_t i = 0; i < song.GetFileSize() / kBufferSize; i++)
    {
      f_open(&fil, song.GetFilePath(), FA_READ);
      f_lseek(&fil, i * kBufferSize);
      f_read(&fil, data, kBufferSize, &bytes_read);
      f_close(&fil);

      audio_decoder_.Buffer(data, bytes_read);
    }
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

    return res;
  }

 private:
  /// TODO: using max song count of 28 for now, should increase the number of
  ///       paths from 28 to ??
  static constexpr size_t kMaxSongListCount = 28;

  const AudioDecoder & audio_decoder_;
  mp3::Mp3File song_list_[kMaxSongListCount];
  size_t song_list_count_;
};
