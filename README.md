# Smart Firefighter Assist Using ESP32

A reconstructed starter project based on the two reference screenshots provided by the project owner.

The project combines:
- ESP32 development board
- MQ-2 smoke/gas sensor
- DHT11 temperature and humidity sensor
- 0.96-inch SSD1306 I2C OLED
- L298N motor driver
- Two DC geared motors
- 7.4 V motor battery pack (use a suitable regulated supply for the ESP32)

> **Important:** The original source code, exact wiring, calibration values, and build files were not included in the screenshots. This repository is a fresh, editable reconstruction—not a recovery of the original project files. Confirm every pin and connection against your actual hardware before powering it.

## What it does

1. Reads the MQ-2 analog output and DHT11 temperature/humidity.
2. Shows live readings and an alert state on the OLED.
3. Stops both drive motors when a smoke or high-temperature alert is detected.
4. Accepts simple movement commands over USB serial (`F`, `B`, `L`, `R`, `S`).
5. Reports readings and status to the Serial Monitor.

This is an educational prototype. It is **not** a certified fire alarm, firefighting robot, or life-safety device. The pictured parts do not include a water pump or extinguisher mechanism, so this code does not claim to extinguish fires. Do not test it in a real fire or hazardous environment.

## Hardware assumed by this reconstruction

| Part | ESP32 connection (default in code) |
|---|---|
| MQ-2 AO | GPIO 34 (ADC input) |
| DHT11 DATA | GPIO 4 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| L298N ENA | GPIO 25 |
| L298N IN1 / IN2 | GPIO 26 / 27 |
| L298N ENB | GPIO 14 |
| L298N IN3 / IN4 | GPIO 16 / 17 |

The pin map is an assumption because it cannot be read from the screenshots. Change the constants in `firmware/smart_firefighter_assist.ino` to match your wiring.

### Wiring and power cautions

- **ESP32 GPIO is 3.3 V logic and is not 5 V tolerant.** Never connect a 5 V sensor output directly to an ESP32 pin.
- MQ-2 modules are commonly powered from 5 V and their AO output may exceed 3.3 V. Use a correctly calculated voltage divider or suitable signal-conditioning circuit before GPIO 34. Verify the actual module output with a meter.
- Connect grounds together (ESP32, sensor supply, and motor-driver logic ground).
- Power motors from an appropriate motor supply through the L298N. Do not power motors from the ESP32 3.3 V pin.
- Use a suitable regulated supply for the ESP32. Do not connect a 7.4 V battery directly to the ESP32 3.3 V pin.
- Remove the L298N ENA/ENB jumpers if using the ESP32 PWM pins shown in this code.
- Check motor polarity and driver wiring with wheels lifted off the surface.

## Software setup

1. Install the Arduino IDE.
2. Add ESP32 board support using the official Espressif Arduino-ESP32 installation instructions.
3. Install these libraries from Arduino Library Manager:
   - `DHT sensor library` by Adafruit
   - `Adafruit Unified Sensor`
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
4. Open `firmware/smart_firefighter_assist.ino`.
5. Select your ESP32 board and the correct serial port.
6. Review and update the pin definitions and alert thresholds.
7. Upload the sketch.
8. Open Serial Monitor at **115200 baud**.

## Serial commands

Send one character in the Serial Monitor (newline setting does not matter):

| Command | Action |
|---|---|
| `F` | Forward |
| `B` | Backward |
| `L` | Turn left |
| `R` | Turn right |
| `S` | Stop |

Motors start stopped. Any detected alert forces an immediate stop. Commands are ignored while an alert is active.

## Calibration

The MQ-2 is a broad smoke/combustible-gas sensor, not a selective smoke detector. Its output depends on warm-up, environment, sensor module, and calibration. The default `SMOKE_ALERT_RAW` is only a placeholder. Observe clean-air readings and perform controlled, safe calibration according to the sensor documentation. Never use an open flame or dangerous gas to test it.

The default temperature alert is 50 °C and is configurable with `TEMP_ALERT_C`. These are demonstration settings, not safety thresholds.

## Repository structure

```text
smart-firefighter-assist-esp32/
├── firmware/
│   └── smart_firefighter_assist.ino
├── docs/
│   └── project-notes.md
├── .gitignore
├── LICENSE
└── README.md
```

## Future improvements

- Add a buzzer and status LED.
- Add a web dashboard over ESP32 Wi-Fi.
- Log sensor readings to CSV or an MQTT broker.
- Add a flame sensor only as an additional experimental input.
- Add a proper, independently engineered suppression subsystem only after a safety review.
- Add a calibrated sensor baseline and configurable alert thresholds.

## Credits and provenance

This code and documentation were newly assembled from the visible component list and project description in the supplied screenshots. They should be reviewed and tested against the actual hardware before use.
