//INCLUDE STATEMENTS
#include <SoftwareSerial.h>

//CUSTOM STRUCTURES

struct OutsidePackage {
  int micValue;
  bool validIR;
  bool validRFID;
  bool validPin;
};

struct InsidePackage {
  bool validIR;
  bool isMoving;

};

struct ControlHubPackage {
  int status;
};
