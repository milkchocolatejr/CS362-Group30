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
#include <Keypad.h>  // keypad library
#include <string.h> // needed for strcmp

struct Message {
    byte to;
    byte from;
    int micValue;
    int validIR;
    bool validPin;
    bool isMoving;
    bool locked;
    bool unlocked;
};

const int SERIAL_BAUD = 115200;
const int MIC_PIN  = A0;  // must use analog input
const int IR_PIN   = 4;   
const int PIR_PIN  = 5;   // motion sensor

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = { // 2d array for keypad
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {6, 7, 8, 9}; // row pins
byte colPins[COLS] = {10, 11, 12, 13}; // column pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
char pinBuffer[5] = "";  // stores the keys that have been entered 
byte pinIndex = 0; // how many digitis have been entered

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
const int buttonPin = 9;  
int lastButtonState = HIGH;
int buttonInput = HIGH;

const size_t SIZE = sizeof(Message);

SoftwareSerial mySerial(2, 3);
bool debug = false; // CHANGE TO TRUE TO DEBUG

void setup() {
  mySerial.begin(SERIAL_BAUD);
  Serial.begin(SERIAL_BAUD);

  pinMode(buttonPin, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  // mic pin is automatically a analog input by default

  //Serial.println("working?");
  //mySerial.println("working?");
}

void loop() {
  Message response;

  if (mySerial.available() >= SIZE && mySerial.peek() == 'O') { // handles the incoming hub messages
    byte readBuf[SIZE];
    mySerial.readBytes(readBuf, SIZE);
    Message requestMessage;
    if (handleInput(readBuf, SIZE, requestMessage)) {
      if (debug) Serial.println("Input handled");
    }
  }

  int reading = digitalRead(buttonPin); // debounce for the keypad and lock button
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  bool send = false;
  if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != buttonInput) {
      buttonInput = reading;
      if (buttonInput == LOW) {
        if (debug) Serial.println("Button pressed");
        send = prepareMessage(response);
      }
    }
  }
  lastButtonState = reading;

  if (send) {
    mySerial.write((byte*)&response, SIZE);
    if (debug) Serial.println("Response sent");
  }
}

bool prepareMessage(Message& response) {
  response.micValue = analogRead(MIC_PIN);
  response.validIR  = digitalRead(IR_PIN);
  response.isMoving = digitalRead(PIR_PIN);
  response.validPin = false;

  char key = keypad.getKey(); // reads keypad digitis

  if (key) {
    if (key != '#') { // exit key
      if (pinIndex < 4) { // add a digit if less than 4 characters so far
        pinBuffer[pinIndex++] = key;
      } 

      if (pinIndex == 4) { // set to null when full
        pinBuffer[pinIndex] = '\0';
      } 
    } 
    
    else {
      if (pinIndex == 4 && strcmp(pinBuffer, "7085") == 0) { // if # is pressed check for our valid pin of 7085
        response.validPin = true;
      }
      pinIndex = 0;
      pinBuffer[0] = '\0';
    }
  }

  // locked commands - WILL SET EVERYTHING TO LOCKED IF NOTHING VALID
  response.locked = true; 
  response.unlocked = false;
  response.to = 'C';
  response.from = 'I';
  return true;
}

bool handleInput(byte* buffer, int numBytes, Message& requestMessage) {
  if (numBytes != (int)SIZE) {
    return false;
  }

  memcpy(&requestMessage, buffer, SIZE);

  if (requestMessage.to != 'O') {
    return false;
  }

  if (requestMessage.locked) {
    Serial.println("LOCKED CMD RECEIVED");
  }
  
  if (requestMessage.unlocked) {
    Serial.println("UNLOCKED CMD RECEIVED");

    return true;
  } 

}
