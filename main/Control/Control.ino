/*******************************************
 *             CONTOL HUB CODE             *         
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

SoftwareSerial customSerial(TX, RX);

/*PURPOSE: */
void setup() {
  // put your setup code here, to run once:
  customSerial.begin(38400);
  Serial.begin(115200);
  
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
