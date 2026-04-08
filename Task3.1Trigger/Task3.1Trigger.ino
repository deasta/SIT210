#include <WiFiNINA.h>
#include "secrets.h"
#include <BH1750.h>
#include <Wire.h>


char ssid[] = SECRET_SSID;   // from secrets.h read network SSID (name) 
char pass[] = SECRET_PASS;   // from secrets.h read network password
WiFiClient client;
char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/sunlightState/with/key/dse46SBqral7z81-k_WkLy"; // change your EVENT-NAME and YOUR-KEY
String queryString = "?value1=57&value2=25";

BH1750 lightMeter;
bool lightState = 0;
bool pastState = 0;
int lightThreshold = 500;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lightMeter.begin();
  while (!lightMeter.begin()) {
    Serial.println("BH1750 not found !");
    delay(1000);
  }
    WiFi.begin(ssid, pass);

}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
      
    }
    Serial.println("\nConnected.");
          // connect to web server on port 80:
  if (client.connect(HOST_NAME, 80)) {
    // if connected:
    Serial.println("Connected to server");
  }
  else {// if not connected:
    Serial.println("Server Connection failed");
  }
  
  }
  
  float lux = lightMeter.readLightLevel();
  if (lux > lightThreshold){
    lightState = 1; //if lightstate is above threshold set state to light = TRUE.
  }
  else {
    lightState = 0; // if state is under threshold, the state is set to light = FALSE.
  }
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  Serial.print("lightState: ");
  Serial.println(lightState);
  

  if (lightState != pastState) {
    // make a HTTP request:
    Serial.println("GET " + PATH_NAME + "?value1="+ lightState +"&value1=" + lux + " HTTP/1.1");
    Serial.println("Host: " + String(HOST_NAME)); // Message to serial showing message being sent to client
        // send HTTP header
    client.println("GET " + PATH_NAME + "?value1="+ lightState +"&value1=" + lux + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println(); // end HTTP header
    if (client.connected()) {
      if (client.available()) {
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }
  }
  pastState = lightState; // update past state for the next loop
  delay(15000);
}
