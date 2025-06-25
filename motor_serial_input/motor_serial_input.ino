#include <ESP32Servo.h>

Servo myServo;

void setup() {
  Serial.begin(9600);       // Start Serial communication
  myServo.attach(33);         // Attach the servo to GPIO 27
  Serial.println("Enter angle (0 to 180):");
}

void loop() {
  if (Serial.available() > 0) {
    int angle = Serial.parseInt();  // Read angle as an integer
    if (angle >= 0 && angle <= 180) {
      myServo.write(angle);         // Move servo to entered angle
      Serial.print("Moved to: ");
      Serial.println(angle);
    } else {
      Serial.println("Invalid angle. Please enter 0–180.");
    }
  }
}
