#pragma once

#include <array>

#include "L3_Application/fatfs.hpp"
#include "L3_Application/task_scheduler.hpp"

#include "../drivers/audio_decoder.hpp"
#include "../utility/mp3_file.hpp"

class Mp3Player
{
 public:
  virtual const AudioDecoder & GetDecoder() const  = 0;
  virtual QueueHandle_t GetSongQueue() const       = 0;
  virtual QueueHandle_t GetDataBufferQueue() const = 0;
};

class Mp3PlayerTask final : public sjsu::rtos::Task<256>,
                            public virtual Mp3Player
{
 public:
  static constexpr size_t kSongQueueLength = 2;
  static constexpr size_t kBufferItemCount = 3;
  static constexpr size_t kBufferLength    = 1024;

  explicit Mp3PlayerTask(AudioDecoder & audio_decoder)
      : Task("Mp3PlayerTask", sjsu::rtos::Priority::kLow),
        audio_decoder_(audio_decoder),
        song_list_count_(0)
  {
    song_queue_ = xQueueCreate(kSongQueueLength, sizeof(mp3::Mp3File));
    buffer_queue_ =
        xQueueCreate(kBufferItemCount, kBufferLength * sizeof(uint8_t));
  }

  // ---------------------------------------------------------------------------
  //                           Task Implementation
  // ---------------------------------------------------------------------------

  bool Setup() override
  {
    FetchSongs();
    Play(0);
    return true;
  }

  bool Run() override
  {
    return true;
  }

  void Play(uint32_t index)
  {
    xQueueSend(song_queue_, &song_list_[index], portMAX_DELAY);
  }

  // ---------------------------------------------------------------------------
  //                         Mp3Player Implementation
  // ---------------------------------------------------------------------------

  const AudioDecoder & GetDecoder() const override
  {
    return audio_decoder_;
  }

  QueueHandle_t GetSongQueue() const override
  {
    return song_queue_;
  }

  QueueHandle_t GetDataBufferQueue() const override
  {
    return buffer_queue_;
  }

 private:
  void FetchSongs()
  {
    FILINFO fno;
    FRESULT res;
    DIR dir;

    song_list_count_ = 0;
    res              = f_findfirst(&dir, &fno, "", "*.mp3");
    while (res == FR_OK && fno.fname[0])
    {
      if (fno.fname[0] != '.')
      {
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
  }

  /// TODO: using max song count of 28 for now, should increase the number of
  ///       paths from 28 to ??
  static constexpr size_t kMaxSongListCount = 5;

  const AudioDecoder & audio_decoder_;
  std::array<mp3::Mp3File, kMaxSongListCount> song_list_;
  size_t song_list_count_;

  QueueHandle_t song_queue_;
  QueueHandle_t buffer_queue_;
};
