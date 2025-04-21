/*******************************************
 *             CONTOL HUB CODE             *         
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
#include <string>
#include <LiquidCrystal_I2C.h>

struct Message{
    byte to;
    byte from;
    int micValue;
    int validIR;
    int validRFID;
    bool validPin;
    bool isMoving;
    bool locked;
    bool unlocked;
};

//GLOBAL PINS
const int TX = 0;
const int RX = 1;
const int vTX = 2;
const int vRX = 3;
const int LCD_SDA = 18;  //A4
const int LCD_SCL = 19;  //A5
const int buzzerPin = 5;
const int lockedLED = 6;
const int unlockedLED = 7;

//RUNTIME ESTABLISHED
SoftwareSerial customSerial(vTX, vRX);
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool locked;
int topPos;
int botPos;
String topLine;
String bottomLine;


//DEBUG FLAG
bool debug = true;

//CALCULATION CONSTANTS
const int READ_BUFFER_SIZE = 2048;
const int SERIAL_BAUD = 115200;


/*PURPOSE: This code is intended to serve as the control hub, receiving input and priniting haptics. */
void setup() {
  customSerial.begin(SERIAL_BAUD);
  Serial.begin(SERIAL_BAUD);
  
  lcd.init();
  lcd.backlight();
  

  locked = false;
  topPos = 0;
  botPos = 0;

  Serial.println("Program Start!");
}

void loop() {
  //Check if data is available
  int numBytes;
  if (numBytes = customSerial.available() && customSerial.peek() == 'C') {
    byte readBuf[READ_BUFFER_SIZE];
    //Populate the buffer
    customSerial.readBytes(readBuf, numBytes);

    Message requestMessage;
    if (handleInput(readBuf, numBytes, requestMessage)) {
      if (debug) { Serial.println("Input handling success!");}
      
    } else {
      if (debug) {
        Serial.println("Input handling failure!");
      }
    }
  }
  if(numBytes = Serial.available() && Serial.peek() == 'C'){
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

bool handleInput(byte* buffer, int numBytes, Message& requestMessage) {
  if(debug){Serial.println("CALLED!");}

  //Read, then do something.
  if (numBytes != sizeof(Message) + 1) {
    if (debug) {
      Serial.println("FATAL: TRANSMISSION ARDUINO FAILURE: SIZE");
    }
    return false;
  }

  memcpy(&requestMessage, buffer, numBytes);

  if(requestMessage.locked || requestMessage.unlocked){
    if(requestMessage.locked){
      if(debug){Serial.println("LOCKED!");}
      topLine = "DOOR LOCKED";
      locked = true;
    }
    else if(requestMessage.unlocked){
      if(debug){Serial.println("UNLOCKED!");}
      topLine = "DOOR UNLOCKED";
      locked = false;
    }

    if(requestMessage.validIR){
        if(debug){Serial.println("BY IR SENSOR");}
          bottomLine = "BY IR SENSOR";
        }
      else if(requestMessage.validPin){
        if(debug){Serial.println("BY KEYPAD");}
        bottomLine = "BY KEYPAD";
      }
  }
  else{
    if(debug){Serial.println("Casual event hit.");}
    if(requestMessage.from == 'O'){
      bottomLine = "NOISE LEVEL: " + requestMessage.micValue;
    }
    if(requestMessage.from == 'I'){
      bottomLine = String("DOOR ") + (requestMessage.isMoving ? "OPEN" : "CLOSED");
    }
  }

  updateLCD();

  return true;
}

void updateLCD(){
  bool topShort = false;
  bool botShort = false;

  if(topPos >= topLine.length()){
    topPos = 0;
  }
  if(botPos >= bottomLine.length()){
    botPos = 0;
  }

  int topEndIndex;
  topEndIndex = topPos + 16;
  if(topLine.length() <= topPos + 16){
    topEndIndex = topLine.length();
    topShort = true;
  }

  int botEndIndex;
  botEndIndex = botPos + 16;
  if(bottomLine.length() <= botPos + 16){
    botEndIndex = bottomLine.length();
    botShort = true;
  }
  
  String topDisplay = (topLine.length() < 16) ? topLine : topLine.substring(topPos, topEndIndex);
  String botDisplay = (bottomLine.length() < 16) ? bottomLine :  bottomLine.substring(botPos, botEndIndex);

  Serial.println(topDisplay);
  Serial.println(botDisplay);

  if(topShort && topDisplay != topLine){
    int left = 16 - topDisplay.length();
    topDisplay += topLine.substring(0, left);
  }
  if(botShort && botDisplay != bottomLine){
    int left = 16 - botDisplay.length();
    botDisplay += bottomLine.substring(0, left);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(topDisplay);
  lcd.setCursor(0, 1);
  lcd.print(botDisplay);

  topPos++;
  botPos++;
}


