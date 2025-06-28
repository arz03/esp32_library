#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <qrcodex.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
const char* API_URL = "http://192.168.20.233:5000/machine";
const char* API_KEY = "123";
const char* ssid = "One";
const char* password = "a38aggu4";
const char* machineId = "1750080734343";
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
  HTTPClient http;
  String url = String(API_URL) + "/initiate?id=" + machineId;
  http.begin(url);
  http.addHeader("x-api-key", API_KEY);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    sessionCode = doc["sessionCode"].as<String>();
    qrURL = "http://192.168.20.230:3000/machine/" + sessionCode;
    displayQRCode(qrURL.c_str());
    http.end();
    return true;
  }
  http.end();
  return false;
}

void displayQRCode(const char* text) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);
  display.clearDisplay();
  int scale = min(SCREEN_WIDTH / qrcode.size, SCREEN_HEIGHT / qrcode.size);
  int shiftX = (SCREEN_WIDTH - qrcode.size * scale) / 2;
  int shiftY = (SCREEN_HEIGHT - qrcode.size * scale) / 2;
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(shiftX + x * scale, shiftY + y * scale, scale, scale, WHITE);
      }
    }
  }
  display.display();
}

bool checkSession() {
  HTTPClient http;
  String url = String(API_URL) + "/check?id=" + machineId;
  http.begin(url);
  http.addHeader("x-api-key", API_KEY);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload);
    bool isLoggedIn = doc["loggedIn"];
    http.end();
    return isLoggedIn;
  }
  http.end();
  return false;
}

bool processWaste() {
  HTTPClient http;
  String url = String(API_URL) + "/start";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", API_KEY);

  DynamicJsonDocument requestBody(512);
  requestBody["id"] = machineId;
  requestBody["sessionCode"] = sessionCode;
  String requestBodyString;
  serializeJson(requestBody, requestBodyString);

  int httpCode = http.POST(requestBodyString);
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload);
    String wasteType = doc["type"].as<String>();
    int coins = doc["coins"];
    displayMessage("Sorting: " + wasteType + "\nCoins: " + String(coins));
    sortWaste(wasteType);
    http.end();
    return true;
  }
  http.end();
  return false;
}

void sortWaste(String text) {
  bucket.write(0);
  delay(500);
  flap.write(90);
  delay(500);

  if (text == "plastic") {
    flap.write(0);
  } else if (text == "metal") {
    flap.write(180);
  } else if (text == "glass") {
    bucket.write(180);
    delay(1000);
    flap.write(180);
    delay(3000);
    flap.write(90);
    bucket.write(0);
    return;
  } else if (text == "paper") {
    bucket.write(180);
    delay(1000);
    flap.write(0);
    delay(3000);
    flap.write(90);
    bucket.write(0);
    return;
  }

  delay(3000);
  flap.write(90);
  delay(500);
}

void terminateSession() {
  HTTPClient http;
  String url = String(API_URL) + "/terminate";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", API_KEY);
  DynamicJsonDocument requestBody(256);
  requestBody["id"] = machineId;
  requestBody["sessionCode"] = sessionCode;
  String requestBodyString;
  serializeJson(requestBody, requestBodyString);
  http.POST(requestBodyString);
  http.end();
}

void resetMachine() {
  HTTPClient http;
  String url = String(API_URL) + "/reset";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", API_KEY);
  DynamicJsonDocument requestBody(256);
  requestBody["id"] = machineId;
  String requestBodyString;
  serializeJson(requestBody, requestBodyString);
  http.POST(requestBodyString);
  http.end();
  delay(2000);
  ESP.restart();
}





