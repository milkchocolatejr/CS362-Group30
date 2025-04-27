/*******************************************
 * INSIDE DOOR CODE            *
 *******************************************
 * Developer 1: AJ Williams            *
 * UIN : 650968054                     *
 * NetID : awill276                      *
 *******************************************
 * Developer 2: Elias Krupa            *
 * UIN : 661040904                     *
 * NetID : ekrup2                        *
 *******************************************
 * Developer 2: Michael Cali           *
 * UIN : 664777671                     *
 * NetID : mcali3                        *
 *******************************************/


#include <SoftwareSerial.h>
#include <IRremote.h>

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

struct AccelStats{
  float x,y,z = 0.0;
};


const int SERIAL_BAUD = 115200;
const int trigPin = 11;
const int echoPin = 10;
const int SIZE = 20; // WARNING: This might not match sizeof(Message). Consider using sizeof(Message).
const int IR_PIN = 12; 


int buttonPin = 9;
// IR Setup
IRrecv irrecv(IR_PIN); // IR receiver setup
decode_results results;
bool unlocked = false; // unlocked state flag
unsigned long irWindowStart = 0;
int irPressCount = 0;
const unsigned long irWindowDuration = 15000; // ms
// End of IR Setup
unsigned long interval; // Should be initialized in setup
int delayTime = 500;
bool debug = true;
// Use sizeof(Message) which is now defined locally
int READ_BUFFER_SIZE = sizeof(Message);
AccelStats baseAccel, currentAccel; // These types are now defined locally


SoftwareSerial mySerial(2, 3);

long tick;

/*PURPOSE: */
void setup() {
  // put your setup code here, to run once:
  mySerial.begin(SERIAL_BAUD);
  Serial.begin(SERIAL_BAUD);
  tick = millis();
  interval = millis(); // Initialize interval here
  pinMode(IR_PIN, INPUT);
  pinMode(trigPin, OUTPUT); // Added pinMode for trigPin
  pinMode(echoPin, INPUT);  // Added pinMode for echoPin
  digitalWrite(trigPin, LOW); // Ensure low initially

  Serial.println("working?");
  mySerial.println("working?");

  baseAccel.x = 0.0; // TODO: Set baseline valus to first read values on setup or a pre-determined set after we have found what that is
  baseAccel.y = 0.0;
  baseAccel.z = 0.0;

  currentAccel.x = 0.0; // TODO: Set values to current live read in values
  currentAccel.y = 0.0;
  currentAccel.z = 0.0;

  irrecv.enableIRIn(); // start IR receiver
}

