#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <GyverPower.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#include "meteo_config.h"
#include "meteo_protocol.h"

namespace {

const uint8_t kDhtPin = 6;
const uint8_t kRadioSetPin = 7;
const uint8_t kRadioRxPin = 8;  // AVR RX <- HC-12 TX
const uint8_t kRadioTxPin = 9;  // AVR TX -> HC-12 RX
const uint8_t kBatteryPin = A3;

DHT dht(kDhtPin, DHT22);
Adafruit_BMP280 bmp;
SoftwareSerial radio(kRadioRxPin, kRadioTxPin);  // ctor order: RX, TX
uint16_t sequenceNumber = 0;
bool bmpReady = false;

#if METEO_DEBUG
#define TRACE(x) Serial.println(F(x))
#else
#define TRACE(x) do {} while (0)
#endif

void configureBmp() {
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X4,
                  Adafruit_BMP280::FILTER_OFF,
                  Adafruit_BMP280::STANDBY_MS_500);
}

bool ensureBmp() {
  if (bmpReady) return true;
  bmpReady = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID) ||
             bmp.begin(BMP280_ADDRESS, BMP280_CHIPID);
  if (bmpReady) configureBmp();
  return bmpReady;
}

bool readSensors(meteo::Telemetry& result) {
  if (!ensureBmp()) {
    TRACE("BMP280 missing");
    return false;  // Next cycle will retry; never sleep permanently on error.
  }
  const float humidity = dht.readHumidity();
  const float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature) ||
      humidity < 0 || humidity > 100 || temperature < -40 || temperature > 85) {
    TRACE("DHT22 invalid");
    return false;
  }

  if (!bmp.takeForcedMeasurement()) {
    bmpReady = false;
    TRACE("BMP280 measure failed");
    return false;
  }
  const float pressurePa = bmp.readPressure();
  // 1 Pa = 0.00750061683 mmHg; wire stores tenths of mmHg.
  if (isnan(pressurePa) || pressurePa < 40000 || pressurePa > 120000) {
    bmpReady = false;
    TRACE("BMP280 invalid pressure");
    return false;
  }

  analogReference(INTERNAL);
  delay(5);  // Let the AVR internal voltage reference settle.
  (void)analogRead(kBatteryPin);  // Discard first reading after reference switch.
  uint32_t total = 0;
  for (uint8_t i = 0; i < 8; ++i) total += analogRead(kBatteryPin);
  const uint16_t mv = static_cast<uint16_t>(
      (total * METEO_BATTERY_ADC_FULL_SCALE_MV + 4092UL) / (8UL * 1023UL));

  result.node_id = METEO_NODE_ID;
  result.sequence = sequenceNumber++;
  result.temperature_x10_c = static_cast<int16_t>(
      temperature * 10.0f + (temperature >= 0 ? 0.5f : -0.5f));
  result.humidity_x10_percent = static_cast<uint16_t>(humidity * 10.0f + 0.5f);
  result.pressure_x10_mmhg = static_cast<uint16_t>(pressurePa * 0.0750061683f + 0.5f);
  result.battery_mv = mv;
  return meteo::valid(result);
}

// HC-12 SET pin: command mode requires >= 40 ms LOW and >= 80 ms HIGH
// before transparent UART mode; margin included in both transitions.
void enterCommandMode() {
  digitalWrite(kRadioSetPin, LOW);
  delay(100);
  while (radio.available()) (void)radio.read();
}

bool waitForResponse(const char* response, uint16_t timeoutMs) {
  const size_t targetLen = strlen(response);
  size_t matched = 0;
  const uint32_t since = millis();
  while (millis() - since < timeoutMs) {
    if (!radio.available()) continue;
    const char ch = static_cast<char>(radio.read());
    if (ch == response[matched]) {
      ++matched;
      if (matched == targetLen) return true;
    } else {
      matched = (ch == response[0]) ? 1 : 0;
    }
  }
  return false;
}

bool wakeRadio() {
  radio.listen();
  enterCommandMode();
  radio.print(F("AT"));  // Also wakes HC-12 from AT+SLEEP.
  const bool ok = waitForResponse("OK", 500);
  digitalWrite(kRadioSetPin, HIGH);
  delay(120);  // Important: module cannot transmit immediately after SET HIGH.
  return ok;
}

void sleepRadio() {
  enterCommandMode();
  radio.print(F("AT+SLEEP"));
  const bool ok = waitForResponse("OK+SLEEP", 500);
  digitalWrite(kRadioSetPin, HIGH);  // Enters deep sleep only after exiting AT mode.
  if (!ok) TRACE("HC12 sleep acknowledgement missing");
}

}  // namespace

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(kRadioSetPin, HIGH);
  pinMode(kRadioSetPin, OUTPUT);

#if METEO_DEBUG
  Serial.begin(9600);
#endif
  radio.begin(METEO_RADIO_BAUD);
  dht.begin();
  analogReference(INTERNAL);
  power.calibrate();
  power.setSleepMode(POWERDOWN_SLEEP);
  power.setSleepResolution(SLEEP_8192MS);
  delay(2000);  // DHT22 needs a short startup delay.
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  meteo::Telemetry sample = {};
  if (readSensors(sample)) {
    char packet[meteo::kMaxFrameLength];
    if (meteo::encode(sample, packet, sizeof(packet))) {
      if (wakeRadio()) {
        radio.print(packet);
        radio.flush();
        delay(METEO_HC12_TX_SETTLE_MS);  // Allow HC-12 to finish its RF TX burst.
      } else {
        TRACE("HC12 did not wake, packet not sent");
      }
      sleepRadio();
    }
  } else {
    // Even on sensor error, put the module back to sleep; next wake retries.
    sleepRadio();
  }
  digitalWrite(LED_BUILTIN, LOW);
  power.sleepDelay(METEO_OUTDOOR_PERIOD_MS);
}
