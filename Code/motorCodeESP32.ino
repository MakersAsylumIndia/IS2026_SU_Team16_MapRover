#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "Jiofiber4g";
const char* password = "jyoti123";

WebServer server(80);

// Motor A
const int ENA = 14;
const int IN1 = 27;
const int IN2 = 26;

// Motor B
const int ENB = 32;
const int IN3 = 25;
const int IN4 = 33;

// New ESP32 core 3.x PWM — no channels needed
const int PWM_FREQ = 1000;
const int PWM_RES  = 8;

int testSpeed = 110;

// ─── HTML Page ─────────────────────────────────────────────────
// Raw string replaced with regular string to avoid IDE issues
const char* htmlPage =
  "<!DOCTYPE html><html>"
  "<head>"
  "<title>Rover Control</title>"
  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
  "<style>"
  "body{display:flex;flex-direction:column;align-items:center;"
  "font-family:sans-serif;background:#111;color:white;}"
  "button{width:80px;height:80px;font-size:28px;margin:8px;"
  "border-radius:12px;border:none;background:#333;color:white;cursor:pointer;}"
  "button:active{background:#555;}"
  ".row{display:flex;}"
  "</style></head>"
  "<body>"
  "<h2>Rover Control</h2>"
  "<div class='row'><button onclick=\"send('F')\">&#9650;</button></div>"
  "<div class='row'>"
  "<button onclick=\"send('L')\">&#9664;</button>"
  "<button onclick=\"send('S')\">&#9632;</button>"
  "<button onclick=\"send('R')\">&#9654;</button>"
  "</div>"
  "<div class='row'><button onclick=\"send('B')\">&#9660;</button></div>"
  "<script>"
  "function send(cmd){fetch('/cmd?v='+cmd);}"
  "</script>"
  "</body></html>";

// ─── Motor Functions ───────────────────────────────────────────

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  ledcWrite(ENA, testSpeed);                        // New API: write directly to pin
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENB, testSpeed);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  ledcWrite(ENA, testSpeed);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  ledcWrite(ENB, testSpeed);
}

void turnLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  ledcWrite(ENA, testSpeed);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENB, testSpeed);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  ledcWrite(ENA, testSpeed);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  ledcWrite(ENB, testSpeed);
}

void stopMotors() {
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

// ─── Setup ─────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // New API: ledcAttach(pin, freq, resolution) — no channels
  ledcAttach(ENA, PWM_FREQ, PWM_RES);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);

  stopMotors();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("Open this IP in your browser: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/cmd", []() {
    if (server.hasArg("v")) {
      char cmd = server.arg("v")[0];
      if      (cmd == 'F') moveForward();
      else if (cmd == 'B') moveBackward();
      else if (cmd == 'L') turnLeft();
      else if (cmd == 'R') turnRight();
      else if (cmd == 'S') stopMotors();
    }
    server.send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("Server started.");
}

// ─── Loop ──────────────────────────────────────────────────────

void loop() {
  server.handleClient();
}