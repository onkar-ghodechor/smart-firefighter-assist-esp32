# Project notes

## System overview

The ESP32 is the controller. The MQ-2 provides an analog sensor reading, while the DHT11 provides temperature and humidity. The OLED is used for local status display. The L298N drives two DC motors.

## Important design limitation

A rising MQ-2 reading does not uniquely identify fire; MQ-2 responds to several gases and is affected by environment and warm-up. The DHT11 is relatively slow and is not a reliable fire detector. Use this as a classroom demonstration only.

## Suggested test sequence

1. Test ESP32 and OLED alone.
2. Test DHT11 readings.
3. Test MQ-2 ADC input with safe, documented calibration procedures.
4. Test L298N and motors separately with wheels lifted.
5. Integrate everything, keeping the motor supply separate from the ESP32 supply and sharing ground.
6. Confirm that an alert stops the motors.
7. Record screenshots of the Serial Monitor and OLED for the repository.

Never test with an open flame, smoke-filled room, or hazardous gas. Do not rely on this prototype for personal or property protection.
