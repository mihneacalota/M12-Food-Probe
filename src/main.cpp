#include <Arduino.h>

//define screen stuff
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <SPI.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//define OOCSI stuff
#include <OOCSI.h>
#include <HTTPClient.h>

//name of the current device on the OOCSI network
const char *OOCSIName = "M12_Food_Probe_P2";
//the address of the OOCSI server here
const char *hostserver = "oocsi.id.tue.nl";
//name of the general oocsi channel
const char *DF_Channel = "food_probe_data";
//id of teh device in DF
const char *DF_device_id = "d79bedf283999469f";

// SSID of your Wifi network, the library currently does not support WPA2 Enterprise networks
const char* ssid = "Mihnea";
// Password of your Wifi network.
const char* password = "12345678";

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



int yellowButton=36;
const char* yellowButtonName="yellow open button";
int yellowButtonState = 0;

int yellowSwitch=39;
const char* yellowSwitchName="yellow switch";
int yellowSwitchState, yellowSwitchStatePrev;



int cameraButton=4;
const char* cameraButtonName="Shutter";
int cameraLED=12;
int cameraButtonState = 0;



int recorderButton=17;
const char* recorderButtonName="Recorder";
int recorderLED=16;
int recorderButtonState = 0, recorderButtonStatePrev=0;


//sleep variables
unsigned long startMillis, currentMillis;

//SD Card variables
// #include "FS.h"
// #include "SD.h"

// File myFile;
// String dataMessage;
// #define SD_CS 35



//Display animations ------------------------------------------------------------------
void draw(const char *s, const char *ss=" ", int loading=0)
{
  u8g2.setDrawColor(0);		// clear the scrolling area
  u8g2.drawBox(0, 49, u8g2.getDisplayWidth()-1, u8g2.getDisplayHeight()-1);
  u8g2.setDrawColor(1);		// set the color for the text

  int16_t len = strlen(s);
  int16_t offset = (int16_t)u8g2.getDisplayWidth()-len*8;

  int16_t len2 = strlen(ss);
  int16_t offset2 = (int16_t)u8g2.getDisplayWidth()-len2*8;

  int dx=40, dy=20;
  
  if (!loading)
  {
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.drawStr(offset/2, 45, s);
    u8g2.drawStr(offset2/2, 62, ss);
    u8g2.sendBuffer();
  } else {

    u8g2.clearBuffer();					// clear the internal memory
    u8g2.drawDisc (dx, dy, 5);
    u8g2.drawStr(offset/2, 45, s);
    u8g2.drawStr(offset2/2, 62, ss);
    u8g2.sendBuffer();
    delay (1000);

    u8g2.clearBuffer();					// clear the internal memory
    u8g2.drawDisc (dx, dy, 5);
    u8g2.drawDisc (dx+25, dy, 5);
    u8g2.drawStr(offset/2, 45, s);
    u8g2.drawStr(offset2/2, 62, ss);
    u8g2.sendBuffer();
    delay (1000);

    u8g2.clearBuffer();					// clear the internal memory
    u8g2.drawDisc (dx, dy, 5);
    u8g2.drawDisc (dx+25, dy, 5);
    u8g2.drawDisc (dx+50, dy, 5);
    u8g2.drawStr(offset/2, 45, s);
    u8g2.drawStr(offset2/2, 62, ss);
    u8g2.sendBuffer();
    delay (1500);

  }

  
}

void callback(){
  //placeholder callback function
}

//setup function --------------------------------------------------------------------

void setup() {

  //begin u8g2
  u8g2.begin();
  u8g2.setFont(u8g2_font_8x13_mf);
  u8g2.enableUTF8Print();


  //pin modes definitions
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(greenButtonLED, OUTPUT);
  pinMode(redButton, INPUT_PULLUP);
  pinMode(redButtonLED, OUTPUT);
  pinMode(blueButton, INPUT_PULLUP);
  pinMode(blueButtonLED, OUTPUT);

  pinMode(yellowButton, INPUT_PULLDOWN);
  pinMode(yellowSwitch, INPUT);

  pinMode(cameraButton, INPUT_PULLUP);
  pinMode(recorderButton, INPUT_PULLUP);
  pinMode(recorderLED, OUTPUT);
  pinMode(cameraLED,OUTPUT);


  //begin serial
  Serial.begin(115200);
  Serial.println ("Hello");
  draw("Hello there!");
  delay(500);

  //Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(T3, callback, 40);
  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();

 
  

  //WIFI Connection
  
  draw("Connecting", "to WIFI!", 1);

  startMillis=millis();

  if(WiFi.status()!=WL_CONNECTED){
    WiFi.begin(ssid, password);
  }

  while((WiFi.status()!=WL_CONNECTED)&&((millis()-startMillis))<=20000){
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status()!=WL_CONNECTED){
    Serial.println("Go to sleep, no wifi");
    //Go to sleep now
    Serial.println("Going to sleep now");
    draw("Going to sleep!", "3");
    delay(1000);
    draw("Going to sleep!", "2");
    delay(1000);
    draw("Going to sleep!", "1");
    delay(1000);
    draw("");
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }

  oocsi.connect(OOCSIName, hostserver);
  Serial.print("Successfully connected to: ");
  Serial.println(ssid);

  draw("Connected to", ssid);  
  startMillis=millis();
  
  if (analogRead(yellowSwitch)>=3500)
  yellowSwitchState=1;
  else
  yellowSwitchState=0;

  delay (1000);

}

