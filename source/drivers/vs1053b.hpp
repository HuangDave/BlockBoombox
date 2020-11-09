#pragma once

#include "L1_Peripheral/gpio.hpp"
#include "L1_Peripheral/spi.hpp"
#include "audio_decoder.hpp"
#include "utility/enum.hpp"
#include "utility/time.hpp"

/// Audio decoder capable of decoding various formats such as Ogg Vorbis, MP3,
/// AAC, WMA, and MIDI.
class Vs1053b : public AudioDecoder, public sjsu::Module
{
 public:
  /// @see 7.4 Serial Protocol for Serial Command Interface (SPI / SCI)
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=20
  enum class Operation : uint8_t
  {
    kWrite = 0x02,
    kRead  = 0x03,
  };

  /// @see 9.6 SCI Registers
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=37
  enum class SciRegister : uint8_t
  {
    /// Mode control.
    kMode = 0x0,
    /// Device status.
    kStatus = 0x1,
    /// Clock multiplier.
    kClockF = 0x3,
    /// Decode time in seconds.
    kDecodeTime = 0x4,
    /// Miscellaneous audio data.
    kAuData = 0x5,
    /// RAM write/read.
    kWRam = 0x6,
    /// Base address for RAM write/read.
    kWRamAddr = 0x7,
    /// Stream header data 0.
    kHDat0 = 0x8,
    /// Stream header data 1.
    kHDat1 = 0x9,
    /// Volume control.
    kVolume = 0xB,
  };

  /// @see 9.6.1 SCI_MODE (RW)
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=38
  class SciModeRegister final : public sjsu::bit::Value<uint16_t>
  {
   public:
    /// Software Reset: 0 = no reset, 1 = reset.
    static constexpr auto kResetMask = sjsu::bit::MaskFromRange(2);
    /// Cancel decoding current file
    static constexpr auto kCancelMask = sjsu::bit::MaskFromRange(3);
    /// SPI mode: 0 = VS1001 Compatibility Mode, 1 = VS10xx New Mode
    static constexpr auto kSdiNewMask = sjsu::bit::MaskFromRange(11);
    /// MIC/LINE1 selector: 0 = MICP, 1 = LINE1
    static constexpr auto kLine1Mask = sjsu::bit::MaskFromRange(14);

    /// @returns The default configuration for the SCI mode register (0x4800).
    static constexpr SciModeRegister Default()
    {
      auto value = SciModeRegister();
      value.Set(kSdiNewMask).Set(kLine1Mask);
      return value;
    }

    explicit constexpr SciModeRegister(uint16_t value = 0x0000) : Value(value)
    {
    }
  };

  /// @see 7.1.1 VS10xx Native Modes (New Mode, recommended)
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=15
  struct ControlPins_t
  {
    /// Reset pin, active low.
    sjsu::Gpio & rst;
    /// Chip select pin, active low. This pin is pulled low by the driver when
    /// writing to the SCI register.
    sjsu::Gpio & cs;
    /// Data chip select pin, active low. This pin is pulled low by the driver
    /// when writing to the SDI register.
    sjsu::Gpio & dcs;
    /// Data request input pin. This pin is pulled high by the device when
    /// processing an operation.
    sjsu::Gpio & dreq;
  };

  // static constexpr uint16_t kSampleRateLut[4][4] = {
  //   { 11025, 11025, 22050, 44100 },
  //   { 12000, 12000, 24000, 48000 },
  //   { 8000, 8000, 16000, 32000 },
  //   { 0, 0, 0, 0 }
  // };

  /// @param spi The SPI bus used to drive the device.
  /// @param pins The various controls pins for the devies.
  explicit Vs1053b(sjsu::Spi & spi, ControlPins_t pins) : spi_(spi), pins_(pins)
  {
  }

