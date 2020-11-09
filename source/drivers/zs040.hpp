#pragma once

#include <span>
#include <string_view>

#include "L1_Peripheral/gpio.hpp"
#include "L1_Peripheral/uart.hpp"
#include "module.hpp"

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
class Zs040 final : public sjsu::Module
{
 public:
  enum class BaudRate : char
  {
    kB115200 = '0',
    kB57600  = '1',
    kB38400  = '2',
    kB19200  = '3',
    kB9600   = '4',
  };

  enum class Role : char
  {
    kSlave  = '0',
    kMaster = '1',
    kSensor = '2',
    kBeacon = '3',
    kWeChat = '4',
  };

  struct Command_t
  {
    const std::string_view instruction;
    // Max response length in bytes not including the including the instruction.
    const std::chrono::milliseconds timeout;

    size_t GetParamPosition() const
    {
      return instruction.size() + 2;
    }
  };

  struct Command  // NOLINT
  {
    static constexpr Command_t kAt         = { "", 10ms };
    static constexpr Command_t kVersion    = { "+VERSION", 40ms };
    static constexpr Command_t kMacAddress = { "+LADDR", 100ms };
    static constexpr Command_t kRole       = { "+ROLE", 25ms };
    static constexpr Command_t kUuid       = { "+UUID", 35ms };
    static constexpr Command_t kBaud       = { "+BAUD", 25ms };
    static constexpr Command_t kDeviceName = { "+NAME", 100ms };
  };

  static constexpr std::chrono::milliseconds kDefaultTimeout = 20ms;

  inline static constexpr uint32_t kDefaultBaudRate = 9'600;
  inline static constexpr uint32_t kAtBaudRate      = 38'400;

  /// @note Some modules may have chip_enable and state as a floating pin;
  ///       therefore, the default parameter for these two pins shall be
  ///       inactive GPIOs.
  ///
  /// @param uart The UART peripheral used to communicate with the device.
  /// @param chip_enable The device's chip enable pin. When driven low, the
  ///                    device will disconnect any connected bluetooth devices.
  /// @param state The device's state pin.
  /// @param use_cr_nl When true, send carriage return (CR) and new line (NL) at
  ///                  the end of each command.
  explicit Zs040(sjsu::Uart & uart,
                 sjsu::Gpio & chip_enable = sjsu::GetInactive<sjsu::Gpio>(),
                 sjsu::Gpio & state       = sjsu::GetInactive<sjsu::Gpio>(),
                 bool use_cr_nl           = true)
      : uart_(uart),
        chip_enable_(chip_enable),
        state_(state),
        use_cr_nl_(use_cr_nl)
  {
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  virtual void ModuleInitialize() override
  {
    state_.SetAsInput();
    chip_enable_.SetAsOutput();
    chip_enable_.SetHigh();

    uart_.Initialize();
    uart_.ConfigureFormat();
    uart_.ConfigureBaudRate(kAtBaudRate);
  }

  virtual void ModuleEnable([[maybe_unused]] bool enable) override
  {
    uart_.Enable();
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  bool GetDeviceState() const
  {
    return state_.Read();
  }

  void Disconnect() const
  {
    chip_enable_.SetLow();
    // TODO: may need a slight delay here
    chip_enable_.SetHigh();
  }

  // ---------------------------------------------------------------------------
  // AT Command Mode
  // ---------------------------------------------------------------------------

  bool IsAtMode()
  {
    SendCommand(Command::kAt);
    return (at_response_buffer[0] == 'O') && (at_response_buffer[1] == 'K');
  }

  bool EnterAtMode() const
  {
    return false;
  }

  bool ExitAtMode() const
  {
    return false;
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  const std::string_view GetVersion()
  {
    SendCommand(Command::kVersion);
    return &at_response_buffer[Command::kVersion.GetParamPosition()];
  }

  Role GetRole()
  {
    SendCommand(Command::kRole);
    return Role(at_response_buffer[Command::kRole.GetParamPosition()]);
  }

  const std::string_view SetDeviceName(const std::string_view device_name)
  {
    SendCommand(Command::kDeviceName, device_name);
    return &at_response_buffer[Command::kDeviceName.GetParamPosition()];
  }

  const std::string_view GetDeviceName()
  {
    SendCommand(Command::kDeviceName);
    return at_response_buffer;
    // return &at_response_buffer[Command::kDeviceName.GetParamPosition()];
  }

  const std::string_view GetMacAddress()
  {
    SendCommand(Command::kMacAddress);
    return &at_response_buffer[Command::kMacAddress.GetParamPosition()];
  }

  BaudRate SetBaudRate([[maybe_unused]] BaudRate baud)
  {
    const char baud_select = sjsu::Value(baud);
    SendCommand(Command::kBaud, &baud_select);
    return BaudRate(at_response_buffer[Command::kBaud.GetParamPosition()]);
  }

  BaudRate GetBaudRate()
  {
    SendCommand(Command::kBaud);
    return BaudRate(at_response_buffer[Command::kBaud.GetParamPosition()]);
  }

  /// @param uuid The UUID should be in the range of 0x0001 to 0xFFE0
  const std::string_view SetUuid(const std::string_view uuid)
  {
    SendCommand(Command::kUuid, uuid);
    return &at_response_buffer[Command::kUuid.GetParamPosition()];
  }

  const std::string_view GetUuid()
  {
    SendCommand(Command::kUuid);
    return &at_response_buffer[Command::kUuid.GetParamPosition()];
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  void SendCommand(const Command_t command,
                   const std::string_view parameter = "")
  {
    constexpr std::string_view kPrefix = "AT";

    uart_.Write(kPrefix);
    uart_.Write(command.instruction);
    uart_.Write(parameter);

    if (use_cr_nl_)
    {
      constexpr std::string_view kCrNl = "\r\n";
      uart_.Write(kCrNl);
    }

    memset(at_response_buffer, '\0', sizeof(at_response_buffer));

    uart_.Read(std::span(reinterpret_cast<uint8_t *>(at_response_buffer),
                         std::size(at_response_buffer)),
               command.timeout);
  }

 private:
  sjsu::Uart & uart_;
  sjsu::Gpio & chip_enable_;
  sjsu::Gpio & state_;

  /// Buffer for holding responses in AT Command Mode.
  char at_response_buffer[30];

  /// Includes CR & NL when sending AT Command when set to true.
  const bool use_cr_nl_;
};
}  // namespace bluetooth
