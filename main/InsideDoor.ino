/*******************************************
 *            INSIDE DOOR CODE             *         
 *******************************************
 * Developer 1: AJ Williams                *
 * UIN : 650968054                         *
 * NetID : awill276                        *
 *******************************************
 * Developer 2: AJ Williams                *
 * UIN : 650968054                         *
 * NetID : awill276                        *
 *******************************************
 * Developer 2: AJ Williams                *
 * UIN : 650968054                         *
 * NetID : awill276                        *
 *******************************************/

#include <SoftwareSerial.h>

struct OutsidePackage {
  int micValue;
  bool validIR;
  bool validRFID;
  bool validPin;
};

struct InsidePackage {
  bool validIR;
  bool isMoving;

};

struct ControlHubPackage {
  int status;
};

const int TX = 0;
const int RX = 1;
const int LCD_SDA = 18;  //A4
const int LCD_SCL = 19;  //A5

const int READ_BUFFER_SIZE = 2048;

bool debug = false;

SoftwareSerial customSerial(TX, RX);

/*PURPOSE: */
void setup() {
  // put your setup code here, to run once:
  customSerial.begin(9600);
  Serial.begin(9600);
  //TODO: Begin LCD and other modules
}

void loop() {
  //Check if data is available
  int numBytes;
  if (numBytes = customSerial.available()) {
    byte readBuf[READ_BUFFER_SIZE];
    //Populate the buffer
    customSerial.readBytes(readBuf, numBytes);

    ControlHubPackage request;
    if (handleInput(readBuf, numBytes, request)) {
      if (debug) {
        Serial.println("Input handling success!");
        //do things with output
      }
    } else {
      if (debug) {
        Serial.println("Input handling failure!");
      }
    }
  }
  //TODO: capture mic, number pad, rfid input. Handle said input
  //TODO: Use speaker if needed.
  //TODO: Begin packaging output for control hub
  bool writing = true;


  if (writing) {
    //Create struct

    //Convert struct to bytes
    //Write struct byte by byte
  }
}

bool handleInput(byte* buffer, int numBytes, ControlHubPackage& output) {
  //Read, then do something.
  if (numBytes != sizeof(ControlHubPackage)) {
    if (debug) {
      Serial.println("FATAL: TRANSMISSION ARDUINO FAILURE: SIZE");
    }
    return false;
  }

  memcpy(&output, buffer, numBytes);
  return true;
}
