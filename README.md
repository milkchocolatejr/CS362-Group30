## Abstract

The Dorm Doorman is a dorm security system that uses three Arduino boards.

- The Outside Arduino asks visitors for a password on a keypad or uses an IR remote to enter a pin, both of which can grant access.  
- The Inside Arduino watches for unwanted entry with an IR sensor and accelerometer.  
- Both of these Arduinos send their status messages over SoftwareSerial to the Control Hub Arduino.  
- The Control Hub shows the door’s current state on a LCD screen and does sound alerts through a buzzer.  
- If the wrong code is entered too many times or the door is forced open, the hub displays the current state and sounds an alarm.  
