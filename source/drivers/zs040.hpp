#pragma once

#include <initializer_list>

#include "L1_Peripheral/gpio.hpp"
#include "L1_Peripheral/uart.hpp"
#include "utility/status.hpp"

namespace sjsu
{
namespace bluetooth
{
class BluetoothLowEnergy
{
 public:
  virtual sjsu::Status Initialize() const = 0;
  virtual bool GetState() const           = 0;
  virtual void Disconnect() const         = 0;
};
}  // namespace bluetooth
}  // namespace sjsu

namespace bluetooth
{
/// Driver for the ZS-040 breakout board. The ZS-040 utilizes a MLT-BT05, a
/// CC254x-based, BLE module. The BLE module is capable of communicating with
/// modern BLE 4.0 compatible devices such as Android or iOS devices.
///
/// @note This driver can be used for other CC254x-based bluetooth module.
///
/// @see For ZS-040 v2.0
///      http://www.martyncurrey.com/bluetooth-modules/#HC-06-zs-040-hc01.com-v2.0
/// @see For ZS-040 v2.1
///      http://www.martyncurrey.com/bluetooth-modules/#HC-05-zs-040-hc01.com-v2.1
class Zs040 final : public sjsu::bluetooth::BluetoothLowEnergy
{
 public:
  struct Command  // NOLINT
  {
    /// Check whether the device is interfaced correctly. Should return "OK".
    static constexpr char kVerify[] = "AT";
    /// Get the device's version.
    static constexpr char kVersion[] = "AT+VERSION";

    /// Set/Get the device's mode.
    static constexpr char kRole[] = "AT+ROLE";
    /// Set/Get the device's serial baud rate.
    static constexpr char kSerialBaudRate[] = "AT+BAUD";
    /// Set/Get the device's UUID.
    static constexpr char kUuid[] = "AT+UUID";
  };

  enum class BaudRate : uint8_t
  {
    kB115200 = 0,
    kB57600  = 1,
    kB38400  = 2,
    kB19200  = 3,
    kB9600   = 4,
  };

  /// @note Some modules may have chip_enable and state as a floating pin;
  ///       therefore, the default parameter for these two pins shall be
  ///       inactive GPIOs.
  ///
  /// @param uart The UART peripheral used to communicate with the device.
  /// @param chip_enable The device's chip enable pin. When driven low, the
  ///                    device will disconnect any connected bluetooth devices.
  /// @param state The device's state pin.
  /// @param default_baud_rate The initial default baud rate on reset.
  ///                          depending on the device used.
  /// @param use_cr_nl When true, send carriage return (CR) and new line (NL) at
  ///                  the end of each command.
  explicit Zs040(sjsu::Uart & uart,
                 sjsu::Gpio & chip_enable   = sjsu::GetInactive<sjsu::Gpio>(),
                 sjsu::Gpio & state         = sjsu::GetInactive<sjsu::Gpio>(),
                 uint32_t default_baud_rate = 9'600,
                 bool use_cr_nl             = true)
      : uart_(uart),
        chip_enable_(chip_enable),
        state_(state),
        default_baud_rate_(default_baud_rate),
        use_cr_nl_(use_cr_nl)
  {
  }

  sjsu::Status Initialize() const override
  {
    state_.SetAsInput();
    chip_enable_.SetAsOutput();
    chip_enable_.SetHigh();

    return uart_.Initialize(default_baud_rate_);
  }

  bool GetState() const override
  {
    return state_.Read();
  }

  void Disconnect() const override
  {
    chip_enable_.SetLow();
    // TODO: may need a slight delay here
    chip_enable_.SetHigh();
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  void SetBaudRate(BaudRate baud_rate) const
  {
    SendCommand(Command::kSerialBaudRate, { sjsu::Value(baud_rate) });
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  void Read() const {}

  void SendCommand(const char * command,
                   const std::initializer_list<uint8_t> parameter = {}) const
  {
    for (size_t i = 0; i < strlen(command); i++)
    {
      uart_.Write(command[i]);
    }
    for (auto param : parameter)
    {
      uart_.Write(param);
    }
    if (use_cr_nl_)
    {
      uart_.Write('\r');
      uart_.Write('\n');
    }

    // Flush out uart, by extracting the response but only printing if debug is
    // enabled.
    constexpr size_t kMaxResponseLength = 20;
    char response[kMaxResponseLength]   = { '\0' };
    uint8_t i                           = 0;
    while (uart_.HasData() && i < kMaxResponseLength)
    {
      char c = uart_.Read();
      if (c == '\n')
      {
        break;
      }
      response[i] = c;
      i++;
    }
    sjsu::LogDebug("%s", response);
  }

 private:
  const sjsu::Uart & uart_;
  const sjsu::Gpio & chip_enable_;
  const sjsu::Gpio & state_;
  /// @note The initial baud rate can be either 9'600 bps or 38'400 bps
  ///       depending on the device used.
  const uint32_t default_baud_rate_;
  const bool use_cr_nl_;
};
}  // namespace bluetooth
