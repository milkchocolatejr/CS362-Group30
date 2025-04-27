/*******************************************
 *            INSIDE DOOR CODE             *         
 *******************************************
 * Developer 1: AJ Williams                *
 * UIN : 650968054                         *
 * NetID : awill276                        *
 *******************************************
 * Developer 2: Elias Krupa                *
 * UIN : 661040904                         *
 * NetID : ekrup2                          *
 *******************************************
 * Developer 2: Michael Cali               *
 * UIN : 664777671                         *
 * NetID : mcali3                          *
 *******************************************/

struct Message{
    byte to;
    byte from;
    flaot ultraSonicDistance;
    int validIR;
    bool validPin;
    bool isMoving;
    bool locked;
    bool unlocked;
};

struct AccelStats{
  float x,y,z = 0.0;
};


#include <SoftwareSerial.h>

const int SERIAL_BAUD = 115200;
unsigned long interval = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10000;    // the debounce time; increase if the output flickers
int buttonPin = 9;
// IR Setup
const int irReceiverPin =7; //the SIG of receiver module attach to pin7 
IRrecv irrecv(irReceiverPin); //Creates a variable of type IRrecv
decode_results results;
// End of IR Setup
unsigned long interval = millis();
int delayTime = 500;
bool debug = true;
int READ_BUFFER_SIZE = sizeof(Message);
int lastButtonState = HIGH;
int buttonInput;
const int SIZE = 20;
AccelStats baseAccel, currentAccel;



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

  baseAccel.x = 0.0; // TODO: Set baseline valus to first read values on setup or a pre-determined set after we have found what that is
  baseAccel.y = 0.0;
  baseAccel.z = 0.0;

  currentAccel.x = 0.0; // TODO: Set values to current live read in values
  currentAccel.y = 0.0;
  currentAccel.z = 0.0;

    irrecv.enableIRIn(); //enable ir receiver module 



  //TODO: Begin LCD and other modules
}

void loop() {
  //Check if data is available
  //Serial.println("testing if its working");
  int numBytes;
  if (numBytes = mySerial.available() && mySerial.peek() == 'O') {
    byte readBuf[SIZE];
    //Populate the buffer
    mySerial.readBytes(readBuf, SIZE);

    Message requestMessage;
    if (handleInput(readBuf, SIZE, requestMessage)) {
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
 

  Message response;
  bool send = false;
  //int reading = digitalRead(buttonPin);

  // Will check every debounce delay (currentl 10s) and if message is successfully prepared, and we confirm the door is moving, send doo
  // TODO: Figure out if we want to send this alert every tick or only when receiving a message from the HUB. This current setup is for every tick.
  if ((millis() - interval) > debounceDelay) {
    interval = millis();
  // Start of IR code for every tick
  if (irrecv.decode(&results)) //if the ir receiver module receiver data
  { 
    Serial.print("irCode: "); //print"irCode: " 
    Serial.print(results.value, HEX); //print the value in hexdecimal 
    Serial.print(", bits: "); //print" , bits: " 
    Serial.println(results.bits); //print the bits
    irrecv.resume(); // Receive the next value 
  } 
    // If we received a IR signal from the remote
    if(results.value) // NOTE: change to "results.value == results.value" if we want to only check for power button press. Currently this accepts any IR value from the remote.
    {
      Serial.println("IR Signal Received!"); //print the bits
      send = prepareMessage(response);
      if(send){
        response.validIR = true;
        mySerial.write((byte*)&response, SIZE);
        Serial.println("VALID IR SENT!");
      }
    }
  //End of IR setup for every tick

    // Beginning of accel tick code
        if(debug) 
        {
          Serial.println("Accel Check Tick");
        }
        send = prepareMessage(response);

        // If prepare message was good and we confirm moving
        if(send && checkIfMoving()){
          response.isMoving = true;
          mySerial.write((byte*)&response, SIZE);
          Serial.println("IS MOVING SENT!");
        }
        /*
      else if (send && !checkIfMoving()){ // Probably dont need this else if as we really only care about writing when we ARE moving.
        response.isMoving = false;
        mySerial.write((byte*)&response, SIZE);
       Serial.println("IS NOT MOVING WRITTEN!");
      }*/ 

    // End of accel tick dode
    
  }
} // End of loop



bool prepareMessage(Message& response){
  response.locked = true;
  response.unlocked = false;
  response.to = 'C';
  response.from = 'I';
  response.ultraSonicDistance = -1;
  response.isMoving = false;
  response.validIR = false;
  response.validPin = false;

  return true;
}

bool handleInput(byte* buffer, int numBytes, Message& requestMessage) {
  //Read, then do something.
  Serial.println("GOTBACK");
  if (numBytes != SIZE) {
    if (debug) {
      Serial.println("FATAL: TRANSMISSION ARDUINO FAILURE: SIZE");
    }
    return false;
  }

  memcpy(&requestMessage, buffer, numBytes);

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

bool checkIfMoving(){
  //Check if any of the xyz values are more than a set threshold
  // TODO: Find the threshhold, currently its just 5 as a placeholder
  if(currentAccel.x - baseAccel.x > 5 || currentAccel.y - baseAccel.y > 5 ||currentAccel.z - baseAccel.z > 5){
    return true;
  }
  return false;
}
