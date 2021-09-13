#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

static unsigned long timer = millis();

int carbone = 0;      // carbon monoxide/dioxide = C
int humidity = 0;     // humidity = H
int pressure = 0;     // pressure = P
int temperature = 0;  // temperature = T


SoftwareSerial radio(4, 3); // TX, RX

LiquidCrystal_I2C lcd(0x27,16,2);

void blink(void){
  if (millis() - timer > 500) {
    timer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

void radioMonitoring(void){
  if (radio.available() && radio.read()=='C'){
    int buff = radio.parseInt();
    carbone = buff;
    lcd.clear();
  }
  if (radio.available() && radio.read()=='H'){
    int buff = radio.parseInt();
    humidity = buff;
    lcd.clear();
  }
  if (radio.available() && radio.read()=='P'){
    int buff = radio.parseInt();
    pressure = buff;
    lcd.clear();
  }
  if (radio.available() && radio.read()=='T'){
    int buff = radio.parseInt();
    temperature = buff;
    lcd.clear();
  }
}

void resultToLCD(void){
    lcd.setCursor(0,0);
    lcd.print("C=");
    lcd.setCursor(2,0);
    lcd.print(carbone);

    lcd.setCursor(8,0);
    lcd.print("H=");
    lcd.setCursor(10,0);
    lcd.print(humidity);

    lcd.setCursor(0,1);
    lcd.print("P=");
    lcd.setCursor(2,1);
    lcd.print(pressure);

    lcd.setCursor(8,1);
    lcd.print("T=");
    lcd.setCursor(10,1);
    lcd.print(temperature);
}



void setup() {
  Serial.begin(9600);

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
  radioMonitoring();
  resultToLCD();

}
