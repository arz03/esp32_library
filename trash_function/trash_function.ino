#include <ESP32Servo.h>

Servo flap;
Servo bucket;
const int servoPin1 = 27; // Flap servo
const int servoPin2 = 33; // Bucket servo
const int homeAngle1 = 90; // Flap home
const int homeAngle2 = 0; // Bucket home

void setup() {
  flap.attach(servoPin1);
  bucket.attach(servoPin2);
  flap.write(90); // Flap home
  delay(1000);
  bucket.write(0);  // Bucket home
  delay(1000);
}

void trash(const char* text) {
  bucket.write(0);
  delay(500);
  flap.write(90);
  delay(500);

  if (strcmp(text, "plastic") == 0) { // right quadrant in 1st half
    flap.write(0); // right
    delay(3000);
    flap.write(90); // reset to home
    delay(500);
  } else if (strcmp(text, "metal") == 0) { // left quadrant in 1st half
    flap.write(180); // left
    delay(3000);
    flap.write(90); // reset to home
    delay(500);
  } else if (strcmp(text, "glass") == 0) { // top right quadrant (when home position)
    bucket.write(180);
    delay(1000);
    flap.write(180); // left
    delay(3000);
    flap.write(90); // reset to home
    delay(500);
    bucket.write(0);
    delay(1000);
    } else if (strcmp(text, "paper") == 0) { // top left quadrant (when home position)
    bucket.write(180);
    delay(1000);
    flap.write(0); // right
    delay(3000);
    flap.write(90); // reset to home
    delay(500);
    bucket.write(0);
    delay(1000);
  }
}



void loop(){
  trash("plastic");
  trash("glass");
  trash("metal");
  trash("paper");

  // to show end: 
  flap.write(45);
  delay(500);
  flap.write(135);
  delay(500);
  flap.write(90);
  delay(500);
}