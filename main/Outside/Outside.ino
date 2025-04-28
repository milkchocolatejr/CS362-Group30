/*******************************************
 *       INSDIE DOOR CODE | Group 30       *
 *******************************************
 * Developer 1: AJ Williams                *
 * UIN : 650968054                         *
 * NetID : awill276                        *
 *******************************************
 * Developer 2: Elias Krupa                *
 * UIN : 661040904                         *
 * NetID : ekrup2                          *
 *******************************************
 * Developer 3: Michael Cali               *
 * UIN : 664777671                         *
 * NetID : mcali3                          *
 *******************************************/

#include <SoftwareSerial.h>
#include <string.h>    
#include <IRremote.h>

struct Message {
  byte to;
  byte from;
  int ultraSonicDistance;
  int validIR;
  bool validPin;
  bool isMoving;
  bool locked;
  bool unlocked;
};

const size_t SIZE = 20; // matches control hub size which is 20 bytes
const int SERIAL_BAUD = 115200;
const int MIC_PIN = A0; // analog mic input - not used but needed for struct structure
const int IR_PIN = 12; 
const int PIR_PIN = 5; // PIR motion sensor - not used but needed for struct structure

const int buttonPins[4] = {5, 4, 3, 2}; // pins for buttons
const int ledPins[4] = {11,10, 9, 8}; // leds corresponding to button pins

const unsigned long debounceDelay = 50; // ms
int buttonState[4] = {LOW, LOW, LOW, LOW};
int lastReading [4] = {LOW, LOW, LOW, LOW};
unsigned long lastDebounceTime[4] = {0, 0, 0, 0};

const int correctCode[4] = {1, 2, 3, 4}; // correct passcode corresponding to buttons
int codeSequence[4];
int codeIndex = 0;

unsigned long irWindowStart = 0;
int irPressCount = 0;
const unsigned long irWindowDuration = 15000; // ms

IRrecv irrecv(IR_PIN); // IR receiver setup
decode_results results;

bool unlocked = true; // unlocked state flag

SoftwareSerial mySerial(6, 7);
bool debug = false; // set true for debug prints

bool handleInput(byte* buffer, int numBytes, Message& requestMessage); // function definitions
void printMessage(const Message& m);

void setup() {
    mySerial.begin(SERIAL_BAUD);
    Serial.begin(SERIAL_BAUD);

    for (int i = 0; i < 4; i++) { // led pin configurations 
        pinMode(buttonPins[i], INPUT);
        pinMode(ledPins[i], OUTPUT);
        digitalWrite(ledPins[i], LOW);
    }

    pinMode(IR_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);

    irrecv.enableIRIn(); // start IR receiver
    delay(3000);
}

