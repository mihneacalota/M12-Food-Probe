#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>

static const int RXPin = 3, TXPin = 1;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup(){
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Serial.println("Start Setup");

}

void loop(){
  // This sketch displays information every time a new sentence is correctly encoded.
  Serial.println("Loop Start");
  Serial.println(ss.read());
  while (ss.available() > 0){
    Serial.println("We have GPS");
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
    }
  }
  delay(1000);
}