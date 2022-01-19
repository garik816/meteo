#include <Arduino.h>
#include <GyverPower.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define DHT22_PIN 6
#define RadioSetPin 7

DHT dht(DHT22_PIN, DHT22);
Adafruit_BMP280 bmp; // I2C

int humidity = 0;
float temperature = 0;
float Pressure = 0;
String data;

int value = 0;
float voltage;

SoftwareSerial radio(8, 9); // TX, RX

void radioSend(void){
    data = String("C01,") + "T=" + String(temperature) + ",H=" + String(humidity) + ",V=" + String(voltage) +  ",P=" + String(Pressure) + ";";
    radio.println(data);
    Serial.println(data);
}

void batteryRead(void){
  value = analogRead(A3);
  voltage = value * 4.075/1023;
}


void setup() {
  power.calibrate(power.getMaxTimeout());
  power.setSleepMode(POWERDOWN_SLEEP);
  Serial.begin(9600);
  radio.begin(9600);
  dht.begin();
  analogReference(INTERNAL);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RadioSetPin, OUTPUT);

  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(RadioSetPin, LOW);
  delay(50);
  radio.println("AT");
  delay(50);
  digitalWrite(RadioSetPin, HIGH);
  delay(50);

  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  if (bmp.takeForcedMeasurement()) {

    Pressure = bmp.readPressure()*0.0075;

  }

  bmp.setSampling(Adafruit_BMP280::MODE_SLEEP);

  batteryRead();
  radioSend();
  delay(350);
  digitalWrite(LED_BUILTIN, LOW);

  digitalWrite(RadioSetPin, LOW);
  delay(50);
  radio.println("AT+SLEEP");
  delay(50);
  digitalWrite(RadioSetPin, HIGH);
  delay(50);

  power.sleepDelay(60000);

}

