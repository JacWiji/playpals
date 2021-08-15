// compiler error handling
#include "Compiler_Errors.h"

// touch includes
#include <MPR121.h>
#include <MPR121_Datastream.h>
#include <Wire.h>

// MP3 includes
#include <SPI.h>
#include <SdFat.h>
#include <FreeStack.h>
#include <SFEMP3Shield.h>

// touch constants
const uint32_t BAUD_RATE = 115200;
const uint8_t MPR121_ADDR = 0x5C;
const uint8_t MPR121_INT = 4;

// serial monitor behaviour constants
const bool WAIT_FOR_SERIAL = false;

// MPR121 datastream behaviour constants
const bool MPR121_DATASTREAM_ENABLE = false;

// MP3 variables
uint8_t result;
uint8_t lastPlayed = 0;

// MP3 constants
SFEMP3Shield MP3player;

// MP3 behaviour constants
const bool REPLAY_MODE = true;  // by default, touching an electrode repeatedly will
                                // play the track again from the start each time
                                //
                                // if you set this to false, repeatedly touching an
                                // electrode will stop the track if it is already
                                // playing, or play it from the start if it is not

// SD card instantiation
SdFat sd;


void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(LED_BUILTIN, OUTPUT);

  if (WAIT_FOR_SERIAL) {
    while (!Serial);
  }

  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }

  if (!MPR121.begin(MPR121_ADDR)) {
    Serial.println("error setting up MPR121");
    switch (MPR121.getError()) {
      case NO_ERROR:
        Serial.println("no error");
        break;
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
         break;
    }
    while (1);
  }

  MPR121.setInterruptPin(MPR121_INT);

  if (MPR121_DATASTREAM_ENABLE) {
    MPR121.restoreSavedThresholds();
    MPR121_Datastream.begin(&Serial);
  } else {
    //lower thresholds higher sensitivity
    MPR121.setTouchThreshold(38);  //touch must always be > release  setTouchThreshold(11,8) means E11 threshold set to 8
    MPR121.setReleaseThreshold(19); //touch must be twice in value than release!!
    
    MPR121.setTouchThreshold(0,40);
    MPR121.setReleaseThreshold(0,20);
    
    MPR121.setTouchThreshold(4,30);
    MPR121.setReleaseThreshold(4,15);
    
    MPR121.setTouchThreshold(7,30); //longer paints like E0 need lower thresholds
    MPR121.setReleaseThreshold(7,15);
    
    MPR121.setTouchThreshold(9,20);
    MPR121.setReleaseThreshold(9,10);
  }

  MPR121.setFFI(FFI_10);
  MPR121.setSFI(SFI_10);
  MPR121.setGlobalCDT(CDT_4US);  // reasonable for larger capacitances
  
  digitalWrite(LED_BUILTIN, HIGH);  // switch on user LED while auto calibrating electrodes
  delay(1000);
  MPR121.autoSetElectrodes();  // autoset all electrode settings
  digitalWrite(LED_BUILTIN, LOW);

  result = MP3player.begin();
  MP3player.setVolume(1,1); //smaller number increases volume of L,R speaker

  if (result != 0) {
    Serial.print("Error code: ");
    Serial.print(result);
    Serial.println(" when trying to start MP3 player");
  }
}

void loop() {
  MPR121.updateAll();

  // only make an action if we have one or fewer pins touched
  // ignore multiple touches
  if (MPR121.getNumTouches() <= 2) {
    for (int i=0; i < 12; i++) {  // check which electrodes were pressed
      if (MPR121.isNewTouch(i)) {
          if (!MPR121_DATASTREAM_ENABLE) {
            Serial.print("pin ");
            Serial.print(i);
            Serial.println(" was just touched");
          }

          digitalWrite(LED_BUILTIN, HIGH);

          if (i <= 11 && i >= 0) {
            if (MP3player.isPlaying()) {
              if (lastPlayed == i && !REPLAY_MODE) {
                // if we're already playing the requested track, stop it
                // (but only if we're not in REPLAY_MODE)
                MP3player.stopTrack();

                if (!MPR121_DATASTREAM_ENABLE) {
                  Serial.print("stopping track ");
                  Serial.println(i-0);
                }
              } else {
                // if we're already playing a different track (or we're in
                // REPLAY_MODE), stop and play the newly requested one
                MP3player.stopTrack();
                MP3player.playTrack(i-0);

                if (!MPR121_DATASTREAM_ENABLE) {
                  Serial.print("playing track ");
                  Serial.println(i);
                }

                lastPlayed = i;
              }
            } else {
              // if we're playing nothing, play the requested track
              MP3player.playTrack(i-0);

              if (!MPR121_DATASTREAM_ENABLE) {
                Serial.print("playing track ");
                Serial.println(i-0);
              }

              lastPlayed = i;
            }
          }
      } else {
          digitalWrite(LED_BUILTIN, LOW);
      }
    }
  }

  if (MPR121_DATASTREAM_ENABLE) {
    MPR121_Datastream.update();
  }
}
