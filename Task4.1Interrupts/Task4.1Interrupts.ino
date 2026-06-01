#include <BH1750.h>
#include <Wire.h>
const int PIN_MOTION =3;
const int PIN_SWITCH = 2;
const int PIN_LED_PORCH = 11;
const int PIN_LED_HALL = 10;

const unsigned long duration_porch = 3000;
const unsigned long duration_hall = 3000;
const float lightThreshold = 50;

volatile bool lightsOnMotion = 0;
volatile bool lightsOnSwitch = 0;
volatile bool cancelLights = 0;
volatile unsigned long switchPressTime = 0;
volatile bool switchPressed = 0;



BH1750 lightMeter;

void triggerOnMotion() {
  lightsOnMotion = true;
}

void triggerOnSwitch() {
  switchPressed = true;
  switchPressTime = millis();
}

void TriggerLed() {
  cancelLights = false;
  digitalWrite(PIN_LED_PORCH, HIGH);
  digitalWrite(PIN_LED_HALL, HIGH);
  Serial.println("lights triggered on.");

  for (unsigned long i = 0; i < duration_porch; i += 50) {
    // Check switch press directly inside loop
    if (switchPressed) {
      Serial.println("Cancelling porch");
      switchPressed = false;
      digitalWrite(PIN_LED_PORCH, LOW);
      digitalWrite(PIN_LED_HALL, LOW);
      lightsOnMotion = false;
      lightsOnSwitch = false;
      return;
    }
    delay(50);
  }

  digitalWrite(PIN_LED_PORCH, LOW);

  for (unsigned long i = 0; i < duration_hall; i += 50) {
    // Check switch press directly inside loop
    if (switchPressed) {
      Serial.println("Cancelling hall");
      switchPressed = false;
      digitalWrite(PIN_LED_HALL, LOW);
      lightsOnMotion = false;
      lightsOnSwitch = false;
      return;
    }
    delay(50);
  }

  digitalWrite(PIN_LED_HALL, LOW);
  Serial.println("Timer done. Lights off.");
  lightsOnMotion = false;
  lightsOnSwitch = false;
  
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  pinMode(PIN_SWITCH, INPUT);
  pinMode(PIN_MOTION, INPUT);
  pinMode(PIN_LED_PORCH, OUTPUT);
  pinMode(PIN_LED_HALL, OUTPUT);
  
  while (!lightMeter.begin()) {
    Serial.println("BH1750 not found !");
    delay(1000);
  }
  delay(500);
  //interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_MOTION), triggerOnMotion, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_SWITCH), triggerOnSwitch, RISING);
  
  Serial.println("LED test");
  digitalWrite(PIN_LED_PORCH, HIGH);
  digitalWrite(PIN_LED_HALL, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_PORCH, LOW);
  digitalWrite(PIN_LED_HALL, LOW);

}


void loop() {

  if (switchPressed && (millis() - switchPressTime > 200)) {
    switchPressed = false;
    Serial.println("Switch triggered");

    if (lightsOnMotion || lightsOnSwitch) {
      cancelLights = true;
      lightsOnMotion = false;
      lightsOnSwitch = false;
    } else {
      lightsOnSwitch = true;
      cancelLights = false;
    }
  }

  if (lightsOnMotion) {
    Serial.println("Motion detected");
  }

  // Interrupts interferring with light read 
  noInterrupts();
  float lux = lightMeter.readLightLevel();
  interrupts();
  // Delay to stop sensor getting overwhelmed
  if (lux < 0) {
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    delay(500);
    return;
  }

  bool isDark = lux < lightThreshold;
  // Serial.print("Light: ");
  // Serial.print(lux);
  // Serial.println(" lx");
  // Serial.print("Is it Dark? ");
  // Serial.println(isDark);

  if (lightsOnSwitch || (isDark && lightsOnMotion)) {
    TriggerLed();
  }
  delay(200);  // Give BH1750 time between reads
}



