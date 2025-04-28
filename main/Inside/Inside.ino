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
#include <Wire.h>
#include <MPU6050.h>

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

struct AccelData {
  float x, y, z;
};
AccelData currentAccel;

float accelX0, accelY0, accelZ0;
const float motionThreshold = 0.1; 

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
MPU6050 mpu;

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

  Wire.begin();
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
  }

  Serial.println("MPU6050 connected. Calibrating...");

  // Capture baseline (resting position)
  accelX0 = mpu.getAccelerationX() / 16384.0;
  accelY0 = mpu.getAccelerationY() / 16384.0;
  accelZ0 = mpu.getAccelerationZ() / 16384.0;

  delay(1000);
  Serial.println("Calibration complete.");

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

bool checkIfMoving() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Convert to g's
  float accelX = ax / 16384.0;
  float accelY = ay / 16384.0;
  float accelZ = az / 16384.0;

  // Update currentAccel struct (OPTIONAL, if you need it elsewhere)
  currentAccel.x = accelX;
  currentAccel.y = accelY;
  currentAccel.z = accelZ;

  float deltaX = abs(accelX - accelX0);
  float deltaY = abs(accelY - accelY0);
  float deltaZ = abs(accelZ - accelZ0);

  if (deltaX > motionThreshold || deltaY > motionThreshold || deltaZ > motionThreshold) {
    if (debug) {
      Serial.println("Movement detected!");
    }
    return true;
  }
  
  if (debug) {
    Serial.println("No movement.");
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
