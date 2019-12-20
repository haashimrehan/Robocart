void driveInit() {
  pinMode(statpin, OUTPUT);

  // Initialize digital pins as outputs
  for (int i = 0; i < 4; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwmpin[i], OUTPUT);
    pinMode(enpin[i], OUTPUT);
  }
  // Initialize braked
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
  // Enable all driving pins
  digitalWrite(enpin[0], HIGH);
  digitalWrite(enpin[1], HIGH);
  digitalWrite(enpin[2], HIGH);
  digitalWrite(enpin[3], HIGH);
  brake();
}


void drive(int dir, int Spd) {
  int mappedSpeed = map (Spd, 0, 100, 0, 255);

  if (dir >= 1) { // Forward
    motorGo(0, CW, mappedSpeed);
    motorGo(1, CCW, mappedSpeed);
    motorGo(2, CW, mappedSpeed);
    motorGo(3, CCW, mappedSpeed);
  } else if (dir <= -1)  { // Reverse
    motorGo(0, CCW, mappedSpeed);
    motorGo(1, CW, mappedSpeed);
    motorGo(2, CCW, mappedSpeed);
    motorGo(3, CW, mappedSpeed);
  }
}

void turn (int dir, int Spd) {
  int mappedSpeed = map (Spd, 0, 100, 0, 255);

  if (dir >= 1) { // Right
    motorGo(0, CW, mappedSpeed);
    motorGo(1, CW, mappedSpeed);
    motorGo(2, CW, mappedSpeed);
    motorGo(3, CW, mappedSpeed);
  } else if (dir <= -1) { // Left
    motorGo(0, CCW, mappedSpeed);
    motorGo(1, CCW, mappedSpeed);
    motorGo(2, CCW, mappedSpeed);
    motorGo(3, CCW, mappedSpeed);
  }
}

// 01
// 23

void mDrive(int dir, int Spd) {
  int mappedSpeed = map (Spd, 0, 100, 0, 255);

  if  (dir > 0) { // Right
    motorGo(1, CW, mappedSpeed);
    motorGo(2, CCW, mappedSpeed);
    motorGo(0, CW,  mappedSpeed);
    motorGo(3, CCW,  mappedSpeed);
  } else if (dir < 0) { // Left
    motorGo(1, CCW, mappedSpeed);
    motorGo(2, CW, mappedSpeed);
    motorGo(0, CCW,  mappedSpeed);
    motorGo(3, CW,  mappedSpeed);
  }
}

void brake() {
  for (int i = 0; i < 4; i++) {
    motorOff(i);
  }
}

void motorOff(int motor)
{
  // Initialize braked
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
  analogWrite(pwmpin[motor], 0);
}

/*
  0: Brake to VCC
  1: Clockwise
  2: Counter Clockwise
  3: Brake to GND
*/
void motorGo(byte motor, byte direct, byte pwm)
{
  if (motor <= 3)
  {
    if (direct <= 4)
    {
      // Set inA[motor]
      if (direct <= 1)
        digitalWrite(inApin[motor], HIGH);
      else
        digitalWrite(inApin[motor], LOW);

      // Set inB[motor]
      if ((direct == 0) || (direct == 2))
        digitalWrite(inBpin[motor], HIGH);
      else
        digitalWrite(inBpin[motor], LOW);

      analogWrite(pwmpin[motor], pwm);
    }
  }
}
