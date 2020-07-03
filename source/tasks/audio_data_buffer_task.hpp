#pragma once

#include "L3_Application/fatfs.hpp"
#include "L3_Application/task_scheduler.hpp"

#include "../drivers/audio_decoder.hpp"
#include "../utility/mp3_file.hpp"
#include "mp3_player_task.hpp"

template <size_t kBufferLength>
class AudioDataBufferTask final : public sjsu::rtos::Task<1024 * 2>
{
 public:
  explicit AudioDataBufferTask(Mp3Player & player)
      : Task("AudioDataBufferTask", sjsu::rtos::Priority::kLow), player_(player)
  {
  }

  bool Run() override
  {
    mp3::Mp3File * song = nullptr;

    const auto & kDecoder   = *player_.GetDecoder();
    const auto & kSongQueue = *player_.GetSongQueue();
    // const auto & kBufferQueue = player_.GetDataBufferQueue();

    if (kSongQueue.Receive(&song) == pdTRUE)
    {
      kDecoder.Enable();

      for (size_t i = 0; i < song->GetFileSize() / kBufferLength; i++)
      {
        uint8_t buffer[kBufferLength];
        FIL fil;
        UINT bytes_read = 0;
        //     // if (!decoder_.IsPlaying())
        //     // {
        //     //   // TODO: player is paused, task should sleep
        //     // }
        //     // else if (true)  // TODO: change check
        //     // {
        //     //   // TODO: different song selected or play next song
        //     //   //       1. should stop buffering data
        //     //   //       2. clear the buffer queue
        //     //   //       3. break loop
        //     // }

        // sjsu::LogInfo("%s", song->GetFilePath());

taskENTER_CRITICAL();

        FATFS fat_fs;
        f_mount(&fat_fs, "", 0);

        if (FR_OK ==
            f_open(&fil, song->GetFilePath(), FA_OPEN_EXISTING | FA_READ))
        {
          f_lseek(&fil, i * kBufferLength);
          f_read(&fil, buffer, kBufferLength, &bytes_read);
          f_close(&fil);
        }
        f_unmount("");

taskEXIT_CRITICAL();
        // for (size_t j = 0; j < bytes_read; j++)
        // {
        //   kDecoder.Buffer(buffer, bytes_read);
        //   // kBufferQueue.Send(buffer[j]);
        // }
        // vTaskDelay(10);
      }
    }

    return true;
  }

 private:
  const Mp3Player & player_;
};

// template <size_t kBufferLength>
// class AudioDataDecodeTask final : public sjsu::rtos::Task<256>
// {
//  public:
//   using BufferQueue = freertos::Queue<uint8_t *, kBufferLength>;

//   AudioDataDecodeTask(AudioDecoder & decoder, BufferQueue & buffer_queue)
//       : Task("AudioDataDecodeTask", sjsu::rtos::Priority::kLow),
//         decoder_(decoder),
//         buffer_queue_(buffer_queue)
//   {
//   }

//   bool Run() override
//   {
//     uint8_t buffer[kBufferLength];

//     if (buffer_queue_.Receive(&buffer) == pdTRUE)
//     {
//       decoder_.Buffer(buffer, kBufferLength);
//     }

//     return true;
//   }

//  private:
//   const AudioDecoder & decoder_;
//   const BufferQueue & buffer_queue_;
// };
