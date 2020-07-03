#pragma once

#include "FreeRTOS.h"
#include "queue.h"

namespace freertos
{
template <typename T>
class QueueInterface
{
 public:
  // virtual const char * GetName() const                                      = 0;
  virtual BaseType_t Send(T item, TickType_t timeout = portMAX_DELAY) const = 0;
  // virtual BaseType_t SendToFront(T item,
  //                                TickType_t timeout = portMAX_DELAY) const  = 0;
  virtual BaseType_t Receive(void * buffer,
                             TickType_t timeout = portMAX_DELAY) const      = 0;
  // virtual size_t GetNumberOfItems() const                                   = 0;
  // virtual size_t GetAvailableSpace() const                                  = 0;
  // virtual void Reset() const                                                = 0;
  // virtual void Delete() const                                               = 0;
};

/// Wrapper class for FreeRTOS statically allocated queue.
///
/// @see https://www.freertos.org/xQueueCreateStatic.html
///
/// @tparam T The item type.
/// @tparam kQueueSize The maximum number of items that can be queued.
template <typename T, size_t kQueueSize>
class Queue : public QueueInterface<T>
{
 public:
  /// Create the queue using xQueueCreateStatic().
  ///
  /// @param name Optional name used for identifying the queue.
  Queue(const char * name = nullptr)
  {
    handle_ =
        xQueueCreateStatic(kQueueSize, sizeof(T), storage_area_, &buffer_);
    // if (name != nullptr)
    // {
    //   vQueueAddToRegistry(handle_, name);
    // }
  }

  /// @returns The name used to identify the queue.
  /// @returns nullptr if no name was specified when creating the queue.
  // const char * GetName() const override
  // {
  //   return pcQueueGetName(handle_);
  // }

  /// Queue an item to the back of the queue.
  ///
  /// @param item Pointer to the item to be copied to the queue.
  /// @param timout The amount of time in ms to block and wait to queue the
  ///               item.
  BaseType_t Send(T item, TickType_t timeout = portMAX_DELAY) const override
  {
    return xQueueSend(handle_, &item, timeout);
  }

  /// Queue the item to the front of the queue.
  ///
  /// @param item Pointer to the item to be copied to the queue.
  /// @param timout The amount of time in ms to block and wait to queue the
  ///               item(s).
  // BaseType_t SendToFront(T item,
  //                        TickType_t timeout = portMAX_DELAY) const override
  // {
  //   return xQueueSendToFront(handle_, &item, timeout);
  // }

  /// Copy the item(s) from the front of the queue to the target location.
  ///
  /// @param buffer The address of the buffer to hold the copied item.
  /// @param item Pointer to the item to be copied to the queue.
  /// @param timout The amount of time in ms to block and wait to queue the
  ///               item.
  BaseType_t Receive(void * buffer,
                     TickType_t timeout = portMAX_DELAY) const override
  {
    return xQueueReceive(handle_, buffer, timeout);
  }

  /// @returns The number of items currently in the queue.
  // size_t GetNumberOfItems() const override
  // {
  //   return uxQueueMessagesWaiting(handle_);
  // }

  /// @returns The number of available space in the queue.
  // size_t GetAvailableSpace() const override
  // {
  //   return uxQueueSpacesAvailable(handle_);
  // }

  /// Empty the queue.
  // void Reset() const override
  // {
  //   xQueueReset(handle_);
  // }

  /// Delete the queue and frees the memory allocated to be used by the queue.
  // void Delete() const override
  // {
  //   vQueueUnregisterQueue(handle_);
  //   vQueueDelete(handle_);
  // }

 private:
  /// The buffer used to store the queued items.
  uint8_t storage_area_[kQueueSize * sizeof(T)];
  /// The buffer for the queue data structure.
  StaticQueue_t buffer_;
  /// The handle used for managing the queue.
  QueueHandle_t handle_;
};
}  // namespace freertos
