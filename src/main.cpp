#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TroykaMQ.h>
#include <DHT.h>

#define DHT11_PIN 7
#define MQ9_PIN  A3

static unsigned long timer = millis();
static unsigned long dhtTimer = millis();
static unsigned long debugTimer = millis();
static unsigned long lcdClearTimer = millis();

String outdoorIdentifier;
String outdoorHumidity;
String outdoorTemperature;
String outdoorVoltage;

float localTemperature;
int localHumidity, localLPG, localMethane, localCarbonMonoxide;
byte mode;

SoftwareSerial radio(4, 3); // TX, RX
LiquidCrystal_I2C lcd(0x27,16,2);
DHT dht(DHT11_PIN, DHT11);
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
    Serial.print("\t localHumidity = " + String(localHumidity));
    Serial.println("\t\t outdoorHumidity = " + String(outdoorHumidity));

    Serial.print("\t localTemperature = " + String(localTemperature));
    Serial.print("\t outdoorTemperature = " + String(outdoorTemperature));
    Serial.println("\t outdoorVoltage = " + String(outdoorVoltage));

    Serial.println("\t MQ-9 LPG: " + String(localLPG) + " ppm");
    Serial.println("\t MQ-9 Methane: " + String(localMethane) + " ppm");
    Serial.println("\t MQ-9 CarbonMonoxide: " + String(localCarbonMonoxide) + " ppm");

    Serial.println(" ");
  }
}

void dhtRead (void){
  if (millis() - dhtTimer > 10000) {
    dhtTimer = millis();
    localHumidity = dht.readHumidity();
    localTemperature = dht.readTemperature();

    localLPG = mq9.readLPG();
    localCarbonMonoxide = mq9.readCarbonMonoxide();
    localMethane = mq9.readMethane();

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
    lcd.setCursor(0,0);
    lcd.print("G=");
    lcd.setCursor(2,0);
    lcd.print(localLPG, 1);

    lcd.setCursor(8,0);
    lcd.print("C=");
    lcd.setCursor(10,0);
    lcd.print(localCarbonMonoxide, 1);

    lcd.setCursor(0,1);
    lcd.print("M=");
    lcd.setCursor(2,1);
    lcd.print(localMethane, 1);

    lcd.setCursor(8,1);
    lcd.print("V=");
    lcd.setCursor(10,1);
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
  mq9.calibrate();

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
