#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

#define MAJU_PIN  23
#define ATAS_PIN  22
#define KIRI_PIN  19
#define CAPIT_PIN 18

const char* AP_SSID = "ServoControl";
const char* AP_PASS = "12345678";

Servo myServo1;
Servo myServo2;
Servo myServo3;
Servo myServo4;

WebServer server(80);

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Servo Control</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: Arial, sans-serif; background: #1a1a2e; color: #eee; display: flex; flex-direction: column; align-items: center; padding: 30px 16px; min-height: 100vh; }
  h1 { margin-bottom: 30px; font-size: 1.6rem; color: #e94560; letter-spacing: 1px; }
  .card { background: #16213e; border-radius: 12px; padding: 24px; width: 100%; max-width: 420px; margin-bottom: 16px; box-shadow: 0 4px 20px rgba(0,0,0,0.4); }
  .label { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; font-size: 0.95rem; }
  .servo-name { font-weight: bold; color: #e94560; }
  .angle-val { font-size: 1.1rem; font-weight: bold; min-width: 42px; text-align: right; }
  input[type=range] { -webkit-appearance: none; width: 100%; height: 8px; border-radius: 4px; background: #0f3460; outline: none; }
  input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 22px; height: 22px; border-radius: 50%; background: #e94560; cursor: pointer; }
  .status { margin-top: 20px; font-size: 0.85rem; color: #888; text-align: center; }
  .dot { display: inline-block; width: 8px; height: 8px; border-radius: 50%; background: #2ecc71; margin-right: 6px; }
</style>
</head>
<body>
<h1>Servo Control Panel</h1>

<div class="card">
  <div class="label"><span class="servo-name">Servo 1 - Maju</span><span class="angle-val" id="v1">90</span>&deg;</div>
  <input type="range" min="0" max="180" value="90" oninput="send(1,this.value)" id="s1">
</div>
<div class="card">
  <div class="label"><span class="servo-name">Servo 2 - Atas</span><span class="angle-val" id="v2">90</span>&deg;</div>
  <input type="range" min="0" max="180" value="90" oninput="send(2,this.value)" id="s2">
</div>
<div class="card">
  <div class="label"><span class="servo-name">Servo 3 - Kiri</span><span class="angle-val" id="v3">90</span>&deg;</div>
  <input type="range" min="0" max="180" value="90" oninput="send(3,this.value)" id="s3">
</div>
<div class="card">
  <div class="label"><span class="servo-name">Servo 4 - Capit</span><span class="angle-val" id="v4">90</span>&deg;</div>
  <input type="range" min="0" max="180" value="90" oninput="send(4,this.value)" id="s4">
</div>

<div class="status"><span class="dot"></span>Terhubung ke ESP32 AP</div>

<script>
  function send(id, val) {
    document.getElementById('v' + id).textContent = val;
    fetch('/set?servo=' + id + '&angle=' + val).catch(() => {});
  }
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleSet() {
  if (server.hasArg("servo") && server.hasArg("angle")) {
    int id  = server.arg("servo").toInt();
    int ang = constrain(server.arg("angle").toInt(), 0, 180);
    switch (id) {
      case 1: 
        myServo1.write(ang); 
        break;
      case 2: 
        myServo2.write(ang); 
        break;
      case 3: 
        myServo3.write(ang); 
        break;
      case 4: 
        myServo4.write(ang); 
        break;
    }
    Serial.printf("Servo %d -> %d deg\n", id, ang);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("HTTP server started");

  ESP32PWM::allocateTimer(0);

  myServo1.setPeriodHertz(50);
  myServo2.setPeriodHertz(50);
  myServo3.setPeriodHertz(50);
  myServo4.setPeriodHertz(50);
  myServo1.attach(MAJU_PIN);
  myServo2.attach(ATAS_PIN);
  myServo3.attach(KIRI_PIN);
  myServo4.attach(CAPIT_PIN);

  // posisi awal tengah
  myServo1.write(90);
  myServo2.write(90);
  myServo3.write(90);
  myServo4.write(90);
}

void loop() {
  server.handleClient();
}
