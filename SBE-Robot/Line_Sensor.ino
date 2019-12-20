int lastError = 0;
int whiteSense = 300;
int blackSense = 800;
bool k = false;

bool leftSignal() {
  if (sensorValues[0] < 100 && sensorValues[1] < 100 && sensorValues[2] > 800 && sensorValues[3] > 800 && sensorValues[4] > 800 && sensorValues[5] > 800) {
    return true;
  }
  return false;

}

bool rightSignal() {
  if (sensorValues[0] > 800 && sensorValues[1] > 800 && sensorValues[2] > 800 && sensorValues[3] > 800 && sensorValues[4] < 100 && sensorValues[5] < 100) {
    return true;
  }
  return false;
}

bool switchLines(int dir, NewPing sonars[]) { //Drive sideways until sees a black line
  int distances[SONAR_NUM];

  long startTime_1 = millis();
  updateSidePings(distances, sonars);
  while (millis() - startTime_1 < 1100) {
    updateSidePings(distances, sonars);
    if (tooClose(distances, 25, 25, 25)) {
      brake();
      timerB.tick();
      return false;
    } else {
      mDrive(-dir, 30);
    }
  }

  updateLines(1);
  while (allSenseWhite()) {
    updateLines(1);
    updateSidePings(distances, sonars);
    if (tooClose(distances, 25, 25, 25)) {

      brake();
      timerB.tick();
    } else {
      mDrive(-dir, 30);
    }
  }
  brake();
  delay(2000);
  return true;
}

void followLinePID(int dir) { // More accurate line following using a feedback loop. (Variable speed for motors)
  //float KP = 0.2;
  //float KD = 1.0;
  if (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3] + sensorValues[4] + sensorValues[5] + sensorValues[6] + sensorValues[7] > 200) {
    foundLine = true;
  }
  int setPoint;
  if (dir > 0) {
    setPoint = 2500;
  } else if (dir < 0) {
    setPoint = 3000;
  }
  int error = linePosition - setPoint;
  int motorSpeed = KP * error + KD * (error - lastError);

  lastError = error;

  int rSpeed = speedOffset + motorSpeed;
  int lSpeed = speedOffset - motorSpeed;
  /*
    Serial.print(linePosition);
    Serial.print("\t");
    Serial.print(lSpeed);
    Serial.print("\t");
    Serial.print(rSpeed);
    Serial.println();
  */
  if (rSpeed < 00) rSpeed = 0;
  if (lSpeed < 0) lSpeed = 0;
  if (rSpeed > 1023) rSpeed = 1023;
  if (lSpeed > 1023) lSpeed = 1023;
  long startTime;
  if (sensorValues[0] < whiteSense && sensorValues[1] < whiteSense && sensorValues[2] < whiteSense && sensorValues[3] < whiteSense && sensorValues[4] < whiteSense && sensorValues[5] < whiteSense && sensorValues[6] < whiteSense && sensorValues[7] < whiteSense) {

    if (!k) {
      startTime = millis();
      k = true;
    }
    if (millis() - startTime > 1000 && !foundLine) {
      brake();
    } else {
      driveLine(dir, lSpeed, rSpeed);
      // brake();
    }

  } else if (sensorValues[0] > blackSense && sensorValues[1] > blackSense && sensorValues[2] > blackSense && sensorValues[3] > blackSense && sensorValues[4] > blackSense && sensorValues[5] > blackSense && sensorValues[6] > blackSense && sensorValues[7] > blackSense) {
    brake();
  } else {
    driveLine(dir, lSpeed, rSpeed);
  }
}

void followLine() { // basic line Following. Turns if to far to one side.

  int error = linePosition - 2500;
  int lSpeed = lineSpeed;
  int rSpeed = lineSpeed;

  //sensorCheck();

  if (error < -500) {
    //Serial.println("turning Right");
    lSpeed = 250;
    rSpeed = 0;
  } else if (error > 500) {
    //Serial.println("turning Left");
    rSpeed = 250;
    lSpeed = 0;
  } else {
    //Serial.println("Driving Straight");
  }

  motorGo(0, CW, lSpeed);
  motorGo(1, CCW, rSpeed);
  motorGo(2, CW, lSpeed);
  motorGo(3, CCW, rSpeed);

}

void driveLine (int dir, int lSpeed, int rSpeed) {
  foundLine = true;
  if (dir > 0) {
    motorGo(0, CW, lSpeed);
    motorGo(1, CCW, rSpeed);
    motorGo(2, CW, lSpeed);
    motorGo(3, CCW, rSpeed);
  } else if (dir < 0) {
    motorGo(0, CCW, lSpeed);
    motorGo(1, CW, rSpeed);
    motorGo(2, CCW, lSpeed);
    motorGo(3, CW, rSpeed);
  }
}

void updateLines(int side) {
  if (side > 0) {
    linePosition = qtrF.readLineBlack(sensorValues);
    for (uint8_t i = 0; i < 6; i++)
    {
     // Serial.print(sensorValues[i]);
    //  Serial.print('\t');

    }
    //   Serial.println();
  } else {
    linePosition = qtrB.readLineBlack(sensorValues);
    for (uint8_t i = 0; i < SensorCount - 1; i++)
    {
      //  Serial.print(sensorValues[i]);
      //    Serial.print('\t');
    }
  }
///  Serial.println(linePosition);
}

void lineSensorsInit() {
  // configure the sensors
  qtrF.setTypeRC();
  qtrF.setSensorPins((const uint8_t[]) {
    31, 33, 37, 39, 43, 45
  }, 6);
  //  31, 33, 35, 37, 39, 41, 43, 45

  qtrB.setTypeRC();
  qtrB.setSensorPins((const uint8_t[]) {
    28, 30, 32, 34, 36, 38, 40, 42
  }, SensorCount - 1);

  delay(200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

  for (uint16_t i = 0; i < 1; i++)
  {
    qtrF.calibrate();
    qtrB.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indic ate we are through with calibration

  for (uint8_t i = 0; i < 6; i++) {
    qtrF.calibrationOn.minimum[i] = 1500; //1800 - 630
    qtrF.calibrationOn.maximum[i] = 1550; //2500
  }
  for (uint8_t i = 0; i < SensorCount - 1; i++) {
    qtrB.calibrationOn.minimum[i] = 750; //615
    qtrB.calibrationOn.maximum[i] = 2000; //2200
  }

  // Serial.println();
  delay(500);
}
