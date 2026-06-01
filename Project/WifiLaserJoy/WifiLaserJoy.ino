#include <WiFi.h>
#include <WiFiUdp.h>
#include <EventJoystick.h>
#include <EventButton.h>
#include "secrets.h"

// WiFi / UDP 
const char* AP_SSID  = SECRET_SSID;
const char* AP_PASS  = SECRET_PASS;
const char* NANO_IP  = "192.168.4.1";
const int   UDP_PORT = 4210;

WiFiUDP udp;

// Pins
const uint8_t pinJoyX  = A0;
const uint8_t pinJoyY  = A1;
const uint8_t pinJoySw = 22;
const uint8_t pinReset = 15;
const uint8_t LED_PIN  = 16;

// Turret state
static uint8_t xAngle  = 90;
static uint8_t yAngle  = 90;
static bool    laserOn = false;
bool wifiConnected     = false;

// LED 
//  Blinks slowly when connecting
//  LED is solid while program is running and connected.
//  Blinks rapidly when sending packets
unsigned long lastActivity   = 0;
const int BLINK_DURATION_MS  = 100;
unsigned long lastSlowFlash  = 0;
const int SLOW_FLASH_MS      = 1000;
bool slowFlashState          = LOW;

// *** LED functinos
void ledActivity() {
  lastActivity = millis();
}

void updateLed() {
  if (!wifiConnected) {
    // Not connected. slow flash
    if (millis() - lastSlowFlash >= SLOW_FLASH_MS) {
      lastSlowFlash  = millis();
      slowFlashState = !slowFlashState;
      digitalWrite(LED_PIN, slowFlashState);
    }
    return;
  }

  // Connected. solid on, brief dip off on each packet send
  if (millis() - lastActivity < BLINK_DURATION_MS) {
    digitalWrite(LED_PIN, LOW);    // activity blink
  } else {
    digitalWrite(LED_PIN, HIGH);   // solid on
  }
}

// *** Input devices ***
EventJoystick joyJoystick(pinJoyX, pinJoyY);
EventButton   joyButton(pinJoySw);
EventButton   resetButton(pinReset);

// *** Callbacks *** 
void onJoyButtonEvent(InputEventType et, EventButton& eb) {
  if (et == InputEventType::CLICKED) {
    laserOn = !laserOn;
    Serial.print("Laser toggled: ");
    Serial.println(laserOn ? "ON" : "OFF");
  }
}

void onResetEvent(InputEventType et, EventButton& eb) {
  if (et == InputEventType::CLICKED) {
    laserOn = false;
    xAngle  = 90;
    yAngle  = 90;
    Serial.println("Reset — centred, laser off.");
  }
}

void onJoystickEvent(InputEventType et, EventJoystick& ea) {
  // When joystick moves left/right
  // Update X with value
  if (et == InputEventType::CHANGED_X) {
    xAngle = (uint8_t)map(ea.x.position(), -25, 25, 40, 140);
    Serial.print("X angle: ");
    Serial.println(xAngle);
  }
  // When Joystick moves up/down
  // Update Y value
  if (et == InputEventType::CHANGED_Y) {
    yAngle = (uint8_t)map(ea.y.position(), -25, 25, 40, 140);
    Serial.print("Y angle: ");
    Serial.println(yAngle);
  }
}

// *** WiFi ***
void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(AP_SSID);
  wifiConnected = false;
  WiFi.begin(AP_SSID, AP_PASS);

  unsigned long startAttempt = millis();
  const unsigned long TIMEOUT_MS = 15000;

  while (WiFi.status() != WL_CONNECTED) {
    updateLed();
    delay(50);
    Serial.print(".");
    if (millis() - startAttempt >= TIMEOUT_MS) {
      Serial.println("\nWiFi timeout — will retry in loop.");
      return;
    }
  }

  wifiConnected = true;
  Serial.println("\nConnected!");
}

// *** Setup ***
void setup() {
  Serial.begin(9600);
  // set pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // set inputs
  joyJoystick.begin();
  joyButton.begin();
  resetButton.begin();
  //set callbacks
  joyJoystick.setCallback(onJoystickEvent);
  joyJoystick.setStartValues();
  joyButton.setCallback(onJoyButtonEvent);
  resetButton.setCallback(onResetEvent);

  //begin wifi
  connectWiFi();
  udp.begin(UDP_PORT);

  Serial.println("Joystick remote ready.");
}

// Loop 
void loop() {
  // Reconnect if WiFi drops
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost — reconnecting...");
    wifiConnected = false;
    //resets values to safe defaults
    laserOn = false;
    xAngle  = 90;
    yAngle  = 90;
    connectWiFi();
  }
  //show status. Slow flash = Not connected. Solid means connected but not sending. Flickering means sending
  updateLed(); 

  //check input states.
  joyJoystick.update();
  joyButton.update();
  resetButton.update();

  uint8_t payload[3] = {
    xAngle,
    yAngle,
    (uint8_t)laserOn
  };
  //sends payload containig updated values.
  udp.beginPacket(NANO_IP, UDP_PORT);
  udp.write(payload, 3);
  udp.endPacket();
  ledActivity();

  Serial.print("X: ");       Serial.print(xAngle);
  Serial.print("  Y: ");     Serial.print(yAngle);
  Serial.print("  Laser: "); Serial.println(laserOn ? "ON" : "OFF");

  delay(50);
}