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
      if(!handleInput(customSerial.read(), numBytes)){
        printf("Input handling failure!");
      }
  }
  //TODO: capture mic, number pad, rfid input. Handle said input
  //TODO: Use speaker if needed.
  //TODO: Begin packaging output for control hub
  bool writing = true;

  if(writing){
    
  }

}

bool handleInput(int request, int numBytes){
  switch(request){
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    default:
      return false;
  }
  return true;
}
