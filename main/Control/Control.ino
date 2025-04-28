/*******************************************
 *       CONTROL HUB CODE | Group 30       *
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
    int ultraSonicDistance;
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
const int lockedLED = 11;
const int unlockedLED = 12;
const int noiseLED[3] = {8,9,10};
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
long buzzerStart;


//DEBUG FLAG
bool debug = false;
bool lcdDebug = false;

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
  
  pinMode(lockedLED, OUTPUT);
  pinMode(unlockedLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  locked = true;
  topPos = 0;
  botPos = 0;
  lastTick = millis();

  unlock();

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
  if(millis() - buzzerStart > 500){
    noTone(buzzerPin);
  }
}

void prepareMessage(Message& command, char recipient){
  command.from = 'C';
  command.to = recipient;
  command.locked = locked;
  command.unlocked = !locked;
  command.ultraSonicDistance = -1;
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
      lock();
      Message command1;
      Message command2;
      prepareMessage(command1, 'I');
      prepareMessage(command2, 'O');
      customSerial.write((byte*)&command1, SIZE);
      customSerial.write((byte*)&command2, SIZE);
    }
    else if(requestMessage.unlocked && locked){
      unlock();
      Message command1;
      Message command2;
      prepareMessage(command1, 'I');
      prepareMessage(command2, 'O');
      customSerial.write((byte*)&command1, SIZE);
      customSerial.write((byte*)&command2, SIZE);
    }
  }
  else{
    if(requestMessage.from == 'I'){
      bottomLine = String("DOOR IS ");
      if(requestMessage.isMoving){
        bottomLine += " MOVING";
      }
      else if(!requestMessage.isMoving){
        bottomLine += " STATIONARY";
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
    
  if(debug && lcdDebug){
    Serial.println(topDisplay);
    Serial.println(botDisplay);
  }
  
  lcd.setCursor(0, 1);
  lcd.print(botDisplay);

  topPos++;
  botPos++;
}

void setDistanceLED(int reading){
  int mappedValue = map(reading, 0, 1000, 1, 3);

  for(int i = 0; i < 3; i++){
    if(i < mappedValue){
      digitalWrite(noiseLED[i], HIGH);
    }
    else{
      digitalWrite(noiseLED[i], LOW);
    }
  }
}

void lock(){
  if(locked){
    return;
  }
  locked = true;
  digitalWrite(lockedLED, HIGH);
  digitalWrite(unlockedLED, LOW);
  buzzerStart = millis();
  tone(buzzerPin, 1500);
  if(debug){Serial.println("LOCKED!");}
  topLine = "DOOR LOCKED";
}

void unlock(){
  if(!locked){
    return;
  }
  locked = false;
  buzzerStart = millis();
  tone(buzzerPin, 1500);
  digitalWrite(lockedLED, LOW);
  digitalWrite(unlockedLED, HIGH);
  if(debug){Serial.println("UNLOCKED!");}
  topLine = "DOOR UNLOCKED";
}