  virtual void ModuleInitialize() override
  {
    pins_.rst.SetAsOutput();
    pins_.rst.SetHigh();
    pins_.cs.SetAsOutput();
    pins_.cs.SetHigh();
    pins_.dcs.SetAsOutput();
    pins_.dcs.SetHigh();

    pins_.dreq.SetAsInput();

    // The VS1053b operates at an input clock frequency of XTALI = 12.288 MHz
    // when SM_CLK_RANGE in the SCI_MODE register is set to 0 or 24-26 MHz when
    // SM_CLK_RANGE is set to 1.
    //
    // The SCI_CLOCKF register is used to increase the internal clock rate of
    // the device. On reset, the multiplier is 1.0x. The clock frequency is:
    //
    //   CLKI = XTALI * multiplier
    //
    // When performing SCI reads, SPI clk should be ~(CLKI/7).
    // When performing SCI / SDI writes, SPI clk should be ~(CLKI/4).
    //
    // Therefore, on reset, the initial SPI clock rate needs to be less
    // than CLKI / 4 = ~3 MHz.
    //
    // Once the SCI_CLOCKF multiplier is set, the SPI clock can be changed to
    // faster speeds.
    spi_.ConfigureFrequency(3_MHz);
    spi_.ConfigureFrameSize(sjsu::Spi::FrameSize::kEightBits);
    spi_.Initialize();

    Reset();

    constexpr units::frequency::hertz_t kXtali = 12.288_MHz;
    read_speed_                                = kXtali / 7;
    write_speed_                               = kXtali / 4;

    constexpr uint16_t kClockMultiplier = 0xA000;  // 4x multiplier
    WriteSci(SciRegister::kClockF, kClockMultiplier);
    WaitForReadyStatus();

    // Can now clock the SPI clock higher once the multiplier is set and the
    // device is ready.
    //
    // The internal device clock is now CLKI = 4 * ~12.288 MHz = ~49.152 MHz
    //    For SCI read, a SPI clock CLKI / 7 = ~7 MHz is desired.
    //    For SCI/SDI write, a SPI clock CLKI / 4 = ~12 MHz is desired.
    constexpr units::frequency::hertz_t kClki = kXtali * 4;
    read_speed_                               = kClki / 7;
    write_speed_                              = kClki / 4;
  }

  virtual void ModuleEnable([[maybe_unused]] bool enable) override {}

  /// @see Data Request Pin DREQ
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=16
  void WaitForReadyStatus() const
  {
    while (!pins_.dreq.Read())
    {
      continue;
    }
  }

  /// Toggles the reset pin to perform a hardware reset.
  void Reset() const override
  {
    pins_.rst.SetHigh();
    pins_.rst.SetLow();
    sjsu::Delay(10us);
    pins_.rst.SetHigh();

    WaitForReadyStatus();
  }

  /// Performs a software reset by sending the reset command to the SCI Mode
  /// register.
  void SoftwareReset() const
  {
    constexpr uint16_t reset_command = SciModeRegister()
                                           .Set(SciModeRegister::kSdiNewMask)
                                           .Set(SciModeRegister::kResetMask);

    WriteSci(SciRegister::kMode, reset_command);
    sjsu::Delay(2us);
    WaitForReadyStatus();
  }

  // ---------------------------------------------------------------------------
  //                  Mp3Player Interface Implementation
  // ---------------------------------------------------------------------------

  /// Start audio decoding from 0:00.
  void Enable() const override
  {
    Resume();
    // Automatic Resync selector
    WriteSci(SciRegister::kWRamAddr, 0x1E29);
    WriteSci(SciRegister::kWRam, 0x0000);
    ClearDecodeTime();
  }

  /// Pause audio decoding. Note that when decoding is resumes, the audio will
  /// be start from the point it was paused.
  void Pause() const override
  {
    constexpr uint16_t kStreamModeCancel =
        SciModeRegister::Default().Set(SciModeRegister::kCancelMask);
    WriteSci(SciRegister::kMode, kStreamModeCancel);

    while (!pins_.dreq.Read() ||
           sjsu::bit::Read(ReadRegister(SciRegister::kMode),
                           SciModeRegister::kCancelMask))
    {
      continue;
    }
  }

