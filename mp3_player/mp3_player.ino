/*
   MP3 player
   A simple implementation of an MP3 player using DFPlayerMini_Fast, FireTimer, EEPROM and OneButton libraries
   Written by Ashley Cox, https://ashleycox.co.uk
   Code provided as-is with no warranty or suitability implied. May not be reproduced in full or in part without credit unless written permission is obtained
   Libraries the property of their respective creators
*/

//Include libraries
#include <EEPROM.h> //Stores and recalls user-defined values including thevolume
#include <OneButton.h> //handles button events
#include <DFPlayerMini_Fast.h> //Library to control the DFPlayer module
#include <FireTimer.h> //Simple non-blocking time-based event triggering
#include <SoftwareSerial.h> //For software serial communications with the DFPlayer module

//declare some global variables

//Boolean values, true or false
bool ROM = false; //Is there data in teh EEPROM?
bool changeVolume; //Are we in the midst of changing the volume?
//The DFPlayer's own status reports are unreliable, so we keep track of the status ourselves
bool isPlaying = false; //Is music playing?

//Integers
int currentVolume; //Set based on a value stored in EEPROM, or defaults to 10
int volumeDelay = 1000; //the speed in milliseconds at which the volume ramps up or down
int EEAddress = 0; //Current address of EEPROM data being accessed or written

//Setup library objects
//Software Serial on pins 7 (RX) and 8 (TX)
//leaves pins 9, 10 and 11 free for an RGB LED if desired
SoftwareSerial mySerial(7, 8);
//instantiate the DFPlayer library
DFPlayerMini_Fast mp3;
// Setup a button on d§igital pin D4
OneButton button1(4, true);
// Setup a button on digital pin D5
OneButton button2(5, true);
//Setup a button on digital pin D6
OneButton button3(6, true);
FireTimer volRampDelay;

void setup() {
  //This code runs once

  //First check the EEPROM
  if (!EEPROM.get(EEAddress, ROM)) {
    //No settings have been written to the EEPROM yet
    //Set some default values
    currentVolume = 10; //A sensible starting value with most speakers
  }
  else {
    //There are settings stored in ROM
    ROM = true;
    //Find the first EEPROM address
    EEAddress += sizeof(ROM);
    //Get the last volume state saved in the EEPROM
    currentVolume = EEPROM.get(EEAddress, currentVolume);
  } //End if

  //Setup a serial monitor so we can see what the program is doing
  Serial.begin(9400);
  //setup software serial for DFPlayer communication
  mySerial.begin(9600);
  //initialise the DFPlayer library using the software serial connection, with debug output enabled
  mp3.begin(mySerial, true);
  //Set theDFPlayer volume, or it will startup at full volume by default
  Serial.println("Setting volume to ");
  Serial.print(currentVolume);
  mp3.volume(currentVolume);

  // link the button 1 functions.
  Serial.println("Setting up button 1");
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleClick1);
  button1.attachLongPressStart(longPressStart1);
  button1.attachLongPressStop(longPressStop1);
  button1.attachDuringLongPress(longPress1);

  // link the button 2 functions.
  Serial.println("Setting up button 2");
  button2.attachClick(click2);
  button2.attachDoubleClick(doubleClick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  button2.attachDuringLongPress(longPress2);

  // link the button 3 functions.
  Serial.println("Setting up button 3");
  button3.attachClick(click3);
  button3.attachDoubleClick(doubleClick3);
  button3.attachLongPressStart(longPressStart3);
  button3.attachLongPressStop(longPressStop3);
  button3.attachDuringLongPress(longPress3);

  //set up a timer for the volume ramp
  volRampDelay.begin(volumeDelay);

  //let the DFPlayer catch up
  delay(1000);

} //End setup

void loop() {
  //this code loops repeatedly

  // keep watching the push buttons:
  button1.tick();
  button2.tick();
  button3.tick();

  delay(20); //Blocking delay to stop the program tripping over itself
} //End loop

// This function will be called when button1 ispressed once
void click1() {
  Serial.println("Button 1 click.");
  isPlaying = true;
  mp3.playNext();
  delay(100);
} //end function

// This function will be called when button1 ispressed twice in a short timeframe
void doubleClick1() {
  Serial.println("Button 1 doubleclick.");
  isPlaying = true;
  mp3.playPrevious();
  delay(100);
} //end function

// This function will be called once, when button1 is held for the period set in pressDuration above
void longPressStart1() {
  Serial.println("Button 1 longPress start");
  //We're going to change the volume
  changeVolume = true;
} //end function

// This function will be called often, when button1 is held for the period set in pressDuration above
void longPress1() {
  Serial.println("Button 1 longPress...");
  //If we're changing the volume, and the current volume is more than theminimum possible
  if (changeVolume && currentVolume > 0) {
    //decrement the current volume by 1
    if (volRampDelay.fire()) {
      currentVolume --;
      //set the DFPlayer to thenew volume level
      mp3.volume(currentVolume);
    } //end if
  } //end if
} //end function

// This function will be called once, when button1 is released after a long press
void longPressStop1() {
  Serial.println("Button 1 longPress stop");
  //We've changed the volume, store the new value in the EEPROM
  EEAddress = 0 + sizeof(ROM); // we need to work out the EEPROM address to store the updated value
  EEPROM.put(EEAddress, currentVolume); //Store the updated value in theEEPROM. The value is only updated if it changes
  //We've finished changing the volume
  changeVolume = false;
} //end function

//The other button functions are as above
void click2() {
  Serial.println("Button 2 click.");
  if (!isPlaying) {
    mp3.resume();
  } else {
    isPlaying = false;
    mp3.pause();
  } //end if
} //end function

void doubleClick2() {
  Serial.println("Button 2 doubleclick.");
  mp3.stop();
} //end function

void longPressStart2() {
  Serial.println("Button 2 longPress start");
  //We're about to change the volume
  changeVolume = true;
} //end function

void longPress2() {
  Serial.println("Button 2 longPress...");
  //If we're supposed to be changing the volume, and the volume is less than the maximum possible
  if (changeVolume && currentVolume < 30) {
    if (volRampDelay.fire()) {
      //increment the current volume by 1
      currentVolume ++;
      //Set the DFPlayer to the new volume
      mp3.volume(currentVolume);
    } //end if
  } //end if
} //end function

void longPressStop2() {
  Serial.println("Button 2 longPress stop");
  //store the final volume value in EEPROM
  EEAddress = 0 + sizeof(ROM); // we need to work out the EEPROM address to store the updated value
  EEPROM.put(EEAddress, currentVolume); //Store the updated value in theEEPROM. The value is only updated if it changes
  //We're no-longer changing thevolume
  changeVolume = false;
} //end function

void click3() {
  Serial.println("Button 3 click");
  isPlaying = true;
  mp3.randomAll();
} //end function

void doubleClick3() {
  Serial.println("Button 3 doubleClick");
  isPlaying = false;
  mp3.stop();
} //end function

void longPressStart3() {
  Serial.println("Button 3 longPress start");

} //end function

void longPress3() {
  Serial.println("Button 3 longPress");
} //end function

void longPressStop3() {
  Serial.println("Button 3 longPress stop");
} //end function
