#include "L0_Platform/startup.hpp"
#include "L1_Peripheral/lpc17xx/gpio.hpp"
#include "L1_Peripheral/lpc17xx/spi.hpp"
#include "L1_Peripheral/lpc17xx/uart.hpp"
#include "L2_HAL/memory/sd.hpp"
#include "L3_Application/fatfs.hpp"
#include "drivers/serial_bluetooth.hpp"
#include "drivers/st7735.hpp"
#include "drivers/vs1053b.hpp"
#include "drivers/zs040.hpp"
#include "tasks/audio_data_buffer_task.hpp"
#include "tasks/mp3_player_task.hpp"
#include "utility/log.hpp"

// private namespace
namespace
{
// sjsu::lpc17xx::Spi spi0(sjsu::lpc17xx::SpiBus::kSpi0);
// sjsu::lpc17xx::Spi spi1(sjsu::lpc17xx::SpiBus::kSpi1);
sjsu::lpc17xx::Uart uart2(sjsu::lpc17xx::UartPort::kUart2);

// -----------------------------------------------------------------------------
//                                MP3 Decoder
// -----------------------------------------------------------------------------

// sjsu::lpc17xx::Gpio dreq(2, 4);  // blue
// sjsu::lpc17xx::Gpio rst(2, 5);   // gree
// sjsu::lpc17xx::Gpio cs(2, 6);    // yellow
// sjsu::lpc17xx::Gpio dcs(2, 7);   // orange
// Vs1053b mp3_decoder(spi0,
//                     {
//                         .rst  = rst,
//                         .cs   = cs,
//                         .dcs  = dcs,
//                         .dreq = dreq,
//                     });

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

// sjsu::lpc17xx::Gpio sd_cs(1, 25);
// sjsu::lpc17xx::Gpio sd_cd(1, 26);
// sjsu::Sd sd_card(spi1, sd_cs, sd_cd);

// -----------------------------------------------------------------------------
//                                  Bluetooth
// -----------------------------------------------------------------------------

constexpr uint32_t kZs040DefaultBaudRate = 38'400;
sjsu::lpc17xx::Gpio ble_state(0, 0);
sjsu::lpc17xx::Gpio ble_enable(0, 1);
// SerialBluetooth zs040(uart2, hm10_enable, hm10_state, kZs040DefaultBaudRate);
bluetooth::Zs040 zs040(uart2, ble_enable, ble_state, kZs040DefaultBaudRate);
// bluetooth::Zs040 zs040(uart2, hm10_enable, hm10_state,
// kZs040DefaultBaudRate);

// -----------------------------------------------------------------------------
//                                  Tasks
// -----------------------------------------------------------------------------

// sjsu::rtos::TaskScheduler task_scheduler;
// Mp3PlayerTask mp3_player_task(mp3_decoder);
// AudioDataBufferTask<Mp3PlayerTask::kBufferLength> audio_buffer_task(
//     mp3_player_task);
// AudioDataDecodeTask<Mp3PlayerTask::kBufferLength>
// decoder_task(mp3_player_task);
}  // namespace

int main()
{
  sjsu::LogInfo("Starting Application");

  /// Configures the CPU clock to run at 96 MHz.
  auto & system_controller   = sjsu::SystemController::GetPlatformController();
  auto & clock_configuration = system_controller.GetClockConfiguration<
      sjsu::lpc17xx::SystemController::ClockConfiguration_t>();
  clock_configuration.cpu.divider = 3; // 48 MHz

  sjsu::InitializePlatform();

  zs040.Initialize();
  zs040.Enable();

  if (zs040.IsAtMode())
  {
    sjsu::LogInfo("Is in AT Mode");
    sjsu::LogInfo("Version:     %.*s", zs040.GetVersion());
    sjsu::LogInfo("Baud:        %c", sjsu::Value(zs040.GetBaudRate()));
    sjsu::LogInfo("Role:        %c", sjsu::Value(zs040.GetRole()));
    sjsu::LogInfo("MAC Address: %.*s", zs040.GetMacAddress());
    sjsu::LogInfo("Set Name:    %d", zs040.SetDeviceName("Some Device"));
    sjsu::LogInfo("Device Name: %.*s", zs040.GetDeviceName());
    sjsu::LogInfo("UUID:        %.*s", zs040.GetUuid());
  }
  else
  {
    sjsu::LogDebug("Not in AT Mode");
  }

  while (true)
  {
    sjsu::Delay(500ms);
  }

  // sd_card.Initialize();
  // Register and mount FatFs
  // FATFS fat_fs;
  // if (!sjsu::RegisterFatFsDrive(&sd_card))
  // {
  //   return -1;
  // }
  // if (f_mount(&fat_fs, "", 0) != 0)
  // {
  //   sjsu::LogError("Failed to mount SD Card");
  //   return -2;
  // }

  // mp3_decoder.Initialize();
  // mp3_decoder.SetVolume(0.8f);

  // task_scheduler.AddTask(&mp3_player_task);
  // task_scheduler.AddTask(&audio_buffer_task);
  // task_scheduler.AddTask(&decoder_task);
  // task_scheduler.Start();

  // sjsu::Halt();

  return 0;
}
