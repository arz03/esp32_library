#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SDA_PIN 26
#define SCL_PIN 32

#define BUTTON_PIN 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int buttonState = HIGH;
int lastButtonState = HIGH;
int pressCount = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // in milliseconds

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Press Count:");
  display.setCursor(0, 30);
  display.println("0");
  display.display();

  Serial.println("Button Test Ready. Press the button...");
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  // If the reading changed, reset the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only act if the button state has been stable for debounceDelay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && buttonState == HIGH) {
      pressCount++;
      Serial.print("Button pressed ");
      Serial.print(pressCount);
      Serial.println(" times");

      // Update OLED
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Press Count:");
      display.setCursor(0, 30);
      display.println(pressCount);
      display.display();
    }
    buttonState = reading;
  }

  lastButtonState = reading;
}
