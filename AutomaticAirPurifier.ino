#include "arduino_secrets.h"
// All the wifi and device credentials


/*
  Automatic Air Purifier
    By V205
  Credits:
  Arduino(For making all this possible, documentaion, IDE, and IoT cloud), Dfrobot(For documentation, software and hardware), 
  Adafruit(for documentation), GroundFungus(for help with IR stuff), Xfpd(for help with compiler errors), DaveX(for help with IR reciever pinout), JohnLincoln(for help with IR reciever pinout), LarryD(for help with IR reciever pinout)
  Espressif(For ESP-32 arduino core), Ptillich(For debugging uploading problem), Pennam(fot help with complier errors),
  Everyone who made IRremote and IoT cloud, Everyone who made the ESP-32 core, Everyone who made the SGP40 library, Eveeryone who made c++, and Ascii.
  Thank you to everyone listed above.
  Please tell me if I forgot to give atribution to you.
  I will add you in to the list.
  
  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  int fanSpeedLevel;
  int VOCIndex;
  CloudSchedule schedule;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

#include "thingProperties.h"  // includes arduino cloud settings
#include <DFRobot_SGP40.h>    //libary for SGP40 Voc sensor
#include "TinyIRSender.hpp"   // includes TinySender sublibrary of IRremote

DFRobot_SGP40 mySgp40;  // initialize SGP 40
#define IR_SEND_PIN D3  // The pin for sending the IR signals.
#define LDRinputPin A5 // LDR pin.


uint8_t sRepeats = 0;//Repeats just zero.

struct IrRemoteKeys  // declare the struct
{
  byte command;
  unsigned long address;
  char name[12];//Name can only be up to 12 characters long
  char comment[16];//Comment can only be up to 16 characters long
};

IrRemoteKeys IRcodes[] =  // decare an array of the struct
  {                       //Command, Address, Name, Comment.
    { 0x13, 0x12, "power", "power toggle" },
    { 0x11, 0x12, "fanUp", "Make fan faster" },
    { 0x10, 0x12, "fanDown", "Make fan slower" },
    { 0x12, 0x12, "Turbo", "Make fan turbo" },
    { 0x16, 0x12, "Auto", "toggle auto" }
  };






const int speakerPin = 9;
int pitch = 805;
int increment = 5;
const int updateInterval = 10;  // interval between updates
unsigned long lastUpdate;       // last update of position

const int alertDuration = 15000;  //lenght of alarm work in progress

const int fanLDRbrightnessTrigger = 100;




int VOCAlert = 400;  // change to the VOC level tht you want to trigger the alarm with
int fanLDRvalue;     // for sensing fan's LDR
/*
int speakerFreq = 0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time buzzer was updated
unsigned long alertHold = 0;
*/

bool freqGoingUp = true;
bool VOC_alertToggleSwitch = true;  // replace with false if you don't want an alarm when voc level exceeds vocalert
//bool autoMode = false;


void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(10000);
  Serial.println(F("Running" __FILE__ " by viggo v205"));
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing tiny IR library version " VERSION_TINYIR));

  Serial.println(F(__DATE__));
  Serial.println(F(__TIME__));
  //delay(9000);
  // while (mySgp40.begin(/*duration = */ 10000) != true) {
  //   Serial.println(F("failed to connect to SGP 40 , please check the wiring"));
  //   delay(1000);
  // }



  //Serial.println("sgp40 initialized successfully!");

  // Defined in thingProperties.h
  initProperties();// init IoT Cloud Properties 

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);  //Begin Connection to Arduino Cloud.


  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
  */
  setDebugMessageLevel(4);
  ArduinoCloud.printDebugInfo();  // Send debug info to Arduino cloud.
}

void loop() {
  ArduinoCloud.update();                   // Update arduino cloud variables.
  unsigned long millisCounter = millis();  // counter
  //Serial.println(F("looping..."));
  VOCIndex = mySgp40.getVoclndex();  // set VOC index to SGP 40 sensor value.

  if (schedule.isActive()) {
    // turn air purifier on
  }
  if (VOCIndex >= VOCAlert && VOC_alertToggleSwitch == true || pitch != 805) {  // checks if VOC level is higher then VOC Alert level
    UpdateSpeakerPitch();// sound alert.

  } else {
    noTone(speakerPin);
  }
}





