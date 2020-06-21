#pragma once

#include "FreeRTOS.h"
#include "queue.h"

namespace rtos
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
    queue_handle_ = xQueueCreateStatic(
        kQueueSize, sizeof(T), queue_storage_area_, &static_queue_buffer_);
  }

  QueueHandle_t GetHandle()
  {
    return queue_handle_;
  }

  BaseType_t Send(T * item, TickType_t timeout)
  {
    return xQueueSend(queue_handle_, item, timeout);
  }

  BaseType_t SendToFront(T * item, TickType_t timeout)
  {
    return xQueueSendToFront(queue_handle_, item, timeout);
  }

  BaseType_t Receive(void * location, TickType_t timeout)
  {
    return xQueueReceive(queue_handle_, location, timeout);
  }

  BaseType_t Reset()
  {
    return xQueueReset(queue_handle_);
  }

 private:
  T queue_storage_area_[kQueueSize];
  StaticQueue_t static_queue_buffer_;
  QueueHandle_t queue_handle_;
};
}  // namespace rtos
