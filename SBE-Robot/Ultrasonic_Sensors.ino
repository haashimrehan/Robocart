void updatePings() {
  for (uint8_t i = 0; i < SONAR_NUM_FRONT; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM_FRONT;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM_FRONT - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
}

void updateSidePings(int distances[], NewPing sonars[]) {
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through each sensor and display results.
    delay(35);
    distances[i] = sonars[i].ping_cm();
    Serial.print(" ");
    Serial.print(distances[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void updatePings_Sides() {
  for (uint8_t i = 0; i < SONAR_NUM_S; i++) {
    if (millis() >= pingTimer_S[i]) {
      pingTimer_S[i] += PING_INTERVAL * SONAR_NUM_S;
      if (i == 0 && currentSensor_S == SONAR_NUM_S - 1) oneSensorCycle_S();
      sonar_S[currentSensor_S].timer_stop();
      currentSensor_S = i;
      cm_S[currentSensor_S] = 0;
      sonar_S[currentSensor_S].ping_timer(echoCheck_S);
    }
  }
}

void echoCheck_S() { // If ping received, set the sensor distance to array.
  if (sonar_S[currentSensor_S].check_timer())
    cm_S[currentSensor_S] = sonar_S[currentSensor_S].ping_result / US_ROUNDTRIP_CM;
}

void echoCheck() {
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void oneSensorCycle_S() {

  for (uint8_t i = 0; i < SONAR_NUM_S; i++) {
    if (cm_S[i] <= 70 && cm_S[i] != 0) { // if something is too close
      // If stopped And nothing is beside switch Lines
      //    switchLine = false;
    } else if (cm_S[i] > 70 || cm_S[i] == 0) {
      //  switchLine = true;
    }
    //  Serial.print(" ");
    //Serial.print(cm_S[i]);
    // Serial.print(" ");
  }
  // if ((cm_S[0] > 70 || cm_S[0] == 0) && (cm_S[1] > 70 || cm_S[1] == 0) && (cm_S[2] > 70 || cm_S[2] == 0)) { // if something is too close
  //   switchLine = true;
  // } else {
  //  switchLine = false;
  // }
  //Serial.println();
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  for (uint8_t i = 0; i < SONAR_NUM_FRONT; i++) {
    // if (cm[i] <= 60 && cm[i] != 0) { // If any sensor detects something too close
    //   lineFollow = false;
    //   brakeM = true;
    //   brake();
    // break; // no need to check other sensors.
    Serial.print(" ");
       Serial.print(cm[i]);
     Serial.print(" ");
  }
  Serial.println();

}
