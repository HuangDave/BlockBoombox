#include "L0_Platform/startup.hpp"
#include "L1_Peripheral/lpc17xx/gpio.hpp"
#include "L1_Peripheral/lpc17xx/spi.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"

#include "drivers/st7735.hpp"
#include "drivers/vs1053b.hpp"

// private namespace
namespace
{
sjsu::lpc17xx::Spi spi0(sjsu::lpc17xx::SpiBus::kSpi0);
sjsu::lpc17xx::Gpio lcd_dc(0, 1);
sjsu::lpc17xx::Gpio lcd_rst(0, 0);
sjsu::lpc17xx::Gpio lcd_cs(2, 7);

sjsu::lpc17xx::Spi spi1(sjsu::lpc17xx::SpiBus::kSpi1);
// NOTES: connections on board
// o    y    g    b
// 0.26 1.31 1.30 1.29
//
// | y | b |
// |---|---|
// | o | g |
sjsu::lpc17xx::Gpio rst(1, 30);
sjsu::lpc17xx::Gpio cs(1, 31);
sjsu::lpc17xx::Gpio dcs(0, 26);
sjsu::lpc17xx::Gpio dreq(1, 29);
Vs1053b mp3_player(spi1,
                   {
                       .rst  = rst,
                       .cs   = cs,
                       .dcs  = dcs,
                       .dreq = dreq,
                   });

constexpr units::frequency::hertz_t kLcdFrequency = 12_MHz;
constexpr size_t kLcdScreenWidth                  = 128;
constexpr size_t kLcdScreenHeight                 = 160;
St7735 lcd(spi0,
           kLcdFrequency,
           lcd_rst,
           lcd_cs,
           lcd_dc,
           kLcdScreenWidth,
           kLcdScreenHeight);

/// Configures the CPU clock to run at 96 MHz.
void ConfigurateSystemClock()
{
  auto & system_controller   = sjsu::SystemController::GetPlatformController();
  auto & clock_configuration = system_controller.GetClockConfiguration<
      sjsu::lpc17xx::SystemController::ClockConfiguration_t>();
  clock_configuration.cpu.divider = 3;

  sjsu::InitializePlatform();
}
}  // namespace

int main()
{
  LOG_INFO("Starting Application");

  ConfigurateSystemClock();

  mp3_player.Initialize();
  lcd.Initialize();

  while (true)
  {
    lcd.FillFrame(graphics::Frame_t(0, 0, lcd.GetWidth(), 10), graphics::kRed);
    lcd.FillFrame(graphics::Frame_t(0, 20, lcd.GetWidth(), 10),
                  graphics::kGreen);
    lcd.FillFrame(graphics::Frame_t(0, 40, lcd.GetWidth(), 10),
                  graphics::kBlue);

    sjsu::Delay(1s);

    lcd.FillFrame(graphics::Frame_t(0, 0, lcd.GetWidth(), 10),
                  graphics::kWhite);
    lcd.FillFrame(graphics::Frame_t(0, 20, lcd.GetWidth(), 10),
                  graphics::kWhite);
    lcd.FillFrame(graphics::Frame_t(0, 40, lcd.GetWidth(), 10),
                  graphics::kWhite);
  }

  return 0;
}
