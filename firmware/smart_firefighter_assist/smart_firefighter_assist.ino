/*
  Smart Firefighter Assist Using ESP32
  Reconstructed educational starter firmware.

  Assumed hardware:
    MQ-2 AO -> GPIO 34 (through a suitable voltage divider/signal conditioner)
    DHT11 DATA -> GPIO 4
    SSD1306 OLED I2C -> SDA 21, SCL 22
    L298N: ENA 25, IN1 26, IN2 27, ENB 14, IN3 16, IN4 17

  IMPORTANT:
  - Verify the pin map against your actual wiring.
  - ESP32 GPIOs are 3.3 V only. Do not feed 5 V into any GPIO.
  - Motors default to STOP. Serial commands move the rover only while no alert is active.
  - Educational prototype only; not a certified fire-safety device.
*/

#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- Pin configuration (verify before use) ----------------
constexpr int MQ2_PIN = 34;       // ADC1 input-only pin
constexpr int DHT_PIN = 4;
constexpr int DHT_TYPE = DHT11;

constexpr int MOTOR_ENA = 25;     // PWM speed, left motor
constexpr int MOTOR_IN1 = 26;
constexpr int MOTOR_IN2 = 27;
constexpr int MOTOR_ENB = 14;     // PWM speed, right motor
constexpr int MOTOR_IN3 = 16;
constexpr int MOTOR_IN4 = 17;

constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// ---------------- Demonstration settings; calibrate for your hardware --
constexpr int SMOKE_ALERT_RAW = 1800; // Placeholder ADC threshold (0..4095)
constexpr float TEMP_ALERT_C = 50.0f;
constexpr uint8_t MOTOR_SPEED = 150;  // 0..255

// ESP32 Arduino core 3.x uses ledcAttach/ledcWrite(pin, duty).
// For older core versions, use the compatibility helpers below.
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  void setupMotorPwm() {
    ledcAttach(MOTOR_ENA, 1000, 8);
    ledcAttach(MOTOR_ENB, 1000, 8);
  }
  void writeMotorPwm(int pin, uint8_t duty) { ledcWrite(pin, duty); }
#else
  constexpr int PWM_FREQ = 1000;
  constexpr int PWM_RESOLUTION = 8;
  constexpr int PWM_CHANNEL_LEFT = 0;
  constexpr int PWM_CHANNEL_RIGHT = 1;
  void setupMotorPwm() {
    ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_ENA, PWM_CHANNEL_LEFT);
    ledcAttachPin(MOTOR_ENB, PWM_CHANNEL_RIGHT);
  }
  void writeMotorPwm(int pin, uint8_t duty) {
    ledcWrite(pin == MOTOR_ENA ? PWM_CHANNEL_LEFT : PWM_CHANNEL_RIGHT, duty);
  }
#endif

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

bool oledReady = false;
bool alertActive = false;
int smokeRaw = 0;
float temperatureC = NAN;
float humidityPct = NAN;
unsigned long lastReadMs = 0;
constexpr unsigned long SENSOR_INTERVAL_MS = 2000;

void stopMotors() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  writeMotorPwm(MOTOR_ENA, 0);
  writeMotorPwm(MOTOR_ENB, 0);
}

void driveForward() {
  digitalWrite(MOTOR_IN1, HIGH); digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, HIGH); digitalWrite(MOTOR_IN4, LOW);
  writeMotorPwm(MOTOR_ENA, MOTOR_SPEED);
  writeMotorPwm(MOTOR_ENB, MOTOR_SPEED);
}

void driveBackward() {
  digitalWrite(MOTOR_IN1, LOW); digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(MOTOR_IN3, LOW); digitalWrite(MOTOR_IN4, HIGH);
  writeMotorPwm(MOTOR_ENA, MOTOR_SPEED);
  writeMotorPwm(MOTOR_ENB, MOTOR_SPEED);
}

