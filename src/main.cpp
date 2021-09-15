#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TroykaMQ.h>
#include <DHT.h>

#define DHT11_PIN 7
#define DHT22_PIN 6

#define MQ2_PIN  A2
#define MQ9_PIN  A3

static unsigned long timer = millis();
static unsigned long dhtTimer = millis();
static unsigned long debugTimer = millis();
static unsigned long lcdClearTimer = millis();

float carbone = 0;      // carbon monoxide/dioxide = C
float humidity = 0;     // humidity = H
float smoke = 0;     // pressure = P
float temperature = 0;  // temperature = T

float h11,t11,h22,t22;

SoftwareSerial radio(4, 3); // TX, RX

LiquidCrystal_I2C lcd(0x27,16,2);

DHT dht11(DHT11_PIN, DHT11);
DHT dht22(DHT22_PIN, DHT22);

MQ2 mq2(MQ2_PIN);
MQ9 mq9(MQ9_PIN);

void blink(void){
  if (millis() - timer > 500) {
    timer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

void debug(void){
  if (millis() - debugTimer > 5000) {
    debugTimer = millis();
    Serial.print("\t h11 = " + String(h11));
    Serial.println("\t\t h22 = " + String(h22));

    Serial.print("\t t11 = " + String(t11));
    Serial.println("\t\t t22 = " + String(t22));

    Serial.print("\t MQ-2 LPG: " + String(mq2.readLPG()) + " ppm");
    Serial.println("\t MQ-9 LPG: " + String(mq9.readLPG()) + " ppm");

    Serial.print("\t MQ-2 Methane: " + String(mq2.readMethane()) + " ppm");
    Serial.println("\t MQ-9 Methane: " + String(mq9.readMethane()) + " ppm");

    Serial.println("\t MQ-2 Smoke: " + String(mq2.readSmoke()) + " ppm");
    Serial.print("\t MQ-2 Hydrogen: " + String(mq2.readHydrogen()) + " ppm");

    Serial.println("\t MQ-9 CarbonMonoxide: " + String(mq9.readCarbonMonoxide()) + " ppm");

    Serial.println(" ");
  }
}

void dhtRead (void){
  if (millis() - dhtTimer > 5000) {
    dhtTimer = millis();
    h11 = dht11.readHumidity();
    t11 = dht11.readTemperature();
    h22 = dht11.readHumidity();
    t22 = dht11.readTemperature();
  }
}

void radioMonitoring(void){
  if (radio.available() && radio.read()=='C'){
    int buff = radio.parseFloat();
    carbone = buff;
    lcd.clear();
  }
  if (radio.available() && radio.read()=='H'){
    int buff = radio.parseFloat();
    humidity = buff;
    lcd.clear();
  }
  if (radio.available() && radio.read()=='S'){
    int buff = radio.parseFloat();
    smoke = buff;
    lcd.clear();
  }
  if (radio.available() && radio.read()=='T'){
    int buff = radio.parseFloat();
    temperature = buff;
    lcd.clear();
  }
}

void resultToLCD(void){

  carbone = mq9.readCarbonMonoxide();
  humidity = h11;
  temperature = t11;
  smoke = mq2.readSmoke();

  lcd.setCursor(0,0);
  lcd.print("C=");
  lcd.setCursor(2,0);
  lcd.print(carbone, 1);

  lcd.setCursor(8,0);
  lcd.print("H=");
  lcd.setCursor(10,0);
  lcd.print(humidity, 1);

  lcd.setCursor(0,1);
  lcd.print("S=");
  lcd.setCursor(2,1);
  lcd.print(smoke, 1);

  lcd.setCursor(8,1);
  lcd.print("T=");
  lcd.setCursor(10,1);
  lcd.print(temperature, 1);
}



void setup() {
  Serial.begin(9600);

  dht11.begin();
  dht22.begin();

  mq2.calibrate();
  mq9.calibrate();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("meteoStation");
  lcd.setCursor(3,1);
  lcd.print("Loading...");

  radio.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  delay(2000);
  lcd.clear();
}

void loop() {
  blink();
  dhtRead();
  radioMonitoring();
  resultToLCD();
  // debug();
}
