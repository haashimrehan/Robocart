void testMotors() { //Go through speeds of motors
  for (int i = 0; i <= 100; i += 10) {
    drive(1, i);
    delay(2000);
  }
   for (int i = 0; i <= 100; i += 10) {
    drive(-1, i);
    delay(2000);
  }
}

void setSpeedMotor (int motor) {
  if (motor == 0 || motor == 2) {
    digitalWrite(inApin[motor], HIGH);
    digitalWrite(inBpin[motor], LOW);
  } else {
    digitalWrite(inApin[motor], LOW);
    digitalWrite(inBpin[motor], HIGH);
  }

  for (int i = 0; i <= 255; i += 51) {
    analogWrite(pwmpin[motor], i);
    delay(2000);
  }
}

void setSpeedsMotors() {
//  setSpeedMotor(0);
  setSpeedMotor(1);
  setSpeedMotor(2);
  setSpeedMotor(3);
}
