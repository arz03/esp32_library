#define BUTTON_PIN 12  // Use your GPIO pin (e.g., 26, 27, 33)

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use internal pull-up resistor
  Serial.println("Button Test Ready. Press the button...");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button Pressed!");
    delay(300);  // Debounce delay
  }
}
