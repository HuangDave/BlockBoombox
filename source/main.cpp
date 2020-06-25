#include <bitset>

#include "L0_Platform/startup.hpp"
#include "L1_Peripheral/lpc17xx/gpio.hpp"
#include "L1_Peripheral/lpc17xx/spi.hpp"
#include "L2_HAL/memory/sd.hpp"
#include "L3_Application/file_io/fatfs.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"

#include "mp3_player_task.hpp"
#include "drivers/st7735.hpp"
#include "drivers/vs1053b.hpp"

// private namespace
namespace
{
sjsu::lpc17xx::Spi spi0(sjsu::lpc17xx::SpiBus::kSpi0);
sjsu::lpc17xx::Spi spi1(sjsu::lpc17xx::SpiBus::kSpi1);

// -----------------------------------------------------------------------------
//                                MP3 Decoder
// -----------------------------------------------------------------------------
sjsu::lpc17xx::Gpio dreq(2, 4);  // blue
sjsu::lpc17xx::Gpio rst(2, 5);   // gree
sjsu::lpc17xx::Gpio cs(2, 6);    // yellow
sjsu::lpc17xx::Gpio dcs(2, 7);   // orange
Vs1053b mp3_player(spi0,
                   {
                       .rst  = rst,
                       .cs   = cs,
                       .dcs  = dcs,
                       .dreq = dreq,
                   });

// -----------------------------------------------------------------------------
//                                TFT LCD
// -----------------------------------------------------------------------------
// constexpr units::frequency::hertz_t kLcdFrequency = 12_MHz;
// constexpr size_t kLcdScreenWidth                  = 128;
// constexpr size_t kLcdScreenHeight                 = 160;
// sjsu::lpc17xx::Gpio lcd_dc(0, 1);
// sjsu::lpc17xx::Gpio lcd_rst(0, 0);
// sjsu::lpc17xx::Gpio lcd_cs(2, 7);
// St7735 lcd(spi0,
//            kLcdFrequency,
//            lcd_rst,
//            lcd_cs,
//            lcd_dc,
//            kLcdScreenWidth,
//            kLcdScreenHeight);

/// Configures the CPU clock to run at 96 MHz.
void ConfigurateSystemClock()
{
  auto & system_controller   = sjsu::SystemController::GetPlatformController();
  auto & clock_configuration = system_controller.GetClockConfiguration<
      sjsu::lpc17xx::SystemController::ClockConfiguration_t>();
  clock_configuration.cpu.divider = 3;

  sjsu::InitializePlatform();
}

// void TestLcd()
// {
//   lcd.FillFrame(graphics::Frame_t(0, 0, lcd.GetWidth(), 10),
//   graphics::kRed);
//   lcd.FillFrame(graphics::Frame_t(0, 20, lcd.GetWidth(), 10),
//   graphics::kGreen);
//   lcd.FillFrame(graphics::Frame_t(0, 40, lcd.GetWidth(), 10),
//   graphics::kBlue);
//   sjsu::Delay(1s);
//   lcd.FillFrame(graphics::Frame_t(0, 0, lcd.GetWidth(), 10),
//   graphics::kWhite);
//   lcd.FillFrame(graphics::Frame_t(0, 20, lcd.GetWidth(), 10),
//   graphics::kWhite);
//   lcd.FillFrame(graphics::Frame_t(0, 40, lcd.GetWidth(), 10),
//   graphics::kWhite);
// }

// -----------------------------------------------------------------------------
//                                  SD Card
// -----------------------------------------------------------------------------
sjsu::lpc17xx::Gpio sd_cs(1, 25);
sjsu::lpc17xx::Gpio sd_cd(1, 26);
sjsu::Sd sd_card(spi1, sd_cs, sd_cd);

Mp3PlayerTask mp3_player_task(mp3_player);
}  // namespace

int main()
{
  LOG_INFO("Starting Application");

  ConfigurateSystemClock();

  if (!sjsu::RegisterFatFsDrive(&sd_card))
  {
    return -1;
  }

  FATFS fat_fs;
  if (f_mount(&fat_fs, "", 0) != 0)
  {
    sjsu::LogError("Failed to mount SD Card");
    return false;
  }

  mp3_player.Initialize();

  sjsu::LogInfo("clockf: %x",
                mp3_player.ReadRegister(Vs1053b::SciRegister::kClockF));
  sjsu::LogInfo(
      "mode: %s",
      std::bitset<16>(mp3_player.ReadRegister(Vs1053b::SciRegister::kMode))
          .to_string()
          .c_str());
  sjsu::LogInfo("volume: %x",
                mp3_player.ReadRegister(Vs1053b::SciRegister::kVolume));

  mp3_player_task.FetchSongs();
  mp3_player_task.Play(1);

  while (true)
  {
    // TestLcd();
  }

  return 0;
}
