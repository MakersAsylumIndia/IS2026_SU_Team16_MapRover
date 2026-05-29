#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

const char* ssid     = "Jiofiber4g";
const char* password = "jyoti123";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Motor A - right
const int ENA = 14;
const int IN1 = 27;
const int IN2 = 26;

// Motor B - left
const int ENB = 32;
const int IN3 = 25;
const int IN4 = 33;

const int PWM_FREQ = 1000;
const int PWM_RES  = 8;

// ─── Motor Functions (accept speed) ───────────────────────────

void moveForward(int spd) {
  ledcWrite(ENA, spd); ledcWrite(ENB, spd);          // Set speed FIRST
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void moveBackward(int spd) {
  ledcWrite(ENA, spd); ledcWrite(ENB, spd);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnLeft(int spd) {
  ledcWrite(ENA, spd); ledcWrite(ENB, spd);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // Right: forward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // Left: backward
}

void turnRight(int spd) {
  ledcWrite(ENA, spd); ledcWrite(ENB, spd);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // Right: backward
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  // Left: forward
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  ledcWrite(ENA, 0); ledcWrite(ENB, 0);              // Kill PWM AFTER pins
}
// ─── WebSocket Handler ─────────────────────────────────────────
// Commands: "F180", "B110", "L200", "R200", "S"
// First char = direction, rest = speed value

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_DATA) {
    String msg = "";
    for (size_t i = 0; i < len; i++) msg += (char)data[i];

    char cmd = msg[0];
    int spd = 150; // fallback default
    if (msg.length() > 1) {
      spd = constrain(msg.substring(1).toInt(), 80, 255);
    }

    if      (cmd == 'F') moveForward(spd);
    else if (cmd == 'B') moveBackward(spd);
    else if (cmd == 'L') turnLeft(spd);
    else if (cmd == 'R') turnRight(spd);
    else if (cmd == 'S') stopMotors();
  }
}

// ─── HTML Page ─────────────────────────────────────────────────

const char* htmlPage =
  "<!DOCTYPE html><html>"
  "<head>"
  "<title>Rover Control</title>"
  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
  "<style>"
  "body{display:flex;flex-direction:column;align-items:center;"
  "font-family:sans-serif;background:#111;color:white;padding-top:20px;}"

  // Direction buttons
  "button{width:80px;height:80px;font-size:28px;margin:8px;"
  "border-radius:12px;border:none;background:#333;color:white;cursor:pointer;"
  "-webkit-tap-highlight-color:transparent;}"
  "button:active{background:#555;}"
  ".row{display:flex;}"

  // Speed slider
  "#speed-wrap{margin-top:24px;width:260px;text-align:center;}"
  "#speed-label{font-size:14px;color:#aaa;margin-bottom:8px;}"
  "#speed-val{font-size:22px;font-weight:bold;color:#4CAF50;}"
  "input[type=range]{width:100%;accent-color:#4CAF50;"
  "-webkit-appearance:none;height:8px;border-radius:4px;"
  "background:#333;outline:none;margin-top:8px;}"
  "input[type=range]::-webkit-slider-thumb{"
  "-webkit-appearance:none;width:24px;height:24px;"
  "border-radius:50%;background:#4CAF50;cursor:pointer;}"

  "#status{font-size:13px;color:#888;margin-top:16px;}"
  "</style></head>"
  "<body>"
  "<h2>Rover Control</h2>"

  "<div class='row'>"
  "<button onmousedown=\"sendDir('F')\" onmouseup=\"send('S')\""
  "        ontouchstart=\"sendDir('F')\" ontouchend=\"send('S')\">&#9650;</button>"
  "</div>"
  "<div class='row'>"
  "<button onmousedown=\"sendDir('L')\" onmouseup=\"send('S')\""
  "        ontouchstart=\"sendDir('L')\" ontouchend=\"send('S')\">&#9664;</button>"
  "<button onmousedown=\"send('S')\" onmouseup=\"send('S')\""
  "        ontouchstart=\"send('S')\" ontouchend=\"send('S')\">&#9632;</button>"
  "<button onmousedown=\"sendDir('R')\" onmouseup=\"send('S')\""
  "        ontouchstart=\"sendDir('R')\" ontouchend=\"send('S')\">&#9654;</button>"
  "</div>"
  "<div class='row'>"
  "<button onmousedown=\"sendDir('B')\" onmouseup=\"send('S')\""
  "        ontouchstart=\"sendDir('B')\" ontouchend=\"send('S')\">&#9660;</button>"
  "</div>"

  // Speed slider — goes from 80 (min motors can spin) to 255 (full)
  "<div id='speed-wrap'>"
  "<div id='speed-label'>Speed: <span id='speed-val'>150</span></div>"
  "<input type='range' id='slider' min='80' max='255' value='150'"
  "  oninput=\"document.getElementById('speed-val').innerText=this.value\">"
  "</div>"

  "<p id='status'>Connecting...</p>"

  "<script>"
  "let ws;"
  "function connect(){"
  "  ws = new WebSocket('ws://'+location.host+'/ws');"
  "  ws.onopen = ()=>document.getElementById('status').innerText='Connected ✓';"
  "  ws.onclose = ()=>{"
  "    document.getElementById('status').innerText='Disconnected. Retrying...';"
  "    setTimeout(connect,2000);"
  "  };"
  "}"

  // send() for plain messages like 'S'
  "function send(msg){"
  "  if(ws && ws.readyState===WebSocket.OPEN) ws.send(msg);"
  "}"

  // sendDir() reads current slider value and appends it to direction
  "function sendDir(dir){"
  "  let spd = document.getElementById('slider').value;"
  "  send(dir + spd);"   // e.g. "F180"
  "}"

  "connect();"
  "</script>"
  "</body></html>";

// ─── Setup ─────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

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

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", htmlPage);
  });

  server.begin();
  Serial.println("Server started.");
}

// ─── Loop ──────────────────────────────────────────────────────

void loop() {
  ws.cleanupClients();
}
