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
  enum class Mode : uint8_t
  {
    kMain,
    kMaster,
  };

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

  enum class AuthType : char
  {
    kNoPassword        = '0',
    kPairing           = '1',
    kPairingAndBinding = '2',
  };

  struct Command  // NOLINT
  {
    static constexpr std::string_view kAt            = "";
    static constexpr std::string_view kSoftwareReset = "+RESET";
    static constexpr std::string_view kSleep         = "+SLEEP";

    static constexpr std::string_view kVersion    = "+VERSION";
    static constexpr std::string_view kMacAddress = "+LADDR";

    static constexpr std::string_view kBaud           = "+BAUD";
    static constexpr std::string_view kRole           = "+ROLE";
    static constexpr std::string_view kUuid           = "+UUID";
    static constexpr std::string_view kCharacteristic = "+CHAR";
    static constexpr std::string_view kDeviceName     = "+NAME";
    static constexpr std::string_view kPin            = "+PIN";

    static constexpr std::string_view kScanDevices = "+INQ";
    static constexpr std::string_view kConnect     = "+CONN";
  };

  struct AtResponseLength
  {
    static constexpr size_t kUuid           = 6;
    static constexpr size_t kCharacteristic = kUuid;
    static constexpr size_t kMacAddress     = 17;
    static constexpr size_t kPin            = 6;
  };

  static constexpr uint32_t kDefaultBaudRate = 9'600;
  // static constexpr uint32_t kAtBaudRate      = 38'400;

  static constexpr std::string_view kCrNl = "\r\n";
  inline static const char * kStatusOk    = "OK\r\n";

  /// @param uart The UART peripheral used to communicate with the device.
  /// @param chip_enable The device's chip enable pin. When driven low, the
  /// device will disconnect any connected bluetooth devices.
  /// @param state The device's state pin.
  /// @param use_cr_nl When true, send carriage return (CR) and new line (NL) at
  /// the end of each command.
  explicit Zs040(sjsu::Uart & uart,
                 sjsu::Gpio & key_pin   = sjsu::GetInactive<sjsu::Gpio>(),
                 sjsu::Gpio & state_pin = sjsu::GetInactive<sjsu::Gpio>(),
                 bool use_cr_nl         = true)
      : uart_(uart),
        key_pin_(key_pin),
        state_pin_(state_pin),
        use_cr_nl_(use_cr_nl)
  {
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  virtual void ModuleInitialize() override
  {
    state_pin_.SetAsInput();
    key_pin_.SetAsOutput();
    key_pin_.SetHigh();

    uart_.Initialize();
    uart_.ConfigureFormat();
    uart_.ConfigureBaudRate(kDefaultBaudRate);
  }

  virtual void ModuleEnable([[maybe_unused]] bool enable) override
  {
    uart_.Enable();
  }

  bool GetDeviceState() const
  {
    return state_pin_.Read();
  }

  // ---------------------------------------------------------------------------
  // AT Command Mode
  // ---------------------------------------------------------------------------

  bool IsAtMode()
  {
    SendCommand(Command::kAt);
    return (strcmp(at_response_buffer, kStatusOk) == 0);
  }

  void EnterAtMode() const {}

  void ExitAtMode() const {}

  void SoftwareReset()
  {
    SendCommand(Command::kSoftwareReset);
    sjsu::Delay(500ms);
  }

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  /// @returns The device's version.
  const std::string_view GetVersion()
  {
    SendCommand(Command::kVersion);
    return std::string_view(at_response_buffer,
                            GetEndOfCurrentResponse());
  }

  //// @returns The current configured role.
  Role GetRole()
  {
    SendCommand(Command::kRole);
    return Role(at_response_buffer[Command::kRole.size() + 1]);
  }

  /// @param device_name The name to set, must be 18 bytes or less.
  bool SetDeviceName(const std::string_view device_name)
  {
    SendCommand(Command::kDeviceName, device_name, true);
    return strstr(at_response_buffer, kStatusOk) != nullptr;
  }

  const std::string_view GetDeviceName()
  {
    SendCommand(Command::kDeviceName);
    return &at_response_buffer[Command::kDeviceName.size() + 1];
  }

  /// @returns The device's MAC address as a string in the following format:
  ///          XX:XX:XX:XX:XX:XX.
  const std::string_view GetMacAddress()
  {
    SendCommand(Command::kMacAddress);

    const char * kStartPointer =
        at_response_buffer + Command::kMacAddress.size() + 1;
    return std::string_view(kStartPointer, AtResponseLength::kMacAddress);
  }

  BaudRate SetBaudRate(const BaudRate baud)
  {
    const char baud_select = sjsu::Value(baud);
    SendCommand(Command::kBaud, &baud_select);
    return BaudRate(at_response_buffer[Command::kBaud.size() + 1]);
  }

  BaudRate GetBaudRate()
  {
    SendCommand(Command::kBaud);
    return BaudRate(at_response_buffer[Command::kBaud.size() + 1]);
  }

  /// @param uuid The 6 byte UUID string ranging between '0x0001' to '0xFFFE'.
  /// @returns
  const std::string_view SetUuid(const std::string_view uuid)
  {
    SendCommand(Command::kUuid, uuid);
    return &at_response_buffer[Command::kUuid.size() + 1];
  }

  /// @returns The 6 byte UUID string ranging between '0x0001' to '0xFFFE'.
  const std::string_view GetUuid()
  {
    SendCommand(Command::kUuid);
    return &at_response_buffer[Command::kUuid.size() + 1];
  }

  // ---------------------------------------------------------------------------
  // Master Mode
  // ---------------------------------------------------------------------------

  void StartScanning()
  {
    SendCommand(Command::kScanDevices, "1");
  }

  void StopScanning()
  {
    SendCommand(Command::kScanDevices, "0");
  }

  void Connect(uint8_t device_index)
  {
    const char index = static_cast<char>(device_index + '0');
    SendCommand(Command::kConnect, &index);
  }

  // ---------------------------------------------------------------------------
  // iBeacon
  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  //
  // ---------------------------------------------------------------------------

  bool SendCommand(const std::string_view command,
                   const std::string_view parameter = "",
                   bool wait_for_ok                 = false)
  {
    constexpr std::string_view kPrefix = "AT";

    uart_.Write(kPrefix);
    uart_.Write(command);
    uart_.Write(parameter);

    if (use_cr_nl_)
    {
      uart_.Write(kCrNl);
    }

    memset(at_response_buffer, '\0', sizeof(at_response_buffer));
    Read(at_response_buffer);

    if (wait_for_ok)
    {
      char status[10];
      memset(status, '\0', sizeof(status));
      Read(status);
      return strstr(status, kStatusOk) != nullptr;
    }

    return true;
  }

 private:
  void Read(std::span<char> buffer)
  {
    constexpr uint8_t kEmptyChar = 255;

    uint32_t idx = 0;
    char c       = kEmptyChar;

    do
    {
      c = uart_.Read();
      if (c != kEmptyChar)
      {
        at_response_buffer[idx++] = c;
      }
    } while (c != '\n' && (idx < buffer.size()));
  }

  size_t GetEndOfCurrentResponse()
  {
    char * term_pointer = strstr(at_response_buffer, "\r\n");
    return term_pointer - at_response_buffer + 1;
  }

  sjsu::Uart & uart_;
  sjsu::Gpio & key_pin_;
  sjsu::Gpio & state_pin_;

  /// Buffer for holding responses in AT Command Mode.
  ///
  /// @note This will not be able to suport the HELP command
  char at_response_buffer[30];

  /// Includes CR & NL when sending AT Command when set to true.
  const bool use_cr_nl_;
};
}  // namespace bluetooth