void loop() {
    Message response;
    unsigned long now = millis();

    if (mySerial.available() >= SIZE && mySerial.peek() == 'O') { // handles incoming form control hub
        byte readBuf[SIZE];
        mySerial.readBytes(readBuf, SIZE);
        Message requestMessage;
        if (handleInput(readBuf, SIZE, requestMessage)) {
            if (debug) {
                Serial.println("Input handled");
            }
        }
    }

    for (int i = 0; i < 4; i++) { // handles the button code entry and LED feedback
        int reading = digitalRead(buttonPins[i]);

        if (reading != lastReading[i]) {
            lastDebounceTime[i] = now;
        }

        if (now - lastDebounceTime[i] > debounceDelay) {
            if (reading != buttonState[i]) {
                buttonState[i] = reading;
                if (buttonState[i] == HIGH) {
                    if (unlocked) { // resets on any button if already unlocked
                        for (int j = 0; j < 4; j++) {
                            digitalWrite(ledPins[j], LOW);
                        }
                        unlocked = false;
                        codeIndex = 0;
                        irPressCount = 0;
                        irWindowStart = 0;
                        Serial.println("Reset Pin");
                        continue; // does not print struct on the reset
                    }

                    digitalWrite(ledPins[i], HIGH); // flashes led
                    {
                        unsigned long waitStart = millis();
                        while (millis() - waitStart < 100) {
                        }
                    }
                    digitalWrite(ledPins[i], LOW);

                    codeSequence[codeIndex++] = i + 1; // records the press

                    if (codeIndex >= 4) { // once 4 presses check if its valid
                        codeIndex = 0;
                        bool correct = true;
                        for (int k = 0; k < 4; k++) {
                            if (codeSequence[k] != correctCode[k]) {
                                correct = false;
                                break;
                            }
                        }

                        response.validIR = digitalRead(IR_PIN);
                        response.isMoving = digitalRead(PIR_PIN);
                        response.validPin = correct;
                        response.locked = !correct;
                        response.unlocked = correct;
                        response.to = 'C';
                        response.from = 'I';

                        if (correct) { // blink the LEDs 3x then stay on 
                            for (int b = 0; b < 3; b++) {
                                for (int j = 0; j < 4; j++) {
                                    digitalWrite(ledPins[j], HIGH);
                                }
                                {
                                    unsigned long waitStart = millis();
                                    while (millis() - waitStart < 100) {
                                    }
                                }
                                for (int j = 0; j < 4; j++) {
                                    digitalWrite(ledPins[j], LOW);
                                }
                                {
                                    unsigned long waitStart = millis();
                                    while (millis() - waitStart < 100) {
                                    }
                                }
                            }
                            for (int j = 0; j < 4; j++) {
                                digitalWrite(ledPins[j], HIGH);
                            }
                            unlocked = true;
                            Serial.println("Door Unlocked! Keypad");
                            printMessage(response);
                        } 
                        else { // wrong code → turn all LEDs off
                            for (int j = 0; j < 4; j++) {
                                digitalWrite(ledPins[j], LOW);
                            }
                            Serial.println("Door Locked!");
                            printMessage(response);
                        }

                        mySerial.write((byte*)&response, SIZE); // send struct no matter what
                        if (debug) {
                            Serial.println("Response sent");
                        }
                    }
                }
            }
        }
        lastReading[i] = reading;
    }

    if (irrecv.decode(&results)) { // handles IR presses for unlock
        irrecv.resume();
        if (!unlocked) {
            if (irWindowStart == 0 || now - irWindowStart > irWindowDuration) {
                irWindowStart = now;
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
                response.validIR   = 1;
                response.isMoving  = digitalRead(PIR_PIN);
                response.validPin  = false;
                response.locked    = false;
                response.unlocked  = true;
                response.to        = 'C';
                response.from      = 'I';

                for (int b = 0; b < 3; b++) { // blink all LEDs 3× then stay on
                    for (int j = 0; j < 4; j++) {
                        digitalWrite(ledPins[j], HIGH);
                    }
                    {
                        unsigned long waitStart = millis();
                        while (millis() - waitStart < 100) {
                        }
                    }
                    for (int j = 0; j < 4; j++) {
                        digitalWrite(ledPins[j], LOW);
                    }
                    {
                        unsigned long waitStart = millis();
                        while (millis() - waitStart < 100) {
                        }
                    }
                }

                for (int j = 0; j < 4; j++) {
                    digitalWrite(ledPins[j], HIGH);
                }

                unlocked = true;
                Serial.println("Door Unlocked! IR sensor");
                printMessage(response);

                mySerial.write((byte*)&response, SIZE);
                if (debug) {
                    Serial.println("IR unlock sent");
                }
            }
        }
    }
}

void printMessage(const Message& m) {
    Serial.print("Message => ");
    Serial.print("to="); Serial.print((char)m.to); Serial.print(", ");
    Serial.print("from="); Serial.print((char)m.from); Serial.print(", ");
    Serial.print("ultraSonicDistance=");Serial.print(m.ultraSonicDistance); Serial.print(", ");
    Serial.print("validIR="); Serial.print(m.validIR); Serial.print(", ");
    Serial.print("validPin=");Serial.print(m.validPin); Serial.print(", ");
    Serial.print("isMoving=");Serial.print(m.isMoving); Serial.print(", ");
    Serial.print("locked="); Serial.print(m.locked); Serial.print(", ");
    Serial.print("unlocked=");Serial.println(m.unlocked);
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
    return false;
}
