#include "L0_Platform/startup.hpp"
#include "L1_Peripheral/lpc17xx/gpio.hpp"
#include "L1_Peripheral/lpc17xx/spi.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"

#include "st7735.hpp"

// private namespace
namespace
{
sjsu::lpc17xx::Spi spi0(sjsu::lpc17xx::SpiBus::kSpi0);
sjsu::lpc17xx::Gpio lcd_dc(2, 5);
sjsu::lpc17xx::Gpio lcd_rst(2, 6);
sjsu::lpc17xx::Gpio lcd_cs(2, 7);

constexpr units::frequency::hertz_t kLcdFrequency = 12_MHz;
sjsu::St7735 lcd(spi0, kLcdFrequency, lcd_rst, lcd_cs, lcd_dc);

/// Configures the CPU clock to run at 96 MHz.
void ConfigurateSystemClock()
{
  auto system_controller =
      sjsu::lpc17xx::SystemController::GetPlatformController();
  auto clock_configuration = system_controller.GetClockConfiguration<
      sjsu::lpc17xx::SystemController::ClockConfiguration_t>();
  clock_configuration.cpu.divider = 3;

  sjsu::InitializePlatform();
}
}  // namespace

int main()
{
  LOG_INFO("Starting Application");

  ConfigurateSystemClock();

  SJ2_ASSERT_FATAL(lcd.Initialize() == sjsu::Status::kSuccess,
                   "Failed to initialize St7735");

  while (true)
  {
    lcd.FillFrame(graphics::Frame_t(0, 0, lcd.GetWidth(), 10),
    graphics::kRed); lcd.FillFrame(graphics::Frame_t(0, 20, lcd.GetWidth(),
    10),
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