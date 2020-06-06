#include "L1_Peripheral/gpio.hpp"
#include "L1_Peripheral/spi.hpp"
#include "L2_HAL/displays/pixel_display.hpp"
#include "utility/bit.hpp"
#include "utility/enum.hpp"
#include "utility/log.hpp"

#include "graphics/graphics.hpp"

namespace sjsu
{
class St7735 final : public PixelDisplay
{
 public:
  enum class Command : uint8_t
  {
    kNoOp              = 0x00,
    kSoftwareReset     = 0x01,
    kReadDeviceId      = 0x04,
    kSetColumnAddress  = 0x2A,
    kSetRowAddress     = 0x2B,
    kRamWrite          = 0x2C,
    kRamRead           = 0x2E,
    kSleepIn           = 0x10,
    kSleepOut          = 0x11,
    kDisplayOff        = 0x28,
    kDisplayOn         = 0x29,
    kSetWriteDirection = 0x36,
  };

  inline static constexpr size_t kMaxScreenWidth  = 128;
  inline static constexpr size_t kMaxScreenHeight = 160;

  /// @param spi           SPI bus used to control the device.
  /// @param spi_frequency Frequency of SPI, should be between 1Mhz - 12Mhz.
  /// @param rst_pin       Reset (active low).
  /// @param cs_pin        Chip select (active low).
  /// @param dc_pin        Data/Command select (active low).
  explicit St7735(sjsu::Spi & spi,
                  units::frequency::hertz_t spi_frequency,
                  sjsu::Gpio & rst_pin,
                  sjsu::Gpio & cs_pin,
                  sjsu::Gpio & dc_pin)
      : spi_(spi),
        kSpiFrequency(spi_frequency),
        rst_pin_(rst_pin),
        cs_pin_(cs_pin),
        dc_pin_(dc_pin)
  {
  }
  /// @returns The initialization status.
  Status Initialize() override
  {
    rst_pin_.SetAsOutput();
    rst_pin_.SetHigh();
    cs_pin_.SetAsOutput();
    cs_pin_.SetHigh();
    dc_pin_.SetAsOutput();
    dc_pin_.SetHigh();

    spi_.SetClock(kSpiFrequency);
    spi_.SetDataSize(sjsu::Spi::DataSize::kEight);
    const Status kStatus = spi_.Initialize();
    if (kStatus != Status::kSuccess)
    {
      return kStatus;
    }
    Reset();
    return kStatus;
  }

  size_t GetWidth() override
  {
    return kMaxScreenWidth;
  }

  size_t GetHeight() override
  {
    return kMaxScreenHeight;
  }

  Color_t AvailableColors() override
  {
    return Color_t{ 1, 1, 1, 1 };
  }

  void Sleep(bool on = true)
  {
    WriteCommand(on ? Command::kSleepIn : Command::kSleepOut);
    Delay(2ms);
  }

  void Enable() override
  {
    WriteCommand(Command::kDisplayOn);
    Delay(100us);
  }

  void Disable() override
  {
    WriteCommand(Command::kDisplayOff);
    Delay(100us);
  }

  void Reset()
  {
    cs_pin_.SetLow();
    {
      rst_pin_.SetLow();
      Delay(1ms);
      rst_pin_.SetHigh();
      Delay(1ms);
    }
    cs_pin_.SetHigh();
    // the device is in sleep mode and display is off after a hardware reset
    Sleep(false);
    Enable();
    // set initial display to show a white screen
    Clear();
    // set orientation X-Y exchange
    WriteCommand(Command::kSetWriteDirection);
    constexpr uint8_t kOrientation = 0x84;
    WriteData(kOrientation);
  }

  void Clear() override
  {
    FillFrame(graphics::Frame_t(0, 0, kMaxScreenWidth, kMaxScreenHeight),
              graphics::kWhite);
  }

  void FillFrame(graphics::Frame_t frame, graphics::Color_t color) const
  {
    SetDrawAddress(frame);
    WriteColor(color, frame.size.width * frame.size.height);
  }

  void DrawBitmap(graphics::Frame_t frame,
                  const graphics::Color_t ** bitmap)
  {
    SetDrawAddress(frame);
    for (uint16_t y = 0; y < frame.size.width; y++)
    {
      for (uint16_t x = 0; x < frame.size.height; x++)
      {
        WriteColor(bitmap[x][y]);
      }
    }
  }

  void DrawPixel(int32_t x, int32_t y, Color_t color) override {}
  void Update() override {}

 private:
  void WriteCommand(Command command) const
  {
    dc_pin_.SetLow();
    cs_pin_.SetLow();
    {
      spi_.Transfer(Value(command));
    }
    cs_pin_.SetHigh();
    dc_pin_.SetHigh();
  }

  void WriteData(uint8_t data) const
  {
    cs_pin_.SetLow();
    {
      spi_.Transfer(data);
    }
    cs_pin_.SetHigh();
  }

  void WriteData(uint16_t data) const
  {
    cs_pin_.SetLow();
    {
      spi_.Transfer(static_cast<uint8_t>(data >> 8));
      spi_.Transfer(data & 0xFF);
    }
    cs_pin_.SetHigh();
  }

  void SetDrawAddress(graphics::Frame_t frame) const
  {
    // write x-component of address window
    WriteCommand(Command::kSetColumnAddress);
    WriteData(frame.origin.y);
    WriteData(static_cast<uint16_t>(frame.origin.y + frame.size.height - 1));
    // write y-component of address window
    WriteCommand(Command::kSetRowAddress);
    WriteData(frame.origin.x);
    WriteData(static_cast<uint16_t>(frame.origin.x + frame.size.width - 1));
    WriteCommand(Command::kRamWrite);
  }

  void WriteColor(graphics::Color_t color, uint32_t repeat_count = 1) const
  {
    while (repeat_count > 0)
    {
      WriteData(color.red);
      WriteData(color.green);
      WriteData(color.blue);
      repeat_count--;
    }
  }

  const sjsu::Spi & spi_;
  const units::frequency::hertz_t kSpiFrequency;
  const sjsu::Gpio & rst_pin_;
  const sjsu::Gpio & cs_pin_;
  const sjsu::Gpio & dc_pin_;
};
}  // namespace sjsu
