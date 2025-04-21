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
    byte to;
    byte from;
    int micValue;
    int validIR;
    bool validPin;
    bool isMoving;
    bool locked;
    bool unlocked;
};


#include <SoftwareSerial.h>

const int SERIAL_BAUD = 115200;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int buttonPin = 9;
unsigned long interval = millis();
int delayTime = 500;
bool debug = true;
int READ_BUFFER_SIZE = sizeof(Message);
int lastButtonState = HIGH;
int buttonInput;


SoftwareSerial mySerial(2, 3);

long tick;

/*PURPOSE: */
void setup() {
  // put your setup code here, to run once:
  mySerial.begin(SERIAL_BAUD);
  Serial.begin(SERIAL_BAUD);
  tick = millis();
  pinMode(buttonPin, INPUT);
  Serial.println("working?");
  mySerial.println("working?");

  //TODO: Begin LCD and other modules
}

void loop() {
  //Check if data is available
  //Serial.println("testing if its working");
  int numBytes;
  if (numBytes = mySerial.available() && mySerial.peek() == 'O') {
    byte readBuf[READ_BUFFER_SIZE];
    //Populate the buffer
    mySerial.readBytes(readBuf, sizeof(Message));

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
    Serial.readBytes(readBuf, sizeof(Message));

    Message requestMessage;
    if (handleInput(readBuf, numBytes, requestMessage)) {
      if (debug) { Serial.println("Input handling success!");}
      
    } else {
      if (debug) {
        Serial.println("Input handling failure!");
      }
    }
  }
  
  int reading = digitalRead(buttonPin);
  // If the state has changed
  if (reading != lastButtonState) {
    // reset the debouncing timer to make the next if statement wait 50ms
    lastDebounceTime = millis();
  }

  Message response;
  bool send = false;
  //int reading = digitalRead(buttonPin);

  // will hit the condition every delay of 100ms if lastDebounceTime was updated. If not, will hit the condition real-time
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed, update the current input
    if (reading != buttonInput) {
      // Set button input after the delay from a bounce
      buttonInput = reading;

      // If current input is HIGH, increase counter and update LEDs
      if (buttonInput == LOW) {
        // When Button is HIGH or Pressed
        if(debug) {
          Serial.println("finished Serial");
          //mySerial.println("finished mySerial");
        }
        send = prepareMessage(response);
    
      }
    }
  }
  lastButtonState = reading;

  if(send){
    mySerial.write((byte*)&response, sizeof(Message));
    Serial.println("RESPONSE WRITTEN!");
  }
  
}

bool prepareMessage(Message& response){
  if(response.locked){
    response.unlocked = true;
    //response.locked = false;
  }
  else{
    response.locked = true;
    //response.locked = true;
  }
  response.to = 'C';
  response.from = 'O';
  response.micValue = -1;
  response.isMoving = false;
  response.validIR = false;
  response.validPin = false;

  return true;
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
