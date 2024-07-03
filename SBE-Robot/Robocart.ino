#include <NewTone.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <QTRSensors.h>
#include <MedianFilter.h>
#include <timer.h>
#include <NewPing.h>

#define LINE1 true // Main Line
#define LINE2 false // Overtaking Line


// BOARD: ARDUINO MEGA 2560
 // REMEMBER TO SELECT RIGHT PORT #

bool line = LINE1;

bool toPackaging = true;

#define REPAIR true
#define PACKAGING false
bool endPoint = PACKAGING;

bool line1 = true;
bool line2 = false;

bool turnLeft = false;
bool turnRight = false;

int lineSpeed = 80;  // Initial Line Speed

bool lineEnd = true; // If true cant switch line

#define INSQUARE true
#define ENDSQUARE false
bool blackSquare = INSQUARE;

#define ARRIVING true
#define LEAVING false
bool state = LEAVING;
int signalCheck = 0;
long signalTime;

#define BRAKEVCC 0
#define CW   1
#define CCW  2
#define BRAKEGND 3

int inApin[4] = {7, 4, 22, 23};  // INA: Clockwise input
int inBpin[4] = {8, 9, 24, 25};  // INB: Counter-clockwise input
int pwmpin[4] = {5, 6, 44, 46};  // PWM input
int enpin[4] = {A0, A1, A8, A9}; // EN: Status of switches output (On/Off)
int statpin = 13;

//Ping Fronts
#define SONAR_NUM     3  // Number of sensors.
#define SONAR_NUM_FRONT     3  // Number of sensors.
#define MAX_DISTANCE 200 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 33 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

unsigned long pingTimer[SONAR_NUM_FRONT]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM_FRONT];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

