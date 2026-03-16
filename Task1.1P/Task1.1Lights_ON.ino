const int BUTTON_PIN = 12;
const int LED_PORCH_PIN = 11;
const int LED_HALL_PIN = 10;
//delay durations before turning off lights.
int duration_porch = 3000;
int duration_hall = 3000;

void TriggerLed() {
  //Function for turning on two LEDs waiting an interval to turn off the first light, and then waiting another interval to turn off the second light.
  //Turn on both lights
  digitalWrite(LED_PORCH_PIN, HIGH);
  digitalWrite(LED_HALL_PIN, HIGH);
  //delay for porch duration
  delay(duration_porch);
  //sets porch light off
  digitalWrite(LED_PORCH_PIN, LOW);
  //wait for remaining duration on hall light.
  delay(duration_hall);
  //sets hall light off
  digitalWrite(LED_HALL_PIN, LOW);
}

void setup() {
  pinMode(LED_PORCH_PIN, OUTPUT);
  pinMode(LED_HALL_PIN, OUTPUT);
  // Using PULLUP: Pin is HIGH normally, LOW when button is pressed
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
}

void loop() {
  // Check if button is pressed (LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    //run LED function
    TriggerLed();
  }
}


