#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#define AP_SSID "Fingerprint_Control"
#define AP_PASS "12345678"

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

SoftwareSerial mySerial(D5, D6);  // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

bool busy = false;
uint8_t id;

bool savedIDs[128];
String lastStatus = "System ready";


#define relay_pin D7   // GPIO13
#define button_pin D2  // GPIO4

String portalPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Fingerprint Control</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body{
  margin:0;
  min-height:100vh;
  display:flex;
  flex-direction:column;
  justify-content:center;
  align-items:center;
  gap:30px;
  background:linear-gradient(135deg,#020617,#020617,#1e293b);
  font-family:system-ui;
  color:#e5e7eb;
}

/* ===== SCANNER ===== */
.scanner{
  width:220px;
  height:220px;
  border-radius:50%;
  position:relative;
  display:flex;
  justify-content:center;
  align-items:center;
}

.ring.outer{
  position:absolute;
  inset:0;
  border-radius:50%;
  border:2px dashed #22c55e;
  box-shadow:0 0 35px #22c55e66;
  animation:spin 10s linear infinite;
}

.ring.inner{
  position:absolute;
  inset:20px;
  border-radius:50%;
  border:2px solid #16a34a;
  box-shadow:inset 0 0 25px #22c55e55;
  animation:spin-rev 6s linear infinite;
}

.scan{
  position:absolute;
  width:100%;
  height:6px;
  background:linear-gradient(to right,transparent,#22c55e,transparent);
  animation:scan 2.2s linear infinite;
  box-shadow:0 0 15px #22c55e;
}

.core{
  width:90px;
  height:90px;
  background:rgba(34,197,94,0.05);
  clip-path:polygon(
    25% 0%,75% 0%,
    100% 50%,
    75% 100%,
    25% 100%,
    0% 50%
  );
  display:flex;
  justify-content:center;
  align-items:center;
  box-shadow:0 0 25px #22c55e88,inset 0 0 15px #22c55e55;
  animation:pulse 2s infinite;
}

.icon{
  font-size:40px;
  text-shadow:0 0 15px #22c55e;
}

.dot{
  position:absolute;
  width:4px;
  height:4px;
  background:#22c55e;
  border-radius:50%;
  box-shadow:0 0 10px #22c55e;
  animation:float 3s infinite ease-in-out;
}
.dot:nth-child(1){top:15%; left:60%}
.dot:nth-child(2){top:70%; left:20%; animation-delay:1s}
.dot:nth-child(3){top:40%; left:85%; animation-delay:2s}

@keyframes spin{to{transform:rotate(360deg)}}
@keyframes spin-rev{to{transform:rotate(-360deg)}}
@keyframes scan{0%{top:0}100%{top:100%}}
@keyframes pulse{50%{transform:scale(1.08)}}
@keyframes float{50%{transform:translateY(-10px);opacity:1}}
.card{
  width:440px;
  background:#020617;
  border-radius:18px;
  padding:26px;
  box-shadow:0 20px 40px rgba(0,0,0,.6);
}
h2{margin:0}
.sub{color:#94a3b8;font-size:13px;margin-bottom:20px}
input,button{
  width:100%;
  padding:14px;
  border-radius:10px;
  margin-bottom:12px;
  border:none;
  font-size:15px;
}
input{background:#020617;color:#fff;border:1px solid #1e293b}
button{font-weight:700;cursor:pointer}
.enroll{background:#22c55e;color:#022c22}
.delete{background:#f97316;color:#3b1a00}
.clear{background:#ef4444;color:#3b0202}
.status{
  padding:14px;
  border-radius:12px;
  border:1px solid #1e293b;
  margin-top:10px;
}
.ids{
  margin-top:16px;
  font-size:13px;
  color:#94a3b8;
}
.headerRow{
  display:flex;
  justify-content:space-between;
}
.count{
  background:#1e293b;
  padding:4px 10px;
  border-radius:999px;
}
.badge{
  display:inline-block;
  padding:4px 8px;
  background:#1e293b;
  border-radius:6px;
  margin:4px;
  font-size:12px;
}
.success{border-color:#22c55e;color:#22c55e}
.warn{border-color:#f97316;color:#f97316}
.error{border-color:#ef4444;color:#ef4444}
</style>
</head>

<body>
<div class="scanner">
  <div class="ring outer"></div>
  <div class="ring inner"></div>
  <div class="scan"></div>

  <div class="core">
    <div class="icon">🧬</div>
  </div>

  <div class="dot"></div>
  <div class="dot"></div>
  <div class="dot"></div>
</div>  
<div class="card">
<h2>Fingerprint Control</h2>
<div class="sub">Secure Access Panel</div>

<input id="eid" type="number" placeholder="Enroll ID (1–127)">
<button class="enroll" onclick="enroll()">Enroll Finger</button>

<input id="did" type="number" placeholder="Delete ID">
<button class="delete" onclick="delID()">Delete ID</button>

<button class="clear" onclick="clearAll()">Delete ALL</button>

<div id="status" class="status">System ready</div>

<div class="ids">
  <div class="headerRow">
    <b>Saved IDs</b>
    <span id="count" class="count">0</span>
  </div>
  <div id="idList"></div>
</div>
</div>

<script>
function enroll(){
 let id=eid.value;
 if(!id) return set("Enter ID","warn");
 fetch("/enroll?id="+id)
   .then(r=>r.text())
   .then(t=>{
     set(t,"warn");   // show ONLY what ESP reports
     eid.value="";
   });
}

function delID(){
 let id=did.value;
 if(!id) return set("Enter ID","warn");
 fetch("/delete?id="+id)
   .then(r=>r.text())
   .then(t=>{
     set(t,"warn");
     did.value="";
   });
}

function clearAll(){
 if(!confirm("Delete ALL fingerprints?"))return;
 fetch("/clear").then(r=>r.text()).then(t=>set(t,"error"));
}

function set(msg,type){
 let s=document.getElementById("status");
 s.className="status "+type;
 s.innerText=msg;
}

function refresh(){
 fetch("/list").then(r=>r.json()).then(j=>{
  idList.innerHTML="";
  count.innerText=j.length;
  j.forEach(i=>{
    let b=document.createElement("span");
    b.className="badge";
    b.innerText=i;
    idList.appendChild(b);
  });
 });
 fetch("/status").then(r=>r.text()).then(t=>{
   set(t,"success");
 });
}
setInterval(refresh,1000);
</script>
</body>
</html>
)rawliteral";
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(relay_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);

  // Turn relay OFF at startup
  digitalWrite(relay_pin, HIGH);

  mySerial.begin(57600);
  finger.begin(57600);
  if (!finger.verifyPassword())
    while (1)
      ;

  memset(savedIDs, false, sizeof(savedIDs));

  for (int i = 1; i < 128; i++) {
    if (finger.loadModel(i) == FINGERPRINT_OK) {
      savedIDs[i] = true;
    }
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASS);

  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/", []() {
    webServer.send(200, "text/html", portalPage());
  });
  webServer.on("/enroll", handleEnroll);
  webServer.on("/delete", handleDelete);
  webServer.on("/clear", handleClear);
  webServer.on("/list", handleList);
  webServer.on("/status", handleStatus);
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", portalPage());
  });

  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  if (!busy) verifyFinger();
}

void verifyFinger() {
  if (finger.getImage() != FINGERPRINT_OK) return;
  if (finger.image2Tz() != FINGERPRINT_OK) return;

  if (finger.fingerSearch() == FINGERPRINT_OK) {
    lastStatus = "Fingerprint matched → ID " + String(finger.fingerID);
    //blinkLED(2000);
    if (digitalRead(button_pin) == LOW) {

      digitalWrite(relay_pin, LOW);
      delay(60000);
      digitalWrite(relay_pin, HIGH);
      delay(800);
    }

    // Relay OFF
    else {

      digitalWrite(relay_pin, HIGH);
      delay(800);
    }
  } else {
    lastStatus = "No fingerprint match";
    delay(800);
  }
}

void blinkLED(uint16_t ms) {
  unsigned long t = millis();
  while (millis() - t < ms) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
  }
}

void handleEnroll() {
  busy = true;
  id = webServer.arg("id").toInt();
  webServer.send(200, "text/plain", "(Remove finger after 3 sec) Place finger for ID " + String(id));

  if (getFingerprintEnroll()) {
    savedIDs[id] = true;
    lastStatus = "Fingerprint SAVED for ID " + String(id);
  } else {
    lastStatus = "Enrollment FAILED for ID " + String(id);
  }
  busy = false;
}

void handleDelete() {
  busy = true;
  uint8_t d = webServer.arg("id").toInt();
  finger.deleteModel(d);
  savedIDs[d] = false;
  lastStatus = "ID " + String(d) + " deleted";
  webServer.send(200, "text/plain", lastStatus);
  busy = false;
}

void handleClear() {
  busy = true;
  finger.emptyDatabase();
  memset(savedIDs, false, sizeof(savedIDs));
  lastStatus = "All fingerprints deleted";
  webServer.send(200, "text/plain", lastStatus);
  busy = false;
}

void handleList() {
  String json = "[";
  bool first = true;
  for (int i = 1; i < 128; i++) {
    if (savedIDs[i]) {
      if (!first) json += ",";
      json += String(i);
      first = false;
    }
  }
  json += "]";
  webServer.send(200, "application/json", json);
}

void handleStatus() {
  webServer.send(200, "text/plain", lastStatus);
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    delay(100);
  }
  finger.image2Tz(1);
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER)
    ;
  p = -1;
  while (p != FINGERPRINT_OK) { p = finger.getImage(); }
  finger.image2Tz(2);
  if (finger.createModel() != FINGERPRINT_OK) return false;
  return finger.storeModel(id) == FINGERPRINT_OK;
}
