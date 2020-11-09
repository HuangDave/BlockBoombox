#pragma once

#include <array>
#include <chrono>
#include <cstring>
#include <span>
#include <string_view>

#include "L1_Peripheral/gpio.hpp"
#include "L1_Peripheral/uart.hpp"
#include "module.hpp"
#include "utility/log.hpp"

//namespace sjsu
//{
// class SerialBluetooth : public sjsu::Module
// {
//  public:
//   inline static constexpr std::chrono::nanoseconds kDefaultTimeout = 20ms;

//   inline static constexpr uint32_t kDefaultBaudRate   = 9'600;
//   inline static constexpr uint32_t kDefaultAtBaudRate = 38'400;

//   explicit SerialBluetooth(sjsu::Uart & uart_port,
//                            sjsu::Gpio & state_pin,
//                            sjsu::Gpio & key_pin,
//                            bool use_cr_nl)
//       : uart_port_(uart_port),
//         state_pin_(state_pin),
//         key_pin_(key_pin),
//         use_cr_nl_(use_cr_nl)
//   {
//   }

//   virtual void ModuleInitialize() override
//   {
//     state_pin_.SetAsInput();
//     key_pin_.SetAsOutput();
//     key_pin_.SetHigh();
//     // uart_port_.Initialize(kDefaultAtBaudRate);
//   }

//   virtual void ModuleEnable([[maybe_unused]] bool enable) override {}

//   /// Sends an AT command to the device.
//   ///
//   /// @param command
//   /// @param timeout
//   void SendCommand(
//       const std::string_view command,
//       std::span<char> receive,
//       const std::chrono::nanoseconds timeout = kDefaultTimeout) const
//   {
//     /*
//     for (const char c : command)
//     {
//       uart_port_.Write(c);
//     }
//     if (use_cr_nl_)
//     {
//       uart_port_.Write('\r');
//       uart_port_.Write('\n');
//     }

//     memset(receive.data(), '\0', receive.size());
//     uart_port_.Read(receive.data(), receive.size(), timeout);
//     */
//   }

//  private:
//   sjsu::Uart & uart_port_;
//   sjsu::Gpio & state_pin_;
//   sjsu::Gpio & key_pin_;
//   const bool use_cr_nl_;
// };
//}  // namespace sjsu

/*
class SerialBluetooth
{
 public:
  inline static constexpr std::chrono::nanoseconds kDefaultTimeout = 20ms;

  inline static constexpr uint32_t kDefaultBaudRate   = 9'600;
  inline static constexpr uint32_t kDefaultAtBaudRate = 38'400;

  explicit SerialBluetooth(sjsu::Uart & uart_port,
                           sjsu::Gpio & state_pin,
                           sjsu::Gpio & key_pin,
                           bool use_cr_nl)
      : uart_port_(uart_port),
        state_pin_(state_pin),
        key_pin_(key_pin),
        use_cr_nl_(use_cr_nl)
  {
  }

  virtual void Initialize() const
  {
    state_pin_.SetAsInput();
    key_pin_.SetAsOutput();
    key_pin_.SetHigh();

    uart_port_.Initialize(kDefaultAtBaudRate);
  }

  /// Sends an AT command to the device.
  ///
  /// @param command
  /// @param timeout
  void SendCommand(
      const std::string_view command,
      std::span<char> receive,
      const std::chrono::nanoseconds timeout = kDefaultTimeout) const
  {
    for (const char c : command)
    {
      uart_port_.Write(c);
    }
    if (use_cr_nl_)
    {
      uart_port_.Write('\r');
      uart_port_.Write('\n');
    }

    memset(receive.data(), '\0', receive.size());
    uart_port_.Read(receive.data(), receive.size(), timeout);
  }

  void SetBaudRate(uint32_t baud) {}

  int GetBaudRate()
  {
    char buffer[10];
    SendCommand("AT+BAUD", buffer);
  }

 private:
  const sjsu::Uart & uart_port_;
  const sjsu::Gpio & state_pin_;
  const sjsu::Gpio & key_pin_;
  const bool use_cr_nl_;
};
*/