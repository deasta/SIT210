/*
  Task2.1WebHook for SIT210
  
  Description: Reads temperature from DHT22 and light from a resistor which are then sent to a thingspeak channel
  Hardware: Arduino nano Iot
  
*/

#include <WiFiNINA.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include "DHT.h"
//Setting DHT pin and type
#define DHTPIN 12 
#define DHTTYPE DHT22
#define BRIGHTPIN A1 //set resistor pin
DHT dht(DHTPIN, DHTTYPE);


char ssid[] = SECRET_SSID;   // from secrets.h read network SSID (name) 
char pass[] = SECRET_PASS;   // from secrets.h read network password
WiFiClient  client;
// Read Thingspeak connection details for thingspeak channel from secrets.h
unsigned long myChannelNumber = SECRET_CH_ID; 
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;


void setup() {
  Serial.begin(115200);  // Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  pinMode(BRIGHTPIN, INPUT);
  ThingSpeak.begin(client);  //Initialize ThingSpeak
  dht.begin(); //Initialise DHT
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  int brightness = analogRead(BRIGHTPIN); // read brightness from analog pin
  float temperature = dht.readTemperature(); //read from DHT temperature sensor
    if (isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  //print temp and brightness locally
  Serial.print(F(" Temperature: "));
  Serial.print(temperature);
  Serial.print("  Brightness: ");
  Serial.println(brightness);

  // update Thinspeak fields with values
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, brightness);

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful."); //update channel on HTTP 200 OK
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  
  delay(20000); // Wait 20 seconds to update the channel again
}
