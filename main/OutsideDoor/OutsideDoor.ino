/*******************************************
 *            OUTSIDE DOOR CODE            *         
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

#include<SoftwareSerial.h>

struct OutsidePackage{
  int micValue;
  bool validRFID;
  bool validPin;
}

const int TX = 0;
const int RX = 1;
const int LCD_SDA = 18; //A4
const int LCD_SCL = 19; //A5




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
  if(numBytes = customSerial.available()){
      byte readBuf[numBytes + 1];
      if(handleInput(&buf, numBytes))){
        
        
      }
      else{
        printf("Input handling failure!");
      }
  }
  //TODO: capture mic, number pad, rfid input. Handle said input
  //TODO: Use speaker if needed.
  //TODO: Begin packaging output for control hub
  bool writing = true;

  if(writing){
    //Create struct
    //Convert struct to bytes
    //Write struct byte by byte
  }

}

bool handleInput(byte& buffer, int numBytes){
  //Read, then do something.
  return true;
}
