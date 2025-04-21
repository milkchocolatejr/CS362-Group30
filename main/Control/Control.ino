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
const int SIZE = 20;

//RUNTIME ESTABLISHED
SoftwareSerial customSerial(vTX, vRX);
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool locked;
int topPos;
int botPos;
String topLine = "DOOR UNLOCKED";
String bottomLine = "WELCOME!";
long lastTick;


//DEBUG FLAG
bool debug = false;

//CALCULATION CONSTANTS
const int READ_BUFFER_SIZE = 2048;
const int SERIAL_BAUD = 115200;
const int REFRESH_RATE = 500;


/*PURPOSE: This code is intended to serve as the control hub, receiving input and priniting haptics. */
void setup() {
  customSerial.begin(SERIAL_BAUD);
  Serial.begin(SERIAL_BAUD);
  
  lcd.init();
  lcd.backlight();
  

  locked = false;
  topPos = 0;
  botPos = 0;
  lastTick = millis();

  Serial.println("Program Start!");
}

void loop() {
  //Check if data is available
  int numBytes;
  if (numBytes = customSerial.available()) {
    byte readBuf[SIZE];
    //Populate the buffer
    customSerial.readBytes(readBuf, SIZE);

    Message requestMessage;
    if (handleInput(readBuf, SIZE, requestMessage)) {
      if (debug) { Serial.println("Input handling success!");}
      
    } else {
      if (debug) {
        Serial.println("Input handling failure!");
      }
    }
  }
  if(numBytes = Serial.available()){
    byte readBuf[SIZE];
    Serial.readBytes(readBuf, SIZE);

    Message requestMessage;
    if (handleInput(readBuf, SIZE, requestMessage)) {
      if (debug) { Serial.println("Input handling success!");}
      
    } else {
      if (debug) {
        Serial.println("Input handling failure!");
      }
    }
  }
  if(millis() - lastTick > REFRESH_RATE){
    updateLCD();
    lastTick = millis();
  }
}

bool prepareMessage(Message& command, char recipient){
  command.from = 'C';
  command.to = recipient;
  command.locked = locked;
  command.unlocked = !locked;
  command.micValue = -1;
  command.isMoving = false;
  command.validIR = false;
  command.validPin = false;
}

bool handleInput(byte* buffer, int numBytes, Message& requestMessage) {
  if(debug){
    Serial.println("CALLED!");
  }
  //Read, then do something.
  if (numBytes != SIZE) {
    if (debug) {
      Serial.println("FATAL: TRANSMISSION ARDUINO FAILURE: SIZE");
    }
    //return false;
  }

  memcpy(&requestMessage, buffer, numBytes);

  if(debug){
    Serial.println(requestMessage.from);
    Serial.println(requestMessage.to);
    Serial.println(numBytes);
  }

  if(requestMessage.locked || requestMessage.unlocked){
    if(requestMessage.locked && !locked){
      if(debug){Serial.println("LOCKED!");}
      topLine = "DOOR LOCKED";
      locked = true;
      Message command;
      prepareMessage(command, (requestMessage.from == 'I' ? 'O' : 'I'));
      customSerial.write((byte*)&command, SIZE);
    }
    else if(requestMessage.unlocked && locked){
      if(debug){Serial.println("UNLOCKED!");}
      topLine = "DOOR UNLOCKED";
      locked = false;
    }
  }
  else{
    if(debug){Serial.println("Casual event hit.");}
    if(requestMessage.from == 'O'){
      bottomLine = String("NOISE LEVEL ");
      if(requestMessage.micValue < 500){
        bottomLine += "LOW! ";
      }
      else{
        bottomLine += "HIGH! ";
      }
    }
    if(requestMessage.from == 'I'){
      bottomLine = String("DOOR ");
      if(requestMessage.isMoving){
        bottomLine += "OPEN";
      }
      else{
        bottomLine += "CLOSED";
      }
    }

  }
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
  if(topLine.length() < topPos + 16){
    topEndIndex = topLine.length();
    topShort = true;
  }

  int botEndIndex;
  botEndIndex = botPos + 16;
  if(bottomLine.length() < botPos + 16){
    botEndIndex = bottomLine.length();
    botShort = true;
  }
  
  String topDisplay = (topLine.length() < 16) ? topLine : topLine.substring(topPos, topEndIndex);
  String botDisplay = (bottomLine.length() < 16) ? bottomLine :  bottomLine.substring(botPos, botEndIndex);

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
    
  if(debug){
    Serial.println(topDisplay);
    Serial.println(botDisplay);
  }
  
  lcd.setCursor(0, 1);
  lcd.print(botDisplay);

  topPos++;
  botPos++;
}


