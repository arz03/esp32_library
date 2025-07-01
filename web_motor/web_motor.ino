#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "ESP32-Servo";
const char* password = "12345678";

WebServer server(80);
Servo myServo;

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>Servo Control</title></head>
<body>
  <h2>Servo Angle: <span id="val">90</span>°</h2>
  <input type="range" min="0" max="180" value="90" id="slider" oninput="updateFromSlider(this.value)">
  <input type="number" min="0" max="180" value="90" id="angleInput" oninput="updateFromInput(this.value)">
  <script>
    let debounceTimer;

    function sendAngle(val) {
      fetch("/angle?value=" + val);
    }

    function updateFromSlider(val) {
      document.getElementById("val").innerText = val;
      document.getElementById("angleInput").value = val;
      debounceSend(val);
    }

    function updateFromInput(val) {
      val = Math.min(180, Math.max(0, val));
      document.getElementById("val").innerText = val;
      document.getElementById("slider").value = val;
      debounceSend(val);
    }

    function debounceSend(val) {
      clearTimeout(debounceTimer);
      debounceTimer = setTimeout(() => sendAngle(val), 300);
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", html);
}

void handleAngle() {
  if (server.hasArg("value")) {
    int angle = server.arg("value").toInt();
    angle = constrain(angle, 0, 180);
    myServo.write(angle);
    server.send(200, "text/plain", "Angle set to: " + String(angle));
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println("AP Started. Connect to WiFi and open browser at 192.168.4.1");

  myServo.attach(27); // GPIO 27

  server.on("/", handleRoot);
  server.on("/angle", handleAngle);
  server.begin();
}

void loop() {
  server.handleClient();
}
