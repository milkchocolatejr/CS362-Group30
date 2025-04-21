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

struct Message{
    char from;
    int micValue;
    int validIR;
    bool validPin;
    bool isMoving;
    bool locked;
    bool unlocked;
};


#include <SoftwareSerial.h>

const int SERIAL_BAUD = 115200;
SoftwareSerial mySerial(2, 3);

/*PURPOSE: */
void setup() {
  // put your setup code here, to run once:
  mySerial.begin(SERIAL_BAUD);
  Serial.begin(SERIAL_BAUD);
  //TODO: Begin LCD and other modules
}

void loop() {
  //Check if data is available
  int numBytes;
  if (numBytes = mySerial.available() && mySerial.peek() == 'O') {
    byte readBuf[READ_BUFFER_SIZE];
    //Populate the buffer
    mySerial.readBytes(readBuf, numBytes);

    Message requestMessage;
    if (handleInput(readBuf, numBytes, requestMessage)) {
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

  if(numBytes = Serial.available() && Serial.peek() == 'O'){
    byte readBuf[READ_BUFFER_SIZE];
    Serial.readBytes(readBuf, numBytes);

    Message requestMessage;
    if (handleInput(readBuf, numBytes, requestMessage)) {
      if (debug) { Serial.println("Input handling success!");}
      
    } else {
      if (debug) {
        Serial.println("Input handling failure!");
      }
    }
  }
  }

  //TODO: capture mic, number pad, rfid input. Handle said input
  //TODO: Use speaker if needed.
  //TODO: Begin packaging output for control hub
  bool writing = true;

  if (writing) {
    Message response;
    //Create struct

    //Convert struct to bytes

    //Write struct byte by byte
    mySerial.write('C');
  }
}

bool handleInput(byte* buffer, int numBytes, Message& requestMessage) {
  //Read, then do something.
  if (numBytes != sizeof(Message) + 1) {
    if (debug) {
      Serial.println("FATAL: TRANSMISSION ARDUINO FAILURE: SIZE");
    }
    return false;
  }

  memcpy(&requestMessage, buffer, numBytes);
  Serial.println("WORKED!");
  return true;
}
