#include <bitset>

#include "L0_Platform/startup.hpp"
#include "L1_Peripheral/lpc17xx/gpio.hpp"
#include "L1_Peripheral/lpc17xx/spi.hpp"
#include "L2_HAL/memory/sd.hpp"
#include "L3_Application/fatfs.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"

// #include "drivers/st7735.hpp"
#include "drivers/vs1053b.hpp"
#include "tasks/audio_data_buffer_task.hpp"
#include "tasks/mp3_player_task.hpp"

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
Vs1053b mp3_decoder(spi0,
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

// -----------------------------------------------------------------------------
//                                  SD Card
// -----------------------------------------------------------------------------

sjsu::rtos::TaskScheduler task_scheduler;
Mp3PlayerTask mp3_player_task(mp3_decoder);
AudioDataBufferTask<Mp3PlayerTask::kBufferLength> audio_buffer_task(
    mp3_player_task);

/// Configures the CPU clock to run at 96 MHz.
void ConfigurateSystemClock()
{
  auto & system_controller   = sjsu::SystemController::GetPlatformController();
  auto & clock_configuration = system_controller.GetClockConfiguration<
      sjsu::lpc17xx::SystemController::ClockConfiguration_t>();
  clock_configuration.cpu.divider = 6;

  sjsu::InitializePlatform();
}
}  // namespace

int main()
{
  sjsu::LogInfo("Starting Application");

  ConfigurateSystemClock();

  sjsu::lpc17xx::Gpio sd_cs(1, 25);
  sjsu::lpc17xx::Gpio sd_cd(1, 26);
  sjsu::Sd sd_card(spi1, sd_cs, sd_cd);
  sd_card.Initialize();

  // Register and mount FatFs
  FATFS fat_fs;
  if (!sjsu::RegisterFatFsDrive(&sd_card))
  {
    return -1;
  }
  if (f_mount(&fat_fs, "", 0) != 0)
  {
    sjsu::LogError("Failed to mount SD Card");
    return -2;
  }

  mp3_decoder.Initialize();
  mp3_decoder.SetVolume(0.8f);

  task_scheduler.AddTask(&mp3_player_task);
  task_scheduler.AddTask(&audio_buffer_task);
  task_scheduler.Start();

  while (true)
  {
    continue;
  }

  return 0;
}