  /// Resume audio decoding.
  void Resume() const override
  {
    constexpr uint16_t kAuDataOption = 0xAC45;
    WriteSci(SciRegister::kMode, SciModeRegister::Default());
    WriteSci(SciRegister::kAuData, kAuDataOption);
  }

  // Resets the current decode time to 0:00, this is done by writing 0x0 to the
  // SCI decode time register twice.
  void ClearDecodeTime() const override
  {
    WriteSci(SciRegister::kDecodeTime, 0x0000);
    WriteSci(SciRegister::kDecodeTime, 0x0000);
  }

  /// Sends audio data to the decoder 32 bytes at a time.
  ///
  /// @see 9.4 Serial Data Interface (SDI)
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=37
  ///
  /// @note Need to wait for DREQ
  void Buffer(const uint8_t * data, size_t length) const override
  {
    constexpr size_t kBufferSize = 32;
    for (size_t i = 0; i < length / kBufferSize; i++)
    {
      WriteSdi(data + (i * kBufferSize), kBufferSize);
    }
  }

  /// Sets the volume for both L and R audio channels.
  ///
  /// @param percentage Volume percentage ranging from 0.0 to 1.0, where 1.0 is
  ///                   100 percent.
  void SetVolume(float percentage) const override
  {
    // Find difference, max volume for device is 0x00 and min volume is 0xFF
    uint16_t difference =
        static_cast<uint8_t>(0xFF - static_cast<uint8_t>(255.0f * percentage));
    // The VS_VOL register contains the 16-bit control for the volume where the
    // higher 8-bits is for the left channel and the lower 8-bits are for the
    // right channel.
    uint16_t volume = static_cast<uint16_t>(difference << 8) | volume;
    WriteSci(SciRegister::kVolume, volume);
  }

 private:
  /// Reads a desired SCI register.
  ///
  /// @param address The address of the SCI register to read.
  /// @return The 16-bit register data.
  uint16_t ReadRegister(SciRegister address) const
  {
    WaitForReadyStatus();

    spi_.ConfigureFrequency(read_speed_);

    uint16_t data = 0x0;
    pins_.cs.SetLow();
    {
      constexpr uint8_t kEmptyByte = 0x00;

      spi_.Transfer(sjsu::Value(Operation::kRead));
      spi_.Transfer(sjsu::Value(address));

      data = static_cast<uint16_t>(data | (spi_.Transfer(kEmptyByte) << 8));
      data = static_cast<uint16_t>(data | spi_.Transfer(kEmptyByte));
    }
    pins_.cs.SetHigh();
    return data;
  }

  /// Writes a byte(s) to the specified SCI register.
  ///
  /// @see 9.5 Serial Control Interface (SCI)
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=37
  ///
  /// @note Need to wait for DREQ
  ///
  /// @param address The address of the SCI register to write to.
  /// @param data The 16-bit data to write.
  void WriteSci(SciRegister address, uint16_t data) const
  {
    WaitForReadyStatus();

    spi_.ConfigureFrequency(write_speed_);

    pins_.cs.SetLow();
    {
      spi_.Transfer(sjsu::Value(Operation::kWrite));
      spi_.Transfer(sjsu::Value(address));
      spi_.Transfer(static_cast<uint8_t>(data >> 8));
      spi_.Transfer(static_cast<uint8_t>(data & 0xFF));
    }
    pins_.cs.SetHigh();
  }

  /// Sends audio data byte(s) for decoding.
  ///
  /// @param data The data to write.
  /// @param length Length of the data array. This should not exceed 32 since
  ///               DREQ must be checked every 32 bytes.
  void WriteSdi(const uint8_t * data, size_t length) const
  {
    WaitForReadyStatus();

    spi_.ConfigureFrequency(write_speed_);

    pins_.dcs.SetLow();
    {
      for (size_t i = 0; i < length; i++)
      {
        spi_.Transfer(data[i]);
      }
    }
    pins_.dcs.SetHigh();
  }

  sjsu::Spi & spi_;
  ControlPins_t pins_;
  mutable units::frequency::hertz_t read_speed_  = 0_MHz;
  mutable units::frequency::hertz_t write_speed_ = 0_MHz;
};
