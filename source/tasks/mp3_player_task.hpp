#pragma once

#include <array>
#include <string.h>

#include "L3_Application/fatfs.hpp"
#include "L3_Application/task_scheduler.hpp"

#include "../drivers/audio_decoder.hpp"
#include "../utility/mp3_file.hpp"
#include "../utility/queue.hpp"

class Mp3Player
{
 public:
  using SongQueue   = freertos::QueueInterface<mp3::Mp3File *>;
  using BufferQueue = freertos::QueueInterface<uint8_t>;

  virtual const AudioDecoder * GetDecoder() const        = 0;
  virtual const SongQueue * GetSongQueue() const         = 0;
  virtual const BufferQueue * GetDataBufferQueue() const = 0;
};

class Mp3PlayerTask final : public sjsu::rtos::Task<128>,
                            public virtual Mp3Player
{
 public:
  static constexpr size_t kSongQueueLength = 2;
  static constexpr size_t kBufferLength    = 256;

  explicit Mp3PlayerTask(AudioDecoder & audio_decoder)
      : Task("Mp3PlayerTask", sjsu::rtos::Priority::kLow),
        audio_decoder_(audio_decoder),
        song_list_count_(0)
  {
  }

  bool Setup() override
  {
    FetchSongs();
    return true;
  }

  bool PreRun() override
  {
    Play(0);

    return true;
  }

  bool Run() override
  {
    return true;
  }

  void Play(uint32_t index)
  {
    auto * song = &song_list_[index];
    song_queue_.Send(song);
  }

  // ---------------------------------------------------------------------------
  //                        Mp3Player Implementation
  // ---------------------------------------------------------------------------

  const AudioDecoder * GetDecoder() const override
  {
    return &audio_decoder_;
  }

  const SongQueue * GetSongQueue() const override
  {
    return &song_queue_;
  }

  const BufferQueue * GetDataBufferQueue() const override
  {
    return &buffer_queue_;
  }

 private:
  FRESULT FetchSongs()
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
        f_unmount("");

    return res;
  }

  /// TODO: using max song count of 28 for now, should increase the number of
  ///       paths from 28 to ??
  static constexpr size_t kMaxSongListCount = 10;

  const AudioDecoder & audio_decoder_;
  std::array<mp3::Mp3File, kMaxSongListCount> song_list_;
  size_t song_list_count_;

  freertos::Queue<mp3::Mp3File *, kSongQueueLength> song_queue_;
  freertos::Queue<uint8_t, kBufferLength> buffer_queue_;
};
