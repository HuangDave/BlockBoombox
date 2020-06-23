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
    FATFS fat_fs;
    if (f_mount(&fat_fs, "", 0) != 0)
    {
      sjsu::LogError("Failed to mount SD Card");
      return false;
    }
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
  FRESULT FetchSongs()
  {
    static FILINFO fno;
    FRESULT res;
    DIR dir;

    res = f_findfirst(&dir, &fno, "", "*.mp3");
    while (res == FR_OK && fno.fname[0])
    {
      if (fno.fname[0] != '.')
      {
        printf("%s\n", fno.fname);
      }
      res = f_findnext(&dir, &fno);
    }

    f_closedir(&dir);

    return res;
  }

  inline static constexpr size_t kStreamQueueLength = 1024;

  const Mp3Decoder & decoder_;
  rtos::Queue<uint8_t, kStreamQueueLength> stream_queue_;
};
