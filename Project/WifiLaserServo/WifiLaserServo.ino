#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include "secrets.h"
/*
  ****** Deakin University SIT 210 Laser toy project ******
  This code controls an Arduino Nano IoT 33 connected laser mount.
  The laser is moved by adjusting two servos that the laser is mounted to.
  Controls are recieved from a wifi connected remote control using a joystick and buttons.
*/


// *** WiFi AP ***
const char* AP_SSID = SECRET_SSID;
const char* AP_PASS = SECRET_PASS;
const int   UDP_PORT = 4210;

// *** Pins ***
const uint8_t pinServoX = 9;
const uint8_t pinServoY = 10;
const uint8_t pinLaser  = 2;
const uint8_t LED_PIN   = 4;

// *** Servos ***
Servo servoX;
Servo servoY;
int currentX = 90, currentY = 90;
const int SMOOTH_STEP = 3;

// *** UDP ***
WiFiUDP udp;
unsigned long lastPacket       = 0;
const unsigned long TIMEOUT_MS = 500;
bool picoConnected             = false;

// *** LED ***
unsigned long lastActivity   = 0;
const int BLINK_DURATION_MS  = 100;
unsigned long lastSlowFlash  = 0;
const int SLOW_FLASH_MS      = 1000;
bool slowFlashState          = LOW;

void ledActivity() {
  lastActivity = millis();
}

void updateLed() {
  if (!picoConnected) {
    // No Pico. Slow flash
    if (millis() - lastSlowFlash >= SLOW_FLASH_MS) {
      lastSlowFlash  = millis();
      slowFlashState = !slowFlashState;
      digitalWrite(LED_PIN, slowFlashState);
    }
    return;
  }

  // Pico connected. solid on, brief dip off on each packet received
  if (millis() - lastActivity < BLINK_DURATION_MS) {
    digitalWrite(LED_PIN, LOW);    // activity blink
  } else {
    digitalWrite(LED_PIN, HIGH);   // solid on
  }
}

// *** safety functions ***
int smoothMove(int current, int target, int step) {
  // Check if the target is within reach of a single step
  if (abs(target - current) <= step) return target;
  // Move current closer to target by exactly one step
  return current + (target > current ? step : -step);
}

void setLaser(bool state) {
  digitalWrite(pinLaser, state ? HIGH : LOW);
}

void safeTurret() {
  setLaser(false);
  servoX.write(90);
  servoY.write(90);
  currentX = currentY = 90;
}

// *** Setup ***
void setup() {
  Serial.begin(9600);
  //set outputs
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(pinLaser, OUTPUT);
  setLaser(false);
  //set servos
  servoX.attach(pinServoX);
  servoY.attach(pinServoY);
  servoX.write(90);
  servoY.write(90);
  //Wifi/access point start up
  Serial.print("Starting AP: ");
  Serial.println(AP_SSID);

  WiFi.beginAP(AP_SSID, AP_PASS);

  while (WiFi.status() != WL_AP_LISTENING) {
    updateLed();
    delay(50);
    Serial.print(".");
  }
  
  Serial.println("\nAP ready — waiting for Pico.");
  Serial.print("Nano IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(UDP_PORT);
}

// *** Loop ***
void loop() {
  updateLed();

  int packetSize = udp.parsePacket();

  if (packetSize == 3) {
    uint8_t data[3];
    udp.read(data, 3);
    //Assigning data from UDP packet to variables
    int  targetX = constrain(data[0], 0, 180);   // constrain data to accepted Servo values 
    int  targetY = constrain(data[1], 0, 180);
    bool laserOn = data[2];

    // turret speed function
    currentX = smoothMove(currentX, targetX, SMOOTH_STEP);
    currentY = smoothMove(currentY, targetY, SMOOTH_STEP);
    // Update laser
    servoX.write(currentX);
    servoY.write(currentY);
    setLaser(laserOn);

    lastPacket = millis();
    ledActivity();

    if (!picoConnected) {
      picoConnected = true;
      Serial.println("Pico connected.");
    }

    Serial.print("X: ");       Serial.print(currentX);
    Serial.print("°  Y: ");    Serial.print(currentY);
    Serial.print("°  Laser: "); Serial.println(laserOn ? "ON" : "OFF");
  }

  // Timeout — Pico lost
  if (picoConnected && millis() - lastPacket > TIMEOUT_MS) {
    picoConnected = false;
    safeTurret();
    Serial.println("Pico lost — turret safed.");
  }
}