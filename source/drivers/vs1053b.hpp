#pragma once

#include <initializer_list>

#include "L1_Peripheral/gpio.hpp"
#include "L1_Peripheral/spi.hpp"
#include "utility/enum.hpp"
#include "utility/time.hpp"

#include "mp3_decoder.hpp"

/// TODO: revise documentation
///
/// The VS1053b operates input clock freq. XTALI = 12.288 MHz or 24-26 MHz when
/// SM_CLK_RANGE in SCI_MODE is set to 1.
///
/// The SCI_CLOCKF register is used to increase the internal clock rate of the
/// device.
///
///   CLKI = XTALI * multiplier
///
/// When performing SCI reads, SPI clk should be ~(CLKI/7).
/// When performing SCI / SDI writes, SPI clk should be ~(CLKI/4).
///
/// Therefore, on reset, the initial SPI clock rate needs to be less
/// than 12.288Mhz / 4 = ~3MHz. Once the SCI_CLOCKF multiplier is set, the SPI
/// clock can be changed to faster speeds.
class Vs1053b : public Mp3Decoder
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
    /// Built-in bass/treble control.
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
    /// Stream mode: 0 = false, 1 = true
    static constexpr auto kStreamModeMask = sjsu::bit::MaskFromRange(6);
    /// SPI mode: 0 = VS1001 Compatibility Mode, 1 = VS10xx New Mode
    static constexpr auto kSdiNewMask = sjsu::bit::MaskFromRange(11);
    /// MIC/LINE1 selector: 0 = MICP, 1 = LINE1
    static constexpr auto kLine1Mask = sjsu::bit::MaskFromRange(14);
    /// Input clock range: 0 = 12-13MHz, 1 = 24-26MHz
    static constexpr auto kClockRangeMask = sjsu::bit::MaskFromRange(15);

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

  /*
  struct SciModeRegister
  {
    /// Differential: 0 = normal in-phase audio, 1 = left channel inverted.
    // static constexpr auto kDifferentialMask = sjsu::bit::MaskFromRange(0);

    /// Allow MPEG layers I & II: 0 = false, 1 = true.
    // static constexpr auto kMpegLayerMask = sjsu::bit::MaskFromRange(1);

    /// Software Reset: 0 = no reset, 1 = reset.
    static constexpr auto kResetMask = sjsu::bit::MaskFromRange(2);
    /// Cancel decoding current file
    static constexpr auto kCancelMask = sjsu::bit::MaskFromRange(3);

    /// EarSpeaker low setting: 0 = off, 1 = active
    // static constexpr auto kEarSpeakerLowMask = sjsu::bit::MaskFromRange(4);

    /// Allow SDI tests
    // static constexpr auto kTestMask = sjsu::bit::MaskFromRange(5);

    /// Stream mode: 0 = false, 1 = true
    static constexpr auto kStreamModeMask = sjsu::bit::MaskFromRange(6);

    /// EarSpeaker high setting: 0 = off, 1 = active
    // static constexpr auto kEarSpeakerHighMask = sjsu::bit::MaskFromRange(7);

    /// DCLK active edge: 0 = rising, 1 = falling
    // static constexpr auto kActiveEdgeMask = sjsu::bit::MaskFromRange(8);

    /// SDI bit order: 0 = MSb first, 1 = MSb last
    // static constexpr auto kBitOrderMask = sjsu::bit::MaskFromRange(9);

    /// Share SPI chip select
    // static constexpr auto kShareCsMask = sjsu::bit::MaskFromRange(10);

    /// SPI mode: 0 = VS1001 Compatibility Mode, 1 = VS10xx New Mode
    static constexpr auto kSdiNewMask = sjsu::bit::MaskFromRange(11);

    /// PCM/ADPCM recording active: 0 = false, 1 = true
    // static constexpr auto kAdpcmMask = sjsu::bit::MaskFromRange(12);

    /// MIC/LINE1 selector: 0 = MICP, 1 = LINE1
    static constexpr auto kLine1Mask = sjsu::bit::MaskFromRange(14);

    /// Input clock range: 0 = 12-13MHz, 1 = 24-26MHz
    static constexpr auto kClockRangeMask = sjsu::bit::MaskFromRange(15);

    static constexpr uint16_t kModeDefault =
        sjsu::bit::Value().Set(kSdiNewMask).Set(kLine1Mask);
  }; */

  enum class SciClockOption : uint16_t
  {
    /// Multiply internal clock by 1x
    kMultiplyBy1 = 0x0000,
    /// Multiply internal clock by 2x
    kMultiplyBy2 = 0x2000,
    /// Multiply internal clock by 2.5x
    kMultiplyBy2_5 = 0x4000,
    /// Multiply internal clock by 3x
    kMultiplyBy3 = 0x6000,
    /// Multiply internal clock by 3.5x
    kMultiplyBy3_5 = 0x8000,
    /// Multiply internal clock by 4x
    kMultiplyBy4 = 0xA000,
  };

  enum class SciAudioDataOption : uint16_t
  {
    kStereo = 0,
    k44100  = 44100,
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

  static constexpr uint16_t kSampleRateLut[4][4] = {
    { 11025, 11025, 22050, 44100 },
    { 12000, 12000, 24000, 48000 },
    { 8000, 8000, 16000, 32000 },
    { 0, 0, 0, 0 }
  };

  /// @param spi The SPI bus used to drive the device.
  /// @param pins The various controls pins for the devies.
  explicit Vs1053b(sjsu::Spi & spi, ControlPins_t pins) : spi_(spi), pins_(pins)
  {
  }

  void Initialize() const override
  {
    pins_.rst.SetAsOutput();
    pins_.rst.SetHigh();
    pins_.cs.SetAsOutput();
    pins_.cs.SetHigh();
    pins_.dcs.SetAsOutput();
    pins_.dcs.SetHigh();

    pins_.dreq.SetAsInput();

    // Initial SPI speed needs to be 3 MHz
    spi_.SetClock(3_MHz);
    spi_.SetDataSize(sjsu::Spi::DataSize::kEight);
    spi_.Initialize();

    Reset();

    WriteSci(SciRegister::kClockF,
             { sjsu::Value(SciClockOption::kMultiplyBy4) });

    while (!IsReady())
    {
      continue;
    }
    // Can now clock the SPI clock higher once the multiplier is set.
    // The internal device clock is now CLKI = 4 * ~12.288 MHz = ~49.152 MHz
    //    For SCI r/w, a SPI clock CLKI / 7 = ~7 MHz is desired.
    //    For SDI write, a SPI clock CLKI / 4 ~12 MHz is desired.

    SetVolume(0.8f);
    printf("VS1053b Initialized\n");
  }

  /// @see Data Request Pin DREQ
  ///      https://cdn-shop.adafruit.com/datasheets/vs1053.pdf#page=16
  bool IsReady() const
  {
    return pins_.dreq.Read();
  }

  /// Toggles the reset pin to perform a hardware reset.
  void Reset() const override
  {
    pins_.rst.SetHigh();
    pins_.rst.SetLow();
    sjsu::Delay(10us);
    pins_.rst.SetHigh();

    while (!IsReady())
    {
      continue;
    }
  }

  /// Performs a software reset by sending the reset command to the SCI Mode
  /// register.
  void SoftwareReset() const
  {
    constexpr uint16_t reset_command = SciModeRegister()
                                           .Set(SciModeRegister::kSdiNewMask)
                                           .Set(SciModeRegister::kResetMask);

    WriteSci(SciRegister::kMode, { reset_command });
    sjsu::Delay(2us);

    while (!IsReady())
    {
      continue;
    }
  }

  // ---------------------------------------------------------------------------
  //                  Mp3Player Interface Implementation
  // ---------------------------------------------------------------------------

  /// Start audio decoding from 0:00.
  void EnablePlayback() const override
  {
    ResumePlayback();
    // Automatic Resync selector
    WriteSci(SciRegister::kWRamAddr, { 0x1E29 });
    WriteSci(SciRegister::kWRam, { 0x0000 });
    ClearDecodeTime();
  }

  /// Pause audio decoding. Note that when decoding is resumes, the audio will
  /// be start from the point it was paused.
  void PausePlayback() const override
  {
    constexpr uint16_t kStreamModeCancel =
        SciModeRegister::Default().Set(SciModeRegister::kCancelMask);
    WriteSci(SciRegister::kMode, { kStreamModeCancel });

    while (!IsReady() || sjsu::bit::Read(ReadRegister(SciRegister::kMode),
                                         SciModeRegister::kCancelMask))
    {
      continue;
    }
  }

  /// Resume audio decoding.
  void ResumePlayback() const override
  {
    constexpr uint16_t kAuDataOption =
        sjsu::Value(SciAudioDataOption::kStereo) |
        sjsu::Value(SciAudioDataOption::k44100);

    WriteSci(SciRegister::kMode, { SciModeRegister::Default() });
    WriteSci(SciRegister::kAuData, { kAuDataOption });
  }

  // Resets the current decode time to 0:00, this is done by writing 0x0 to the
  // SCI decode time register twice.
  void ClearDecodeTime() const override
  {
    WriteSci(SciRegister::kDecodeTime, { 0x0000 });
    WriteSci(SciRegister::kDecodeTime, { 0x0000 });
  }

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
    uint16_t volume =
        static_cast<uint8_t>(0xFF - static_cast<uint8_t>(255.0f * percentage));
    // The VS_VOL register contains the 16-bit control for the volume where the
    // higher 8-bits is for the left channel and the lower 8-bits are for the
    // right channel.
    uint16_t data = static_cast<uint16_t>(volume << 8) | volume;
    WriteSci(SciRegister::kVolume, { 0x2222 });
  }

  /// Reads a desired SCI register.
  ///
  /// @param address The address of the SCI register to read.
  /// @return The 16-bit register data.
  uint16_t ReadRegister(SciRegister address) const
  {
    while (!IsReady())
    {
      continue;
    }

    spi_.SetClock(3_MHz);

    uint16_t data = 0x0;
    pins_.cs.SetLow();
    {
      spi_.Transfer(sjsu::Value(Operation::kRead));
      spi_.Transfer(sjsu::Value(address));
      data = static_cast<uint16_t>(data | (spi_.Transfer(0x00) << 8));
      data = data | spi_.Transfer(0x00);
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
  /// @param data The data to write.
  void WriteSci(SciRegister address,
                const std::initializer_list<uint16_t> data) const
  {
    while (!IsReady())
    {
      continue;
    }

    spi_.SetClock(3_MHz);
    pins_.cs.SetLow();
    {
      spi_.Transfer(sjsu::Value(Operation::kWrite));
      spi_.Transfer(sjsu::Value(address));

      for (uint16_t word : data)
      {
        spi_.Transfer(static_cast<uint8_t>(word >> 8));
        spi_.Transfer(word & 0xFF);
      }
    }
    pins_.cs.SetHigh();
  }

  void WriteSdi(const uint8_t * data, size_t length) const
  {
    while (!IsReady())
    {
      continue;
    }

    spi_.SetClock(12_MHz);

    pins_.dcs.SetLow();
    {
      for (size_t i = 0; i < length; i++)
      {
        spi_.Transfer(data[i]);
      }
    }
    pins_.dcs.SetHigh();
  }

 private:
  const sjsu::Spi & spi_;
  const ControlPins_t pins_;
};