//Custom Functions ---------------------------------------------------------------------

void listenForButtons()
{
  //screen test
  draw("Waiting...");

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
  if (analogRead(yellowButton)>=4000)
  yellowButtonState=1;
  else
  yellowButtonState=0;

  yellowSwitchStatePrev=yellowSwitchState;
  if (analogRead(yellowSwitch)>=4000)
  yellowSwitchState=1;
  else
  yellowSwitchState=0;

  //listen for recorder button
  recorderButtonStatePrev=recorderButtonState;
  if (digitalRead(recorderButton)==HIGH)
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
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", greenButtonName);
    oocsi.addString("Event", "ON");
    oocsi.sendMessage();

    //write to SD
    

    //give user feedback on screen
  
  } else
  {
    //serial feedback
    Serial.println("Green is off");

    //turn on light
    digitalWrite(greenButtonLED, LOW);

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", greenButtonName);
    oocsi.addString("Event", "OFF");
    oocsi.sendMessage();

    //give user feedback on screen
    draw ("Registered event", greenButtonName, 1);
 

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
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", redButtonName);
    oocsi.addString("Event", "ON");
    oocsi.sendMessage();

    //give user feedback on screen
  
  } else
  {
    //serial feedback
    Serial.println("Red is off");

    //turn on light
    digitalWrite(redButtonLED, LOW);

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", redButtonName);
    oocsi.addString("Event", "OFF");
    oocsi.sendMessage();

    //give user feedback on screen
    draw ("Registered event", redButtonName, 1);


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
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", blueButtonName);
    oocsi.addString("Event", "ON");
    oocsi.sendMessage();

    //give user feedback on screen
  
  } else
  {
    //serial feedback
    Serial.println("Blue is off");

    //turn on light
    digitalWrite(blueButtonLED, LOW);

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", blueButtonName);
    oocsi.addString("Event", "OFF");
    oocsi.sendMessage();

    //give user feedback on screen
    draw ("Registered event", blueButtonName, 1);
 

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
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", yellowSwitchName);
    oocsi.addString("Event", "LEFT");
    oocsi.sendMessage();

    //give user feedback on screen
    draw ("Flipped switch", yellowSwitchName, 1);

  } else
  {
    //serial feedback
    Serial.println("Yellow Switch is RIGHT");

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", yellowSwitchName);
    oocsi.addString("Event", "RIGHT");
    oocsi.sendMessage();

    //give user feedback on screen
    draw ("Flipped switch", yellowSwitchName, 1);
 

  }


  
}

if (yellowButtonState)
  {
    //serial feedback
    Serial.println("Yellow was pressed");

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", yellowButtonName);
    oocsi.addString("Event", "Pressed");
    oocsi.sendMessage();

    //give user feedback on screen
    draw ("Button press", yellowButtonName, 1);

    delay(250);
  
  }

}

void cameraShutter()
{
  if (cameraButtonState)
  {
    //serial feedback
    Serial.println("Photo was taken");
    digitalWrite(cameraLED,HIGH);
    delay (200);
    digitalWrite(cameraLED,LOW);
    delay (200);
    digitalWrite(cameraLED,HIGH);
    delay (200);
    digitalWrite(cameraLED,LOW);
    delay (200);
    digitalWrite(cameraLED,HIGH);
    delay (200);
    digitalWrite(cameraLED,LOW);
    delay (200);
    digitalWrite(cameraLED,HIGH);
    delay (2000);
    digitalWrite(cameraLED,LOW);
    

    //send OOCSI message
    oocsi.newMessage(DF_Channel);
    oocsi.addString("device_id", DF_device_id);
    oocsi.addString("Interaction", cameraButtonName);
    oocsi.addString("Event", "Photo taken");
    oocsi.sendMessage();

    //give user feedback on screen
    draw ("Photo captured", "Uploading", 1);

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
      digitalWrite(recorderLED,HIGH);

      //send OOCSI message
      oocsi.newMessage(DF_Channel);
      oocsi.addString("device_id", DF_device_id);
      oocsi.addString("Interaction", recorderButtonName);
      oocsi.addString("Event", "Recording started");
      oocsi.sendMessage();

      //give user feedback on screen
      draw ("Recording start");
      delay(1000);
    
    } else
    {
      //serial feedback
      Serial.println("Recording ended");
      digitalWrite(recorderLED,LOW);

      //send OOCSI message
      oocsi.newMessage(DF_Channel);
      oocsi.addString("device_id", DF_device_id);
      oocsi.addString("Interaction", recorderButtonName);
      oocsi.addString("Event", "Recording ended");
      oocsi.sendMessage();

      //give user feedback on screen
      draw ("Recording end", "Uploading", 1);

      delay(250);
    }
  }


}

void sleepCheck(float minutes)
{ 
  currentMillis=millis();
  if(redButtonState||greenButtonState||redButtonState||yellowButtonState||cameraButtonState||recorderButtonState)
  {
    startMillis=millis();
  } 
  else 
  {
    if ((currentMillis-startMillis)>(minutes*60000))
    {    
      //Go to sleep now
      Serial.println("Going to sleep now");
      draw("Going to sleep!", "3");
      delay(1000);
      draw("Going to sleep!", "2");
      delay(1000);
      draw("Going to sleep!", "1");
      delay(1000);
      draw("");
      esp_deep_sleep_start();
      Serial.println("This will never be printed");
    }
  }
  
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
sleepCheck(1);

}


