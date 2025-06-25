#include <ESP32Servo.h>

Servo myServo;
Servo myServo2;
const int servoPin1 = 27; // Flap servo
const int servoPin2 = 33; // Bucket servo
const int homeAngle1 = 90; // Flap home
const int homeAngle2 = 0; // Bucket home

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin1);
  myServo2.attach(servoPin2);
  myServo.write(homeAngle1);
  delay(1000);  // Start Flap at home
}

void loop() {
  // Move flap to home (90)
  moveTo(homeAngle1);
  delay(1000);
  // moveTo(100);
  // delay(400);
  // moveTo(80);
  // delay(400);

  // Move bucket to home (0)
  myServo2.write(0);
  delay(2000);

  // Move flap to 180 via home
  moveTo(homeAngle1);
  delay(500);
  moveTo(180);
  delay(1000);
  moveTo(homeAngle1);

  // Move bucket

  delay(1000);
  myServo2.write(180);
  delay(1000);

  // Return flap to home before next move
  moveTo(homeAngle1);
  delay(500);
  moveTo(0);
  delay(1000);
  moveTo(homeAngle1);
  delay(1000);

  // Return bucket to home
  myServo2.write(0);
  delay(1000);
}

void moveTo(int angle) {
  Serial.println("Moving to: " + String(angle));
  myServo.write(angle);
}