/*
  Since FanSpeedLevel is READ_WRITE variable, onFanSpeedLevelChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onFanSpeedLevelChange() {
  // Add your code here to act upon FanSpeedLevel change
  Serial.println("fanSpeedLevel = ");
  Serial.println(fanSpeedLevel);
  if (fanSpeedLevel <= 4) {
    if (fanSpeedLevel == 0) {
      // Turn of air purifier


      //send turbo code
      sendNEC(IR_SEND_PIN, IRcodes[4].address, IRcodes[4].command, sRepeats);
      delay(1000);
      fanLDRvalue = analogRead(LDRinputPin);
      if (fanLDRvalue >= fanLDRbrightnessTrigger) {
        sendNEC(IR_SEND_PIN, IRcodes[1].address, IRcodes[1].command, sRepeats);  // send IR code for power on/ off
        Serial.println(F("Sending power Code"));

        delay(1000);
      }




    } else {
      //send turbo
      sendNEC(IR_SEND_PIN, IRcodes[4].address, IRcodes[4].command, sRepeats);
      Serial.println(F("Sending turbo Code line 187"));
      delay(1000);
      //int fanSigNumber = 4 - fanSpeedLevel;
      fanLDRvalue = analogRead(LDRinputPin);
      if (fanLDRvalue >= fanLDRbrightnessTrigger) {
        sendNEC(IR_SEND_PIN, IRcodes[1].address, IRcodes[1].command, sRepeats);  // send IR code for power on/ off
        Serial.println(F("Sending power Code"));

        delay(1000);
      }

      for (int fanSigNumber; fanSigNumber <= 4 - fanSpeedLevel; fanSigNumber++) {
        sendNEC(IR_SEND_PIN, IRcodes[3].address, IRcodes[3].command, sRepeats);  // send IR code for lower fan speed
        Serial.println(F("Sending lower fan speed Code"));
        Serial.println(fanSigNumber);
        delay(1000);
      }
    }

  } else if (fanSpeedLevel == 5) {
    // do built in auto mode

    //send turbo
    sendNEC(IR_SEND_PIN, IRcodes[4].address, IRcodes[4].command, sRepeats);
    Serial.println(F("Sending turbo Code"));
    delay(1000);
    fanLDRvalue = analogRead(LDRinputPin);
    if (fanLDRvalue >= fanLDRbrightnessTrigger) {
      sendNEC(IR_SEND_PIN, IRcodes[1].address, IRcodes[1].command, sRepeats);  // send IR code for power on/ off
      Serial.println(F("Sending power Code"));

      delay(1000);
    }

    sendNEC(IR_SEND_PIN, IRcodes[5].address, IRcodes[5].command, sRepeats);  // send IR code for Built in auto mode
    Serial.println(F("Sending Auto Code"));
    delay(1000);
  }
}




void UpdateSpeakerPitch() {

  if ((millis() - lastUpdate) > updateInterval)  // time to update
  {
    lastUpdate = millis();
    pitch += increment;
    tone(speakerPin, pitch);

    //Serial.println(pitch);
    if ((pitch >= 900) || (pitch <= 800))  // tone boundaries.
    {
      // reverse pitch direction
      increment = -increment;
    }
  }
}

int playing = 0;
void tone(byte pin, int freq) {
  ledcSetup(0, 2000, 8);   // setup beeper
  ledcAttachPin(pin, 0);   // attach beeper
  ledcWriteTone(0, freq);  // play tone
  playing = pin;           // store pin
}
void noTone() {
  tone(playing, 0);// Stop tone
}

/*
  Everything under here is the unused onVARIABLE_NAMEChange() functions
  I have not used these functions.
  It is possible that I may use them in the future.


  Since Schedule is READ_WRITE variable, onScheduleChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onScheduleChange() {
  //No code is needed here
  // Add your code here to act upon Schedule change
}
