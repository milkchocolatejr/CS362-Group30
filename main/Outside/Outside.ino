/*******************************************
 *            OUTSIDE DOOR CODE            *         
 *******************************************
 * Developer 1: AJ Williams                *
 * UIN : 650968054                         *
 * NetID : awill276                        *
 *******************************************
 * Developer 2: Elias Krupa                *
 * UIN : 661040904                         *
 * NetID : ekrup2                          *
 *******************************************
 * Developer 2: AJ Williams                *
 * UIN : 650968054                         *
 * NetID : awill276                        *
 *******************************************/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);

/*PURPOSE: */
void setup() {
  // put your setup code here, to run once:
  mySerial.begin(38400);
  Serial.begin(115200);
  //TODO: Begin LCD and other modules
}

void loop() {
  //Check if data is available
  int numBytes;
  if (numBytes = mySerial.available()) {
    byte readBuf[READ_BUFFER_SIZE];
    //Populate the buffer
    mySerial.readBytes(readBuf, numBytes);

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

  // listening for input form control hub arduino
  if (Serial.available()) {
    int incomingByte = Serial.read();
    // handle incoming byte from main Arduino
    if (debug) {
      Serial.print("Received from Control Hub: ");
      Serial.println(incomingByte);
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
    mySerial.write('A');
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
