#include <Arduino.h>
#include <SoftwareSerial.h>

static unsigned long timer = millis();
static unsigned long radioTimer = millis();

char data = 0;

SoftwareSerial radio(4, 3); // TX, RX

void blink(void){
  if (millis() - timer > 1000) {
    timer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

void transmit(void){
  radio.write(data);
  data++;
}

void radioSend(void){
  if (millis() - radioTimer > 2000) {
    radioTimer = millis();
    transmit();
  }
}

void setup() {
  Serial.begin(9600);
  radio.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  blink();
  radioSend();
}
