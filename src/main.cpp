#include <Arduino.h>

//define screen stuff
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <SPI.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);



//define GPS pins
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

static const int RXPin = 3, TXPin = 1;
static const uint32_t GPSBaud = 9600;
HardwareSerial neogps(1);
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);


//define OOCSI stuff
#include <OOCSI.h>
#include <HTTPClient.h>

//name of the current device on the OOCSI network
const char *OOCSIName = "M12_Food_Probe_1";
//the address of the OOCSI server here
const char *hostserver = "oocsi.id.tue.nl";
//name of the general oocsi channel
const char *DF_Channel = "food_probe_data";

// SSID of your Wifi network, the library currently does not support WPA2 Enterprise networks
const char* ssid = "The Donut Wifi";
// Password of your Wifi network.
const char* password = "donutismad";

// OOCSI reference for the entire sketch
OOCSI oocsi = OOCSI();


//define button pins
int greenButton=14;
int greenButtonLED=25;
const char* greenButtonName="green button";
int greenButtonState = 0, greenButtonStatePrev=0;

int redButton=13;
int redButtonLED=26;
const char* redButtonName="red button";
int redButtonState = 0, redButtonStatePrev=0;

int blueButton=2;
int blueButtonLED=0;
const char* blueButtonName="blue button";
int blueButtonState = 0, blueButtonStatePrev=0;



int yellowButton=4;
const char* yellowButtonName="yellow open button";
int yellowButtonState = 0;

int yellowSwitch=12;
const char* yellowSwitchName="yellow switch";
int yellowSwitchState = 0, yellowSwitchStatePrev=0;



int cameraButton=17;
const char* cameraButtonName="Shutter";
int cameraButtonState = 0;



int recorderButton=16;
const char* recorderButtonName="Recorder";
int recorderButtonState = 0, recorderButtonStatePrev=0;



void setup() {
  //begin u8g2
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.enableUTF8Print();

  //egin GPS
  ss.begin(GPSBaud);

  //pin modes definitions
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(greenButtonLED, OUTPUT);
  pinMode(redButton, INPUT_PULLUP);
  pinMode(redButtonLED, OUTPUT);
  pinMode(blueButton, INPUT_PULLUP);
  pinMode(blueButtonLED, OUTPUT);

  pinMode(yellowButton, INPUT_PULLUP);
  pinMode(yellowSwitch, INPUT_PULLUP);

  pinMode(cameraButton, INPUT_PULLUP);
  pinMode(recorderButton, INPUT_PULLUP);

  //begin serial
  Serial.begin(115200);
  Serial.println ("Hello");

  //WIFI Connection
  oocsi.connect(OOCSIName, hostserver, ssid, password);
  Serial.print("Successfully connected to: ");
  Serial.println(ssid);

}


//Custom Functions ---------------------------------------------------------------------

void listenForButtons()
{
  //listen for toggle buttons module
  greenButtonStatePrev=greenButtonState;
  if (digitalRead(greenButton)==HIGH)
  greenButtonState=1;
  else
  greenButtonState=0;

  redButtonStatePrev=redButtonState;
  if (digitalRead(redButton)==HIGH)
  redButtonState=1;
  else
  redButtonState=0;

  blueButtonStatePrev=blueButtonState;
  if (digitalRead(blueButton)==HIGH)
  blueButtonState=1;
  else
  blueButtonState=0;


  //listen for open tracker button and switch
  if (digitalRead(yellowButton)==LOW)
  yellowButtonState=1;
  else
  yellowButtonState=0;

  yellowSwitchStatePrev=yellowSwitchState;
  if (digitalRead(yellowSwitch)==LOW)
  yellowSwitchState=1;
  else
  yellowSwitchState=0;

  //listen for recorder button
  recorderButtonStatePrev=recorderButtonState;
  if (digitalRead(recorderButton)==LOW)
  recorderButtonState=1;
  else
  recorderButtonState=0;

  //listen for camera shutter button
  if (digitalRead(cameraButton)==LOW)
  cameraButtonState=1;
  else
  cameraButtonState=0;
}

void toggleModule()
{
if (greenButtonState!=greenButtonStatePrev)
{
  if (greenButtonState)
  {
    //serial feedback
    Serial.println("Green is on");

    //turn on light
    digitalWrite(greenButtonLED, HIGH);

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("Interaction", greenButtonName);
    oocsi.addString("Event", "ON");
    oocsi.sendMessage();

    //give user feedback on screen
  
  } else
  {
    //serial feedback
    Serial.println("Green is off");

    //turn on light
    digitalWrite(greenButtonLED, LOW);

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("Interaction", greenButtonName);
    oocsi.addString("Event", "OFF");
    oocsi.sendMessage();

    //give user feedback on screen
 

  }


  
}

if (redButtonState!=redButtonStatePrev)
{
  if (redButtonState)
  {
    //serial feedback
    Serial.println("Red is on");

    //turn on light
    digitalWrite(redButtonLED, HIGH);

    //send OOCSI message

    //give user feedback on screen
  
  } else
  {
    //serial feedback
    Serial.println("Red is off");

    //turn on light
    digitalWrite(redButtonLED, LOW);

    //send OOCSI message

    //give user feedback on screen
 

  }


  
}

if (blueButtonState!=blueButtonStatePrev)
{
  if (blueButtonState)
  {
    //serial feedback
    Serial.println("Blue is on");

    //turn on light
    digitalWrite(blueButtonLED, HIGH);

    //send OOCSI message

    //give user feedback on screen
  
  } else
  {
    //serial feedback
    Serial.println("Blue is off");

    //turn on light
    digitalWrite(blueButtonLED, LOW);

    //send OOCSI message

    //give user feedback on screen
 

  }


  
}
}

void openTrackerModule()
{
if (yellowSwitchState!=yellowSwitchStatePrev)
{
  if (yellowSwitchState)
  {
    //serial feedback
    Serial.println("Yellow Switch is LEFT");

    //send OOCSI message

    //give user feedback on screen
  
  } else
  {
    //serial feedback
    Serial.println("Yellow Switch is RIGHT");

    //send OOCSI message

    //give user feedback on screen
 

  }


  
}

if (yellowButtonState)
  {
    //serial feedback
    Serial.println("Yellow was pressed");

    //send OOCSI message

    //give user feedback on screen

    delay(250);
  
  }

}

void cameraShutter()
{
  if (cameraButtonState)
  {
    //serial feedback
    Serial.println("Photo was taken");

    //send OOCSI message

    //give user feedback on screen

    delay(250);
  
  }
}

void recorderPress()
{
  if(recorderButtonState!=recorderButtonStatePrev)
  {
      if (recorderButtonState)
    {
      //serial feedback
      Serial.println("Recording started");

      //send OOCSI message

      //give user feedback on screen

      delay(250);
    
    } else
    {
      //serial feedback
      Serial.println("Recording ended");

      //send OOCSI message

      //give user feedback on screen

      delay(250);
    }
  }


}

void readGPS()
{
  gps.encode(ss.read());
  Serial.print("Latitude= "); 
  Serial.print(gps.location.lat(), 6);
  Serial.print(" Longitude= "); 
  Serial.println(gps.location.lng(), 6);
  //Serial.println(ss.read());
  if (ss.available() > 0){
    //Serial.println("We have GPS");

    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
    }
  }
  delay(1000);
}


//main loop ---------------------------------------------------------------------------
void loop() {
    
listenForButtons();
delay(5);

toggleModule();
openTrackerModule();
cameraShutter();
recorderPress();

//readGPS(); 

oocsi.check();

}


