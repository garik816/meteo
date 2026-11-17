#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#include "meteo_config.h"
#include "meteo_protocol.h"

namespace {

const uint8_t kDhtPin = 7;
const uint8_t kMq135Pin = A3;
const uint8_t kRadioRxPin = 4;  // AVR RX <- HC-12 TX
const uint8_t kRadioTxPin = 3;  // AVR TX -> HC-12 RX
const uint32_t kLocalSampleMs = 10000UL;
const uint32_t kDisplayUpdateMs = 500UL;
const uint32_t kPageDurationMs = 8000UL;

SoftwareSerial radio(kRadioRxPin, kRadioTxPin);  // ctor order: RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(kDhtPin, DHT11);
meteo::StreamDecoder stream;
meteo::Telemetry outdoor = {};

bool haveOutdoor = false;
bool haveLocal = false;
bool haveMqSample = false;
uint32_t lastOutdoorMs = 0;
uint32_t lastLocalMs = 0;
uint32_t lastDisplayMs = 0;
uint16_t mqAdc = 0;
int16_t localTempX10 = 0;
uint16_t localRhX10 = 0;
char cachedRows[2][17] = {{0}, {0}};

void formatSignedTenths(int16_t value, char* dest, size_t capacity) {
  const bool negative = value < 0;
  const uint16_t magnitude = negative ? static_cast<uint16_t>(-value)
                                      : static_cast<uint16_t>(value);
  snprintf(dest, capacity, "%s%u.%u", negative ? "-" : "",
           static_cast<unsigned>(magnitude / 10),
           static_cast<unsigned>(magnitude % 10));
}

void renderLine(uint8_t row, const char* message) {
  char padded[17];
  memset(padded, ' ', 16);
  const size_t length = strlen(message);
  memcpy(padded, message, length < 16 ? length : 16);
  padded[16] = '\0';
  if (memcmp(padded, cachedRows[row], sizeof(padded)) == 0) return;
  lcd.setCursor(0, row);
  lcd.print(padded);  // Update in place: no lcd.clear() flicker.
  memcpy(cachedRows[row], padded, sizeof(padded));
}

bool outdoorFresh(uint32_t now) {
  return haveOutdoor && (now - lastOutdoorMs < METEO_OUTDOOR_STALE_MS);
}

void readLocalSensors(uint32_t now) {
  const float humidity = dht.readHumidity();
  const float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature) ||
      humidity < 0 || humidity > 100 || temperature < -40 || temperature > 85) {
    haveLocal = false;
  } else {
    localTempX10 = static_cast<int16_t>(temperature * 10.0f +
                       (temperature >= 0 ? 0.5f : -0.5f));
    localRhX10 = static_cast<uint16_t>(humidity * 10.0f + 0.5f);
    haveLocal = true;
  }

  if (now >= METEO_MQ135_WARMUP_MS) {
    (void)analogRead(kMq135Pin);
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 4; ++i) sum += analogRead(kMq135Pin);
    mqAdc = (sum + 2) / 4;
    haveMqSample = true;
  }
}

void receiveRadio() {
  while (radio.available()) {
    const char next = static_cast<char>(radio.read());
    meteo::Telemetry packet;
    if (!stream.feed(next, packet) || packet.node_id != METEO_NODE_ID) continue;
    outdoor = packet;
    haveOutdoor = true;
    lastOutdoorMs = millis();
#if METEO_DEBUG
    Serial.print(F("Meteo RX seq="));
    Serial.println(outdoor.sequence);
#endif
  }
}

void refreshDisplay(uint32_t now) {
  char top[24];
  char bottom[24];
  const bool fresh = outdoorFresh(now);
  if ((now / kPageDurationMs) % 2 == 0) {
    if (haveLocal) {
      char temp[10];
      formatSignedTenths(localTempX10, temp, sizeof(temp));
      snprintf(top, sizeof(top), "IN  %sC %u%%", temp,
               static_cast<unsigned>(localRhX10 / 10));
    } else {
      strcpy(top, "IN  sensor ERROR");
    }
    if (fresh) {
      char temp[10];
      formatSignedTenths(outdoor.temperature_x10_c, temp, sizeof(temp));
      snprintf(bottom, sizeof(bottom), "OUT %sC %u%%", temp,
               static_cast<unsigned>(outdoor.humidity_x10_percent / 10));
    } else {
      strcpy(bottom, haveOutdoor ? "OUT LINK LOST" : "OUT waiting...");
    }
  } else {
    if (fresh) {
      snprintf(top, sizeof(top), "P:%u.%u V:%u.%02u",
               static_cast<unsigned>(outdoor.pressure_x10_mmhg / 10),
               static_cast<unsigned>(outdoor.pressure_x10_mmhg % 10),
               static_cast<unsigned>(outdoor.battery_mv / 1000),
               static_cast<unsigned>((outdoor.battery_mv % 1000) / 10));
    } else {
      strcpy(top, haveOutdoor ? "P/V: LINK LOST" : "P/V: waiting...");
    }
    if (haveMqSample) {
      // Raw ADC only. MQ135 does not selectively measure absolute CO2 ppm.
      snprintf(bottom, sizeof(bottom), "MQ135 ADC:%u",
               static_cast<unsigned>(mqAdc));
    } else {
      strcpy(bottom, "MQ135 warming up");
    }
  }
  renderLine(0, top);
  renderLine(1, bottom);
}

}  // namespace

void setup() {
#if METEO_DEBUG
  Serial.begin(9600);
#endif
  radio.begin(METEO_RADIO_BAUD);
  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Meteo 2.0"));
  lcd.setCursor(0, 1);
  lcd.print(F("Starting..."));
  delay(1200);
  lcd.clear();  // Only once at startup, never during normal page rotation.
  readLocalSensors(millis());
  lastLocalMs = millis();
  refreshDisplay(millis());
  lastDisplayMs = millis();
}

void loop() {
  receiveRadio();
  const uint32_t now = millis();
  if (now - lastLocalMs >= kLocalSampleMs) {
    lastLocalMs = now;
    readLocalSensors(now);
  }
  if (now - lastDisplayMs >= kDisplayUpdateMs) {
    lastDisplayMs = now;
    refreshDisplay(now);
  }
  digitalWrite(LED_BUILTIN, (now / 1000UL) % 2);
}
