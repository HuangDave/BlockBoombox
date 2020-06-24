#pragma once

#include "FreeRTOS.h"
#include "queue.h"

namespace freertos
{
/// Wrapper class for FreeRTOS static queue.
///
/// @tparam T
/// @tparam kQueueSize
template <typename T, size_t kQueueSize>
class Queue
{
 public:
  Queue()
  {
    handle_ = xQueueCreateStatic(
        kQueueSize, sizeof(T), storage_area_, &buffer_);
  }

  BaseType_t Send(T * item, TickType_t timeout)
  {
    return xQueueSend(handle_, item, timeout);
  }

  BaseType_t SendToFront(T * item, TickType_t timeout)
  {
    return xQueueSendToFront(handle_, item, timeout);
  }

  BaseType_t Receive(void * location, TickType_t timeout)
  {
    return xQueueReceive(handle_, location, timeout);
  }

  BaseType_t Reset()
  {
    return xQueueReset(handle_);
  }

 private:
  uint8_t storage_area_[kQueueSize * sizeof(T)];
  StaticQueue_t buffer_;
  QueueHandle_t handle_;
};
}  // namespace rtos
