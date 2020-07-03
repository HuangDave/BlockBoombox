#pragma once

#include "L3_Application/task_scheduler.hpp"

#include "mp3_player_task.hpp"
#include "utility/semaphore.hpp"
#include "utility/queue.hpp"

class ApplicationController
{
 public:
  explicit ApplicationController(Mp3PlayerTask & mp3_player_task)
      : mp3_player_task_(mp3_player_task)
  {
  }

 private:
  static constexpr size_t kSongQueueLength  = 2;
  static constexpr size_t kSongBufferLength = 1024;

  freertos::Queue<mp3::Mp3File *, kSongQueueLength> song_queue;
  freertos::Queue<uint8_t, kSongBufferLength> stream_queue;

  Mp3PlayerTask & mp3_player_task_;
};
