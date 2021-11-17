#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TroykaMQ.h>
#include <DHT.h>

#define DHT11_PIN 7
#define MQ135_PIN  A3

static unsigned long timer = millis();
static unsigned long dhtTimer = millis();
static unsigned long debugTimer = millis();
static unsigned long lcdClearTimer = millis();

String outdoorIdentifier;
String outdoorHumidity;
String outdoorTemperature;
String outdoorVoltage;

float localTemperature;
int localHumidity, localCarbonDioxide;
byte mode;

SoftwareSerial radio(4, 3); // TX, RX
LiquidCrystal_I2C lcd(0x27,16,2);
DHT dht(DHT11_PIN, DHT11);
MQ135 mq135(MQ135_PIN);

void blink(void){
  if (millis() - timer > 1000) {
    timer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

}

void debug(void){
  if (millis() - debugTimer > 5000) {
    debugTimer = millis();
    Serial.print("\t localHumidity = " + String(localHumidity));
    Serial.println("\t\t outdoorHumidity = " + String(outdoorHumidity));

    Serial.print("\t localTemperature = " + String(localTemperature));
    Serial.print("\t outdoorTemperature = " + String(outdoorTemperature));
    Serial.println("\t outdoorVoltage = " + String(outdoorVoltage));

    Serial.println("\t MQ-135 CarbonDioxide: " + String(localCarbonDioxide) + " ppm");

    Serial.println(" ");
  }
}

void dhtRead (void){
  if (millis() - dhtTimer > 5000) {
    dhtTimer = millis();
    localHumidity = dht.readHumidity();
    localTemperature = dht.readTemperature();

    localCarbonDioxide = mq135.readCO2();

    lcd.clear();
    mode++;

  }
}

void radioMonitoring(void){
  //input "C01,T=23.30,H=21,V=4.02;"
  if (radio.available()){
    String buff = radio.readString();
    // Serial.println(buff);
    outdoorIdentifier = buff.substring(buff.indexOf('C')+1, buff.indexOf('T')-1);
    outdoorTemperature = buff.substring(buff.indexOf('T')+2, buff.indexOf('H')-2);
    outdoorHumidity = buff.substring(buff.indexOf('H')+2, buff.indexOf('V')-1);
    outdoorVoltage = buff.substring(buff.indexOf('V')+2, buff.indexOf(';'));
  }

}


void resultToLCD(void){

  if ((mode % 2) == true) {
    lcd.setCursor(0,0);
    lcd.print("H=");
    lcd.setCursor(2,0);
    lcd.print(String(localHumidity)+"%");

    lcd.setCursor(8,0);
    lcd.print("H=");
    lcd.setCursor(10,0);
    lcd.print(String(outdoorHumidity)+"%");

    lcd.setCursor(0,1);
    lcd.print("T=");
    lcd.setCursor(2,1);
    lcd.print(localTemperature, 1);

    lcd.setCursor(8,1);
    lcd.print("T=");
    lcd.setCursor(10,1);
    lcd.print(outdoorTemperature);
  }
  else {
    // lcd.setCursor(0,0);
    // lcd.print("G=");
    // lcd.setCursor(2,0);
    // lcd.print(localLPG, 1);

    lcd.setCursor(0,0);
    lcd.print("Co2=");
    lcd.setCursor(4,0);
    lcd.print(localCarbonDioxide, 1);

    // lcd.setCursor(0,1);
    // lcd.print("M=");
    // lcd.setCursor(2,1);
    // lcd.print(localMethane, 1);

    lcd.setCursor(0,1);
    lcd.print("V=");
    lcd.setCursor(2,1);
    lcd.print(outdoorVoltage);
  }
}


void setup() {
  radio.begin(9600);
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("meteoStation");
  lcd.setCursor(3,1);
  lcd.print("Loading...");
  delay(2000);

  dht.begin();
  mq135.calibrate();

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
