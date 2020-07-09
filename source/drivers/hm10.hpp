#pragma once

#include "L1_Peripheral/gpio.hpp"
#include "L1_Peripheral/uart.hpp"
#include "utility/status.hpp"

namespace bluetooth
{
class BluetoothDevice
{
 public:
};

/// HM-10, also can be used for other CC2541-based bluetooth module.
class Hm10
{
 public:
  explicit Hm10(sjsu::Uart & uart, sjsu::Gpio & chip_enable, sjsu::Gpio & state)
      : uart_(uart), chip_enable_(chip_enable), state_(state)
  {
  }

  sjsu::Status Initialize() const
  {
    chip_enable_.SetAsOutput();
    chip_enable_.SetHigh();

    constexpr uint32_t kBaudRate = 115'200;
    return uart_.Initialize(kBaudRate);
  }

  void Enable()
  {
    chip_enable_.SetLow();
  }

  void Disable()
  {
    chip_enable_.SetHigh();
  }

  void Send(const char * command)
  {
    for (size_t i = 0; i < strlen(command); i++)
    {
      uart_.Write(command[i]);
    }
  }

 private:
  const sjsu::Uart & uart_;
  const sjsu::Gpio & chip_enable_;
  const sjsu::Gpio & state_;
};
}  // namespace bluetooth
