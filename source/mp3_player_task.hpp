#pragma once

#include "L3_Application/task.hpp"

#include "rtos/queue.hpp"
#include "drivers/mp3_decoder.hpp"

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
      : Task("MP3_Player_Task", sjsu::rtos::Priority::kLow), decoder_(decoder)
  {
  }

  bool Setup() override
  {
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

 private:
  void FetchSongs()
  {
    // TODO: implement
  }

  inline static constexpr size_t kStreamQueueLength = 1024;

  const Mp3Decoder & decoder_;
  rtos::Queue<uint8_t, kStreamQueueLength> stream_queue_;
};

class FetchTask final : public sjsu::rtos::Task<256>
{
 public:
  bool Run() override
  {
    return true;
  }

 private:
};
