// TODO:
// переназначить все порты

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Servo.h>

#ifndef APSSID
#define APSSID "TREE"
#define APPSK  "garik816"
#endif

#define DFBusyPin 6 //digitalRead(DFBusyPin)=0 - busy;
#define feedback A0
#define eyePin 11
#define mouthPin 12

#define feedbackSence 100

static unsigned long timer = millis();
static unsigned long eyeTimer = millis();
static unsigned long commandTimer = millis();

const char *ssid = APSSID;
const char *password = APPSK;

bool DFstatus = false;
int phrase = 0;
int eyePos = 0;
int mouthPos = 0;

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

Servo mouthServo;
Servo eyeServo;
SoftwareSerial DFplayerSoftwareSerial(7, 8); // RX, TX
DFRobotDFPlayerMini voiceDFPlayer;

void blink(void){
  if (millis() - timer > 1000) {
    timer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    DFstatus = digitalRead(DFBusyPin);
  }
}

void eyeBlinkAnimation(void){
  if (millis() - eyeTimer > 10000) {
    eyeTimer = millis();
    eyePos = 0;
    eyeServo.write(eyePos);
    Serial.println("eyeAnimClose"); //debug
    delay(500);
  }
  else eyePos = 120;
  eyeServo.write(eyePos);
  Serial.println("eyeAnimOpen"); //debug
}

void mouthAnimation(void){
  mouthPos = map(analogRead(feedback), 0, 512, 0, 180);
  mouthServo.write(mouthPos);
  Serial.println("mAnim"); //debug
}


void voice(void){
  if (phrase!=0){
    voiceDFPlayer.play(phrase);
    phrase=false;
    Serial.println("voice"); //debug
  }
}

String SendHTML(void)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Phrase Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<fieldset><legend>control</legend>\n";
  ptr +="<form method=\"get\" action=\"\">\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"001\" type=\"submit\" name=\"send_t\">Track 001</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"002\" type=\"submit\" name=\"send_t\">Track 002</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"003\" type=\"submit\" name=\"send_t\">Track 003</button><br><br>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"004\" type=\"submit\" name=\"send_t\">Track 004</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"005\" type=\"submit\" name=\"send_t\">Track 005</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"006\" type=\"submit\" name=\"send_t\">Track 006</button><br><br>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"007\" type=\"submit\" name=\"send_t\">Track 007</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"008\" type=\"submit\" name=\"send_t\">Track 008</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"009\" type=\"submit\" name=\"send_t\">Track 009</button><br><br>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"010\" type=\"submit\" name=\"send_t\">Track 010</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"011\" type=\"submit\" name=\"send_t\">Track 011</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"012\" type=\"submit\" name=\"send_t\">Track 012</button><br><br>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"013\" type=\"submit\" name=\"send_t\">Track 013</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"014\" type=\"submit\" name=\"send_t\">Track 014</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"015\" type=\"submit\" name=\"send_t\">Track 015</button><br><br>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"016\" type=\"submit\" name=\"send_t\">Track 016</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"017\" type=\"submit\" name=\"send_t\">Track 017</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"018\" type=\"submit\" name=\"send_t\">Track 018</button><br><br>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"019\" type=\"submit\" name=\"send_t\">Track 019</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"020\" type=\"submit\" name=\"send_t\">Track 020</button>\n";
  ptr +="<button style=\"width:100px;height:32px;margin-right:10px\" value=\"021\" type=\"submit\" name=\"send_t\">Track 021</button><br><br>\n";
  ptr +="</form></fieldset>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void listnenOfClient(void){
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  String req = client.readStringUntil('\r');
  client.flush();
  if (req.indexOf("/?send_t=001") != -1){
  }
  if (req.indexOf("/?send_t=002") != -1){
  }
}


void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.begin(115200);
  DFplayerSoftwareSerial.begin(9600);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  mouthServo.attach(mouthPin);
  mouthServo.write(0);
  eyeServo.attach(eyePin);
  eyeServo.write(0);

  if (!voiceDFPlayer.begin(DFplayerSoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }

  if (digitalRead(DFBusyPin)==0){voiceDFPlayer.stop();}
  voiceDFPlayer.volume(30);  //Set volume value. From 0 to 30
  delay(250);
}

void loop() {
  blink();
  server.handleClient();

  if(digitalRead(DFBusyPin)==false){
    eyeBlinkAnimation();
    mouthAnimation();
    delay(feedbackSence);
  }
  else {
    mouthServo.write(0);
    eyeServo.write(0);
  }

  voice();
}
