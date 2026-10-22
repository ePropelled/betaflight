# HSI_H743 Betaflight Changelog


## EP-PILOT Integration

- Added the `HSI_H743` STM32H743 target configuration and EP-PILOT peripheral mapping.
- Added the ePropelled Betaflight config submodule fork.
- Mapped the EP-PILOT sensors and interfaces, including BMI270, BMP581, LIS2MDL,
  SDMMC, USB, UARTs, CAN, ADC inputs, motor outputs, heater, power enables, and LEDs.

## LED Indicator Update

The HSI LED indicators were updated to match the board wiring and provide
dedicated status feedback:

- **RGB status LED D680:** PE4 is blue, PE2 is green, and PE3 is red.
- **Amber status:** red and green are driven together when the status requires
  amber.
- **Heartbeat LED D693:** PE10 now provides a dedicated heartbeat indication.
- **Armed LED D694:** PB2 now indicates the armed state using active-low
  control.
- **Electrical behavior:** RGB and indicator outputs use the required
  open-drain configuration.
- **Startup behavior:** the HSI startup sequence now uses the RGB channels for
  color and status indication without changing other Betaflight targets.


## Current Validation State

- HSI_H743 Docker build succeeds and produces ELF/HEX images after the LED
  indicator update.
- BMI270, BMP580, and LIS2MDL are detected at runtime.
- Gyro 2 is disabled and removed from the active SPI mapping.
- PA8 and PC13 are configured through PinIO and measured near 3.3 V after startup.
- SD-card startup, receiver input, ADC/core-temperature reporting, motor/DShot,
  failsafe, and power-good behavior remain to be validated.
- The status-indicator implementation has not yet been tested fully.
