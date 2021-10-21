#include <Arduino.h>
#include <GyverPower.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define DHT22_PIN 6
#define RadioSetPin 7

DHT dht(DHT22_PIN, DHT22);

int humidity = 0;
float temperature = 0;
String data;

int value = 0;
float voltage;

SoftwareSerial radio(8, 9); // TX, RX

void radioSend(void){
    data = String("C01,") + "T=" + String(temperature) + ",H=" + String(humidity) + ",V=" + String(voltage) + ";";
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