void loop() {

  //Check if data is available
  int numBytes; // This variable is assigned but its value isn't used effectively in the original if condition

  if (mySerial.available() && mySerial.peek() == 'O') { // Simplified condition check
    byte readBuf[SIZE]; // Using hardcoded SIZE = 20
    //Populate the buffer - Need to check if enough bytes are available first
    if (mySerial.available() >= SIZE) { // Check if enough bytes are actually available
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
  }


  if(Serial.available() && Serial.peek() == 'O'){ // Simplified condition check
    byte readBuf[READ_BUFFER_SIZE]; // Using READ_BUFFER_SIZE which is sizeof(Message)
     if (Serial.available() >= READ_BUFFER_SIZE) { // Check if enough bytes are actually available
        int bytesRead = Serial.readBytes(readBuf, READ_BUFFER_SIZE); // Use the correct size

        Message requestMessage;
        if (handleInput(readBuf, bytesRead, requestMessage)) {
          if (debug) { Serial.println("Input handling success!");}

        } else {
          if (debug) {
            Serial.println("Input handling failure!");
          }
        }
     } 
  }

  Message response;
  bool send = false;

  // START OF IR CODE
  /////////////// 
 if (irrecv.decode(&results)) { // handles IR presses for unlock
        irrecv.resume();
            if (irWindowStart == 0 || millis() - irWindowStart > irWindowDuration) {
                irWindowStart = millis();
                irPressCount = 1;
            } 
            else {
                irPressCount++;
            }

            if (debug) {
                Serial.print("IR press #");
                Serial.println(irPressCount);
            }
            if (irPressCount >= 7) { // populate struct for IR unlock
                send = prepareMessage(response);
                response.validIR = true;
                response.locked = false;
                response.unlocked = true;

                Serial.println("Door Unlocked! IR sensor");

                // send unlocked message
                mySerial.write((byte*)&response, SIZE);
                if (debug) {
                    Serial.println("IR unlock sent");
                }
            }
        
    }




  /////////////// 
  // END OF IR CODE




 

  // Change delayTime for how long between reading ultrasonic/accel and sends the message
    if ((millis() - interval) > delayTime) {
        interval = millis(); // Reset interval timer
        float dist = readDistance();
        Serial.print("Distance: ");
        Serial.print(dist); 
        Serial.println(" cm");

        if(debug)
        {
          Serial.println("Accel Check Tick");
        }
        // Prepare default values
        send = prepareMessage(response);

        // Send either both ultrasonic and moving data if true or just ultrosonic. Default isMoving is fault from prepareMessage()
        if(send && checkIfMoving()){
            response.isMoving = true;
            response.ultraSonicDistance = dist;
        }
        else if(send){
            response.ultraSonicDistance = dist;
        }
          // Send the message and confirm in serial
          mySerial.write((byte*)&response, SIZE);
          Serial.println("IS Accel and Ultrasonic message SENT!");
    } 
}



bool prepareMessage(Message& response){
  response.locked = true; // Default value, should reflect actual state if possible
  response.unlocked = false; // Default value
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
  // Serial.println("GOTBACK"); // Original debug message
  // Use READ_BUFFER_SIZE (sizeof(Message)) for comparison
  if (numBytes != READ_BUFFER_SIZE) {
    if (debug) {
      Serial.print("FATAL: TRANSMISSION ARDUINO FAILURE: SIZE MISMATCH. Expected: ");
      Serial.print(READ_BUFFER_SIZE);
      Serial.print(" Got: ");
      Serial.println(numBytes);
    }
    return false;
  }

  memcpy(&requestMessage, buffer, numBytes);

  // Original code checked against 'O', assuming that was the intended target address.
  // If 'I' (Inside) is the target, change this check.
  if (requestMessage.to != 'O') { // Assuming 'O' is the HUB/Outside based on original check
    if (debug) {
        Serial.print("Message target '");
        Serial.print((char)requestMessage.to);
        Serial.println("' does not match expected 'O'.");
    }
    // return false; // Keep original behavior: return false if target doesn't match
  }

  if (requestMessage.locked) {
    Serial.println("LOCKED CMD RECEIVED");
    // Add code to handle locking
  }

  if (requestMessage.unlocked) {
    Serial.println("UNLOCKED CMD RECEIVED");
    // Add code to handle unlocking
  }

  return true; // *** CORRECTED: Return true after processing flags ***
}

bool checkIfMoving(){
  //Check if any of the xyz values are more than a set threshold
  // TODO: Implement actual accelerometer reading and update currentAccel.x, y, z
  // TODO: Find the threshhold, currently its just 5 as a placeholder
  float threshold = 5.0; // Example threshold
  // Use abs() for checking difference in either direction
  if(abs(currentAccel.x - baseAccel.x) > threshold || abs(currentAccel.y - baseAccel.y) > threshold || abs(currentAccel.z - baseAccel.z) > threshold){
    return true;
  }
  return false;
}

float readDistance() {
  unsigned long intervalStartTime; // Changed variable name for clarity

  digitalWrite(trigPin, LOW);
  intervalStartTime = micros();
  while (micros() - intervalStartTime < 2) {
  } // Empty loop for waiting

  digitalWrite(trigPin, HIGH);
  intervalStartTime = micros();
  while (micros() - intervalStartTime < 10) {
  } // Empty loop for waiting

  digitalWrite(trigPin, LOW);

  // Using pulseIn without timeout (default 1 second)
  long duration = pulseIn(echoPin, HIGH);

  // Calculate distance, handle potential 0 duration from timeout
  int distance = 0;
  if (duration > 0) {
    distance = duration / 58;
  }

  return distance;
}