NewPing sonar[SONAR_NUM_FRONT] = {     // Sensor object array.
  NewPing(47, 47, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(27, 27, MAX_DISTANCE),
  NewPing(26, 26, MAX_DISTANCE),
//    NewPing(49, 49, MAX_DISTANCE),
//    NewPing(51, 51, MAX_DISTANCE),
//    NewPing(48, 48, MAX_DISTANCE)
};

// Ping Sides
#define SONAR_NUM_S     3 // Number of sensors. 6 total for sides

unsigned long pingTimer_S[SONAR_NUM_S]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm_S[SONAR_NUM_S];         // Where the ping distances are stored.
uint8_t currentSensor_S = 0;          // Keeps track of which sensor is active.

NewPing sonar_S[SONAR_NUM_S] = {     // Sensor object array.
  NewPing(A3, A3, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(A4, A4, MAX_DISTANCE),
  NewPing(A5, A5, MAX_DISTANCE)
};

NewPing sonar_2[SONAR_NUM] = {     // Sensor object array.
  NewPing(A3, A3, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(A4, A4, MAX_DISTANCE),
  NewPing(A5, A5, MAX_DISTANCE)
};
unsigned int cm_2[SONAR_NUM];

NewPing sonar_4[SONAR_NUM] = {     // Sensor object array.
  NewPing(50, 50, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(53, 53, MAX_DISTANCE),
  NewPing(52, 52, MAX_DISTANCE)
};
unsigned int cm_4[SONAR_NUM];

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //physical mac address
byte ip[] = { 192, 168, 0, 100 }; // ip in lan
byte gateway[] = { 192, 168, 0, 1 }; // internet access via router
byte subnet[] = { 255, 255, 255, 0 }; //subnet mask
EthernetServer server(84); //server port

File webFile;

QTRSensors qtrF;
QTRSensors qtrB;
int linePosition;

MedianFilter pingF(10, 30);

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

boolean lineFollow = false;
bool foundLine = false;

// For lineFollowPID (currently not used)
float KP = 0.2;
float KD = 1.0;
int speedOffset = 650; //650
auto timer = timer_create_default(); // Old ping Update
auto timerB = timer_create_default(); // Buzzer
bool switchLine = false;
bool brakeM = false;

int timesChecked = 0;

bool sendFromPackaging = false;
bool sendFromRepair = false;
bool callFromPackaging = false;
bool callFromRepair = false;

int sendButtonPin = A14;
int positionAPin = A13;
int positionBPin = A12;
int sendButton = 1;
int positionA = 1;
int positionB = 1;

void setup() {
  Serial.begin(115200);
  delay(500);
  driveInit();
  lineSensorsInit();
  // ethernetSetup();
  brake();

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); // Deselect the SD card
  //  timer.every(5, updatePing);
  timerB.every(1000, beep);

  // Ping
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) { // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  }

  pingTimer_S[0] = millis() + 75;
  for (uint8_t i = 1; i < SONAR_NUM_S; i++) {
    pingTimer_S[i] = pingTimer_S[i - 1] + PING_INTERVAL;
  }

  pinMode(sendButtonPin, INPUT_PULLUP);
  pinMode(positionAPin, INPUT_PULLUP);
  pinMode(positionBPin, INPUT_PULLUP);

  lineFollow = true;

  if (state == LEAVING) {
    readButtons();
    while (sendButton == HIGH) {
      readButtons();
    }
    beep();
  }

  long startT;
  while (millis() - startT < 1200) {
    updatePings();
  }


}

void beep() {
  NewTone(10, 440, 200);
  NewTone(11, 440, 200);
  delay(200);
  noNewTone(10);
  noNewTone(11);
  delay(200);
}

int spd = 100;
void loop() { //35
  //updatePings();
  //updateSidePings(cm_2, sonar_2);
  // updateSidePings(cm_4, sonar_4);
  //updateLines(1);
 
  testOne();
  //readButtons();
}

void readButtons() {
  sendButton = digitalRead(sendButtonPin);
  positionA = digitalRead(positionAPin);
  positionB = digitalRead(positionBPin);
  /*Serial.print(sendButton);
    Serial.print(" ");
    Serial.print(positionA);
    Serial.print(" ");
    Serial.print(positionB);
    Serial.print(" ");
    Serial.println();*/
}

//switchline delay



bool switchLine1 = false;
void testOne() {
  updatePings();

  if (millis() > 2000) {
    lineFollow = true;
    updateLines(1);

    if (allSenseWhite()) {
      lineFollow = false;
    } else if (tooClose(cm, 50, 55, 55)) { // FRONT PING SENSOR DISTANCE (LEFT, RIGHT RIGHT) --- Part 1
      if (timesChecked >= 3) {
        if (!tooClose(cm_S, 70, 65, 70)) { // SIDE PING SENSOR DISTANCE (Switching Lines)
          switchLine = true;
        }
      } else {
        switchLine = false;
      }
      timesChecked++;

      lineFollow = false;
      long startTime = millis();
      while (millis() - startTime < 4000) {
        updatePings();
        if (!Serial.available() && millis() - startTime > 1000) {
          timerB.tick();
        }
        brake();
      }
      updatePings();
      // Check Again with all sensors so it dosen't start and stop
      if (tooClose(cm, 50, 55, 55)) {  // FRONT PING SENSOR DISTANCE (LEFT, RIGHT RIGHT) --- Part 2
        //  brakeM = false;
        lineFollow = false;
      }

    } else {
      lineFollow = true;
    }

    if (allSenseBlack()) { // At a black Square

      if (line1) {
        if (blackSquare == INSQUARE) {
          if (state == ARRIVING) {
            brake();
            delay(2000);
            turn(-1, 25); // TURN SPEED
            delay(800);
            updateLines(1);
            while (allSenseWhite()) {
              updateLines(1);
              turn(-1, 25);  // TURN SPEED
            }
            brake();
            delay(2000);
            blackSquare = ENDSQUARE;
            state = LEAVING;
          } else { //state == LEAVING
            brake();
            delay(2000);
            turn(1, 25); // TURN SPEED
            delay(800);
            updateLines(1);
            while (allSenseWhite()) {
              updateLines(1);
              turn(1, 25); // TURN SPEED
            }
            brake();
            delay(2000);
            blackSquare = INSQUARE;
            state = ARRIVING;
          }
          //turnleft 90
        } else { //square == ENDSQUARE
          turnAround();
          blackSquare = INSQUARE;
          state = LEAVING;
        }

      } else {
        brake();
        delay(2000);
        switchLine = true;
      }


    }

  } else {
    lineFollow = false;
  }

  if (switchLine) {

    if (!lineEnd) {
      line = !line;
      line1 = !line1;
      line2 = !line2;
      timesChecked = 0;

      //   drive(-1, 100);

      delay(1000);
      brake();
      if (line) {
        if (!switchLines(1, sonar_2)) {
          line = !line;
        }
      } else {
        if (!switchLines(-1, sonar_4)) {
          line = !line;
        }
      }
    }
    switchLine = false;
  } else if (lineFollow) {
    timesChecked = 0;
    if (leftSignal()) {

      lineSpeed = 80; // LOWEST SPEED along endpoints 0-255
      lineEnd = true;
    } else if (rightSignal()) {

      lineSpeed = 110;  // MAX SPEED Along main line 0-255
      lineEnd = false;
    } else {
      followLine();
    }
  } else {
    brake();
  }
}

void turnAround() {
  toPackaging = !toPackaging;
  line = !line;
  endPoint = !endPoint;
  brake();
  delay(2000);
  turn(1, 40);  // TURN AROUND SPEED Pt 1
  delay(1500);
  updateLines(1);
  while (allSenseWhite()) {
    updateLines(1);
    turn(1, 40);  // TURN AROUND SPEED Pt 2
  }
  brake();
  //delay(2000);
  readButtons();
  while (sendButton == HIGH) {
    readButtons();
    if (positionA == LOW) { //Packaging
      if (endPoint == REPAIR) {
        line = LINE1;
      }
    }

    if (positionB == LOW) { // REPAIR
      if (endPoint == PACKAGING) {
        line = LINE2;
      }
    }
  }
  beep();
}

bool allSenseWhite() {
  for (int i = 0; i < 6; i++) {
    if (sensorValues[i] > 800) {
      return false;
    }
  }
  //Serial.println("ALL WHITE");
  return true;
}

bool allSenseBlack() {
  for (int i = 0; i < 6; i++) {
    if (sensorValues[i] < 100) {
      return false;
    }
  }
  //Serial.println("ALL BLACK");
  return true;
}

bool tooClose(unsigned int distances[], int maxDistL, int maxDistC, int maxDistR) {
  if (distances[0] <= maxDistL && distances[0] != 0) {
    return true;
  }
  if (distances[1] <= maxDistC && distances[1] != 0) {
    return true;
  }
  if (distances[2] <= maxDistR && distances[2] != 0) {
    return true;
  }
  return false;
}
