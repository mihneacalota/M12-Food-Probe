// Released to the public domain
//
// This sketch will read an uploaded file and increment a counter file
// each time the sketch is booted.

// Be sure to install the Pico LittleFS Data Upload extension for the
// Arduino IDE from:
//    https://github.com/earlephilhower/arduino-pico-littlefs-plugin/
// The latest release is available from:
//    https://github.com/earlephilhower/arduino-pico-littlefs-plugin/releases

// Before running:
// 1) Select Tools->Flash Size->(some size with a FS/filesystem)
// 2) Use the Tools->Pico Sketch Data Upload tool to transfer the contents of
//    the sketch data directory to the Pico

#include <LITTLEFS.h>
#include "FS.h"
#include "DFDataset.h"
// #include <Arduino_LSM6DSOX.h>
// #include <PDM.h>

// SSID of your Wifi network, the library currently does not support WPA2 Enterprise networks
const char* ssid = "wifi name bla";
// Password of your Wifi network.
const char* password = "wifi password bla";
// Data Foundry address
const char* datafoundry = "data.id.tue.nl";
// Device name, needs to be unique in batch
const char* device_name = "device_name_1";

// Create connection to dataset with server address, dataset id, and the access token
DFDataset iot(datafoundry, 10000, "sdv98sdv98sydv98ysdv98ysdv98ysdv");

// hardware
const int btn1Pin = D3;
const int btn2Pin = D4;
const int sldrPin = A3;
// default number of output channels
static const char channels = 1;
// default PCM output frequency
static const int frequency = 16000;

// control flow
long timestamp = 0;
long nextLogInstance = 0;
int btn1State = LOW;
int btn2State = LOW;

// data
int btn1Value = 0;
int btn2Value = 0;
int sldrValue = 0;
double acceleration = 0;
double accelFilter = 0;
// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[512];
// Number of audio samples read
volatile int samplesRead;
double sqsum;


void setup() {
  Serial.begin(115200);
  delay(5000);

  // establish Wifi connection
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }

  Serial.println("Connected to the WiFi network");

  // check Wifi periodically
  //  if (WiFi.status() == WL_CONNECTED) {
  // check timestamp
  while (timestamp <= 0) {
    // set time with current run time as offset
    timestamp = WiFi.getTime() - millis();
    if (timestamp > 0) {
      Serial.print("Acquired time: "); Serial.println(timestamp);
      Serial.print("Time offset: "); Serial.println(millis());
    }

    delay(500);
  }
  //  }

  // --------------------------------------------------------------

  pinMode(btn1Pin, INPUT_PULLUP);
  pinMode(btn2Pin, INPUT_PULLUP);
  // TODO: pinMode(sldrPin, ...);

  // --------------------------------------------------------------

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  } else {
    Serial.print("IMU sample rate: ");
    Serial.println(IMU.accelerationSampleRate());
  }

  // --------------------------------------------------------------

  PDM.onReceive(onPDMdata);
  // Optionally set the gain
  // Defaults to 20 on the BLE Sense and -10 on the Portenta Vision Shields
  // PDM.setGain(30);
  // Initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense 32
  // - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shields
  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }

  // --------------------------------------------------------------

  LittleFS.begin();
  
  // --------------------------------------------------------------

  // configure the IoT device id and activity
  iot.device(device_name);
  iot.activity("at CHI 2022");

}

void loop() {

  // --------------------------------------------------------------

  int controlFlowBtns = 0;

  // collect data

  int temp = digitalRead(btn1Pin);
  if (temp != btn1State) {
    if (btn1State == HIGH) {
      Serial.println("Button 1 pressed");
      btn1Value = 1;
      controlFlowBtns++;
    }
    btn1State = temp;
  }

  temp = digitalRead(btn2Pin);
  if (temp != btn2State) {
    if (btn2State == HIGH) {
      Serial.println("Button 2 pressed");
      btn2Value = 1;
      controlFlowBtns++;
    }
    btn2State = temp;
  }

  // TODO: slide poti read
  //  int temp3 = analogRead(sldrPin);
  //  if (temp3 != sldrValue) {
  //    Serial.println("Slider value");
  //    sldrValue = temp3;
  //  }
  //  Serial.print(temp3);

  logAccelerationLevel();

  soundPressure();

  // --------------------------------------------------------------

  // control flow

  // check whether both button have been pressed to connect to backend
  if (controlFlowBtns == 2) {
    Serial.println("Connecting to backend...");
    if (checkConnection()) {
      Serial.println("Connecting successful. Starting upload...");

      File i = LittleFS.open("logdata.txt", "r");
      if (i) {
        while (i.available()) {
          String str = i.readStringUntil('\n');
          const char* buf = str.c_str();
          long t, b1, b2, sl;
          double a, sd;
          int assigned = sscanf(buf, "t %ld b1 %d b2 %d sl %d a %lf sd %lf", &t, &b1, &b2, &sl, &a, &sd);
          if (assigned == 6) {
            // log data to backend
            //iot.clear();
            iot.activity("at CHI 2022");
            iot.addLong("timestamp", t);
            iot.addBool("button1", b1);
            iot.addBool("button2", b2);
            iot.addInt("slider", sl);
            iot.addFloat("acceleration", a);
            iot.addFloat("soundPressure", sd);
            iot.logItem();
          }
          Serial.print("|");
        }
        Serial.println("---------------");
        i.close();
      }

      delay(2000);

      Serial.println("Cleaning local log...");

      LittleFS.remove("logdata.txt");

      Serial.println("Resume logging...");
    }
  }

  // only log every couple of seconds (10s)
  if (nextLogInstance > millis()) {
    delay(20);
    return;
  }

  // --------------------------------------------------------------

  logData();

  // set next log instance in 10 seconds
  nextLogInstance += 10000;

}

void logData() {

  // print to flash storage format
  char buffer[85];
  snprintf(buffer, 84, "t %ld b1 %d b2 %d sl %d a %lf sd %lf\n", timestamp + millis(), btn1Value, btn2Value, sldrValue, acceleration, sqrt(sqsum));
  //  Serial.print(buffer);

  // append to logdata file
  File f = LittleFS.open("logdata.txt", "a");
  if (f) {
    f.write(buffer, strlen(buffer));
    f.close();
  }

  // reset data
  btn1Value = 0;
  btn2Value = 0;
  sldrValue = 0;
  acceleration = 0;
  sqsum = 0;
}

void logAccelerationLevel() {
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
  }

  // filter IMU accel data
  accelFilter += (sqrt((sq(x) + sq(y) + sq(z)) / 3) - accelFilter) / 10;
  acceleration += abs(sqrt((sq(x) + sq(y) + sq(z)) / 3) - accelFilter);
}

void soundPressure() {
  if (samplesRead) {
    double squareMean = 0;
    for (int i = 0; i < samplesRead; i++) {
      squareMean += sq(sampleBuffer[i]);
    }
    sqsum += squareMean / samplesRead;

    // Clear the read count
    samplesRead = 0;
  }
}

bool checkConnection() {
  WiFiClient wifi;
  HttpClient http = HttpClient(wifi, "data.id.tue.nl", 80);
  http.beginRequest();
  http.get("/");
  http.beginBody();
  http.endRequest();

  int httpCode = http.responseStatusCode();
  return httpCode == 200;
}

/**
   Callback function to process the data from the PDM microphone.
   NOTE: This callback is executed as part of an ISR.
   Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
