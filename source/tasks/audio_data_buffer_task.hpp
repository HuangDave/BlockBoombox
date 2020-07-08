#pragma once

#include "L3_Application/fatfs.hpp"
#include "L3_Application/task_scheduler.hpp"

#include "../drivers/audio_decoder.hpp"
#include "../utility/mp3_file.hpp"
#include "mp3_player_task.hpp"

template <size_t kBufferLength>
class AudioDataBufferTask final : public sjsu::rtos::Task<4 * 1024>
{
 public:
  explicit AudioDataBufferTask(Mp3Player & player)
      : Task("AudioDataBufferTask", sjsu::rtos::Priority::kLow),
        decoder_(player.GetDecoder()),
        song_queue_(player.GetSongQueue()),
        buffer_queue_(player.GetDataBufferQueue())
  {
    buffer =
        reinterpret_cast<uint8_t *>(malloc(kBufferLength * sizeof(uint8_t)));
  }

  bool Run() override
  {
    mp3::Mp3File song;

    if (xQueueReceive(song_queue_, &song, portMAX_DELAY))
    {
      decoder_.Enable();

      FIL fil;
      UINT bytes_read = 0;

      for (size_t i = 0; i < song.GetFileSize() / kBufferLength; i++)
      {
        if (FR_OK == f_open(&fil, song.GetFilePath(), FA_READ))
        {
          f_lseek(&fil, i * kBufferLength);
          f_read(&fil, buffer, kBufferLength, &bytes_read);
          f_close(&fil);
        }

        xQueueSend(buffer_queue_, buffer, portMAX_DELAY);
        vTaskDelay(15);
      }
    }

    return true;
  }

 private:
  const AudioDecoder & decoder_;
  const QueueHandle_t song_queue_;
  const QueueHandle_t buffer_queue_;

  uint8_t * buffer;
};

template <size_t kBufferLength>
class AudioDataDecodeTask final : public sjsu::rtos::Task<512>
{
 public:
  AudioDataDecodeTask(Mp3Player & player)
      : Task("AudioDataDecodeTask", sjsu::rtos::Priority::kLow),
        decoder_(player.GetDecoder()),
        buffer_queue_(player.GetDataBufferQueue())
  {
    buffer =
        reinterpret_cast<uint8_t *>(malloc(kBufferLength * sizeof(uint8_t)));
  }

  bool Run() override
  {
    if (xQueueReceive(buffer_queue_, buffer, portMAX_DELAY))
    {
      decoder_.Buffer(buffer, kBufferLength);
    }
    return true;
  }

 private:
  const AudioDecoder & decoder_;
  const QueueHandle_t buffer_queue_;

  uint8_t * buffer;
};
