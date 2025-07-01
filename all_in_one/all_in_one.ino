#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <qrcodex.h>
#include <ESP32Servo.h>

// Wi-Fi credentials
const char* ssid = "rr";
const char* password = "asdfghjkl";

// Servo objects
Servo flap;
Servo bucket;

// Servo control pins and home positions
const int servoPin1 = 27;  // Flap servo
const int servoPin2 = 33;  // Bucket servo
const int homeAngle1 = 90; // Flap home
const int homeAngle2 = 0;  // Bucket home

// OLED configuration
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int OLED_RESET = -1;
const int SDA_PIN = 26;
const int SCL_PIN = 32;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button pin
const int BUTTON_PIN = 12;


// Session Variables
String sessionCode = "";
String qrURL = "";
bool sessionActive = false;
bool loggedIn = false;


void setup() {

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (1);
  }


  // Connecting to wifi
  displayMessage("Connecting WiFi...");
  WiFi.mode(WIFI_STA);  // Set ESP32 to Station Mode
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  displayMessage("WiFi Connected\nPress Button");


  // Servo motors setup
  flap.attach(servoPin1);
  bucket.attach(servoPin2);
  flap.write(90); // Flap home
  delay(1000);
  bucket.write(0);  // Bucket home
  delay(1000);
}





void loop() {
  if (!sessionActive) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(300); // Debounce
      displayMessage("Starting Session...");
      if (initiateSession()) {
        sessionActive = true;
      } else {
        resetMachine();
      }
    }
  } else if (!loggedIn) {
    displayMessage("Waiting for Login...");
    if (checkSession()) {
      loggedIn = true;
      displayMessage("User Logged In\nPut Waste\nPress Button");
    } else {
      delay(2000); // Poll every 2 seconds
    }
  } else {
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(300); // Debounce
      displayMessage("Processing Waste...");
      if (processWaste()) {
        terminateSession();
        sessionActive = false;
        loggedIn = false;
        displayMessage("Session Complete\nPress to Start");
      } else {
        resetMachine();
      }
    }
  }
}








// OLED Display Helper
void displayMessage(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(message);
  display.display();
}

// Mock API: Initiate Session
bool initiateSession() {
  // Mock response
  sessionCode = "MOCK123";
  qrURL = "https://ur.com/a/" + sessionCode;

  Serial.println("Session code: " + sessionCode);
  displayQRCode(qrURL.c_str());w

  delay(1000); // Simulate network delay
  return true; // Always succeed
}

// QR Code Display
void displayQRCode(const char* text) {
  // Create a QR code object
  QRCode qrcode;
  
  // Define the size of the QR code (1-40, higher means bigger size)
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

  // Clear the display
  display.clearDisplay();

  // Calculate the scale factor
  int scale = min(SCREEN_WIDTH / qrcode.size, SCREEN_HEIGHT / qrcode.size);
  
  // Calculate horizontal shift
  int shiftX = (SCREEN_WIDTH - qrcode.size*scale)/2;
  
  // Calculate horizontal shift
  int shiftY = (SCREEN_HEIGHT - qrcode.size*scale)/2;

  // Draw the QR code on the display
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(shiftX+x * scale, shiftY + y*scale, scale, scale, WHITE);
      }
    }
  }

  // Update the display
  display.display();
}

// Mock API: Polling Login
bool checkSession() {
  static int attempts = 0;
  attempts++;

  if (attempts >= 3) { // Simulate user logging in after 3 polls
    Serial.println("User logged in (mock)");
    return true;
  }

  Serial.println("User not logged in yet (mock)");
  delay(500); // Simulate polling delay
  return false;
}


// Mock API: Process Waste
bool processWaste() {
  // Mock waste type and coins
  String wasteTypes[] = {"plastic", "glass", "metal", "paper"};
  String selectedWaste = wasteTypes[random(0, 4)]; // Randomly pick a type
  int coins = random(1, 5); // Random coin count

  Serial.println("Waste Type: " + selectedWaste + " | Coins: " + String(coins));
  displayMessage("Sorting: " + selectedWaste + "\nCoins: " + String(coins));
  sortWaste(selectedWaste.c_str());

  delay(1000); // Simulate processing time
  return true;
}


// Waste Sorting using 2 Servos
void sortWaste(const char* text) {
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

// Mock API: Terminate Session
void terminateSession() {
  Serial.println("Session terminated (mock)");
}

// Mock API: Reset Machine
void resetMachine() {
  Serial.println("Machine reset (mock)");
  delay(2000);
  ESP.restart();
}






() {
  // put your main code here, to run repeatedly:

}
