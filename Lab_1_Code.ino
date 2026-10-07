#include <Arduino.h>
#include "geeWhiz.h"

int MOT_PIN = A0;
int BAL_PIN = A1;

char voltageInput[24];
size_t inputLength = 0;

void setup() {
  analogReadResolution(14);
  pinMode(A5, OUTPUT);

  Serial.begin(115200);
  delay(300);

  geeWhizBegin();
  set_control_interval_ms(5);
  setMotorVoltage(0.0f);

  Serial.println("geeWhiz Started");
}

void loop() {

}

void interval_control_code(void) {
  int maxy = 17000;
  int miny = 0;

  int motor = analogRead(MOT_PIN);
  int ball = analogRead(BAL_PIN);

  // Linearize Theta
  float m = ((-PI / 4.0f) - (PI / 4.0f)) / (6287.0f - 2048.0f);
  float offset = -m * 3983.0f;
  float theta = m * motor + offset;

  // Change angle between -0.8 and +0.8 rad
  float phase = fmodf(millis() / 1000.0f, 36.0f);
  float thetaRef;

  if (phase < 8.0f) {
    thetaRef = 0.1f * phase;
  } 
  
  else if (phase < 10.0f) {
    thetaRef = 0.8f;
  } 
  
  else if (phase < 18.0f) {
    thetaRef = 0.8f - 0.1f * (phase - 10.0f);
  } 
  
  else if (phase < 26.0f) {
    thetaRef = -0.1f * (phase - 18.0f);
  } 
  
  else if (phase < 28.0f) {
    thetaRef = -0.8f;
  } 
  
  else {
    thetaRef = -0.8f + 0.1f * (phase - 28.0f);
  }

  // Theta Saturation
  float thetaRefSat = constrain(thetaRef, -0.7f, 0.7f);

  // Holds changing target angle for certain amount of time
  /*int holdTime = 750;
  unsigned long timeMs = millis();
  if ((timeMs / holdTime) % 2 == 0) {
    thetaRef = -0.1f;
  } else {
    thetaRef = 0.1f;
  }*/

  // Add proportional feedback  
  const float Kp = -20.0f;
  float error = thetaRefSat - theta;
  float commandVoltage = Kp * error;

  // Compensate for striction
  float motorVoltage = commandVoltage;
  if (commandVoltage > 0.0f) {
    motorVoltage += 0.14f;
  } else if (commandVoltage < 0.0f) {
    motorVoltage -= 0.14f;
  }

  // Change voltage to compensate for striction
  motorVoltage = constrain(motorVoltage, -5.9f, 5.9f);
  setMotorVoltage(motorVoltage);

  digitalWrite(A5, HIGH);

  Serial.print(millis());
  Serial.print(",");
  Serial.print(thetaRef, 5);
  Serial.print(",");
  Serial.print(thetaRefSat, 5);
  Serial.print(",");
  Serial.println(theta, 5);

  digitalWrite(A5, LOW);
}