void turnLeft() {
  digitalWrite(MOTOR_IN1, LOW);  digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(MOTOR_IN3, HIGH); digitalWrite(MOTOR_IN4, LOW);
  writeMotorPwm(MOTOR_ENA, MOTOR_SPEED);
  writeMotorPwm(MOTOR_ENB, MOTOR_SPEED);
}

void turnRight() {
  digitalWrite(MOTOR_IN1, HIGH); digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);  digitalWrite(MOTOR_IN4, HIGH);
  writeMotorPwm(MOTOR_ENA, MOTOR_SPEED);
  writeMotorPwm(MOTOR_ENB, MOTOR_SPEED);
}

void updateSensors() {
  smokeRaw = analogRead(MQ2_PIN);
  temperatureC = dht.readTemperature();
  humidityPct = dht.readHumidity();

  const bool smokeAlert = smokeRaw >= SMOKE_ALERT_RAW;
  const bool temperatureAlert =
      !isnan(temperatureC) && temperatureC >= TEMP_ALERT_C;

  alertActive = smokeAlert || temperatureAlert;
  if (alertActive) {
    stopMotors();
  }
}

void updateDisplay() {
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("SMART FIREFIGHTER ASSIST"));
  display.println(F("------------------------"));

  display.print(F("Smoke ADC: "));
  display.println(smokeRaw);

  display.print(F("Temp: "));
  if (isnan(temperatureC)) display.println(F("Sensor error"));
  else {
    display.print(temperatureC, 1);
    display.println(F(" C"));
  }

  display.print(F("Humidity: "));
  if (isnan(humidityPct)) display.println(F("Sensor error"));
  else {
    display.print(humidityPct, 1);
    display.println(F(" %"));
  }

  display.println();
  display.print(F("Status: "));
  display.println(alertActive ? F("ALERT - MOTORS STOP") : F("NORMAL"));
  display.display();
}

void printSerialStatus() {
  Serial.print(F("MQ2 ADC="));
  Serial.print(smokeRaw);
  Serial.print(F(" | Temp C="));
  if (isnan(temperatureC)) Serial.print(F("N/A"));
  else Serial.print(temperatureC, 1);
  Serial.print(F(" | Humidity %="));
  if (isnan(humidityPct)) Serial.print(F("N/A"));
  else Serial.print(humidityPct, 1);
  Serial.print(F(" | Status="));
  Serial.println(alertActive ? F("ALERT") : F("NORMAL"));
}

void handleSerialCommand() {
  if (!Serial.available()) return;

  const char command = toupper(static_cast<char>(Serial.read()));
  if (command == '\n' || command == '\r' || command == ' ') return;

  if (alertActive) {
    stopMotors();
    Serial.println(F("Alert active: movement command blocked. Resolve the cause and reset safely."));
    return;
  }

  switch (command) {
    case 'F': driveForward();  Serial.println(F("Forward")); break;
    case 'B': driveBackward(); Serial.println(F("Backward")); break;
    case 'L': turnLeft();      Serial.println(F("Left")); break;
    case 'R': turnRight();     Serial.println(F("Right")); break;
    case 'S': stopMotors();    Serial.println(F("Stopped")); break;
    default:
      Serial.println(F("Commands: F=forward, B=back, L=left, R=right, S=stop"));
      break;
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(MQ2_PIN, INPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  setupMotorPwm();
  stopMotors();

  dht.begin();
  Wire.begin(21, 22);
  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

  Serial.println();
  Serial.println(F("Smart Firefighter Assist - ESP32"));
  Serial.println(F("Motors are stopped at startup."));
  Serial.println(F("Serial commands: F, B, L, R, S"));
  if (!oledReady) Serial.println(F("OLED not detected; check I2C wiring/address."));
}

void loop() {
  handleSerialCommand();

  const unsigned long now = millis();
  if (now - lastReadMs >= SENSOR_INTERVAL_MS) {
    lastReadMs = now;
    updateSensors();
    updateDisplay();
    printSerialStatus();
  }
}
