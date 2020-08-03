//#define HARDCODE_MAC
#define STAGING

#define VERSION "v0.1"
#define ESP32
#define ROUTER_SSID "XXXX"
#define ROUTER_PASS "XXXX"

#include <SocketIoClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>


//Access Point credentials
String SCAD_SSID = "";
String SCAD_PASS = "Password";

bool isClient = false;
String mac_address = "";

int led = 2;
bool setupFinished = false;
unsigned long prevMillis;


/// Pin Settings ///
int LEDPin = 2;
int buttonPin = 0;
bool LEDState = false;

/// Socket.IO Settings ///
#ifndef STAGING 
char host[] = "irs-socket-server.herokuapp.com"; // Socket.IO Server Address
#else
char host[] = "irs-socket-server-staging.herokuapp.com"; // Socket.IO Staging Server Address
#endif
int port = 80; // Socket.IO Port Address
char path[] = "/socket.io/?transport=websocket"; // Socket.IO Base Path


SocketIoClient webSocket;
WiFiMulti wifiMulti;
Preferences preferences;
WebServer server(80);


void setup() {
  Serial.begin(115200);

  pinMode(LEDPin, OUTPUT);
  pinMode(buttonPin, INPUT);


#ifdef HARDCODE_MAC
  mac_address = "TE:ST:TE:ST:TE:ST";
#else if
  //Check if device already has a pair macaddress
  preferences.begin("scads", false);
  mac_address = preferences.getString("user_mac", "");
  preferences.end();
#endif

  if (mac_address == "") {
    Serial.println("Scanning for available SCADS");
    scanningForSCAD();
    if (isClient == false) {
      Serial.println("No available SCAD Network, creating AP and server");
      createSCADAP();
    } else {
      //become client
      postDataToServer();
    }
  } else {
    Serial.print("connected to:");
    Serial.println(mac_address);
    //connect to router to talk to server
    Serial.println("Connecting to Router");
    wifiMulti.addAP(ROUTER_SSID, ROUTER_PASS);
    while ((wifiMulti.run() != WL_CONNECTED)) {
      delay(100);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    checkForUpdate();

    // Setup 'on' listen events
    webSocket.on("connect", socket_Connected);
    webSocket.on("event", socket_event);
    webSocket.on("send mac", socket_sendMac);
    webSocket.on("msg", socket_msg);
    webSocket.begin(host, port, path);

    setupFinished = true;
  }

}

void loop() {

  if (isClient == false) {
    server.handleClient();
  }
  if (setupFinished == true) {
    webSocket.loop();
    checkLEDState();
  }
}

void scanningForSCAD() {
  // WiFi.scanNetworks will return the number of networks found
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
      SCAD_SSID = WiFi.SSID(i);
      if (SCAD_SSID.indexOf("SCAD-") > -1) {
        Serial.println("Found SCAD");
        isClient = true;
        wifiMulti.addAP(SCAD_SSID.c_str(), SCAD_PASS.c_str());
        while ((wifiMulti.run() != WL_CONNECTED)) {
          delay(500);
          Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
      }
    }
  }
}

void createSCADAP() {
  //Creates Access Point for other device to connect to
  SCAD_SSID = "SCAD-" + String((unsigned long)ESP.getEfuseMac(), DEC);
  Serial.print("Wifi name:");
  Serial.println(SCAD_SSID);
  WiFi.softAP(SCAD_SSID.c_str(), SCAD_PASS.c_str());
  IPAddress myIP = WiFi.softAPIP();
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/MAC/", HTTP_POST, handleMac);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void handleMac() {
  //Receives client devices MAC and stores to preferences, then returns own MAC
  const size_t capacity = JSON_OBJECT_SIZE(1) + 38;
  DynamicJsonDocument doc(capacity);
  String json = server.arg("plain");
  deserializeJson(doc, json);
  String MAC = doc["MAC"];
  Serial.println(MAC);

  //save to preferences
  saveMac(MAC);
  DynamicJsonDocument returnDoc(capacity);
  returnDoc["MAC"] = WiFi.macAddress();
  String requestBody;
  serializeJson(returnDoc, requestBody);
  Serial.println(requestBody);
  server.send(200, "application/json", requestBody);

  blinkForever();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void postDataToServer() {
  //Sends Server device MAC address and receives Server device MAC address in return, then saves to preferences.
  HTTPClient http;
  if (wifiMulti.run() == WL_CONNECTED) {
    http.begin("http://192.168.4.1/MAC/");
    http.addHeader("Content-Type", "application/json");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 38;
    DynamicJsonDocument retDoc(capacity);
    retDoc["MAC"] = WiFi.macAddress();
    String requestBody;
    serializeJson(retDoc, requestBody);
    Serial.println(requestBody);
    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      //Save to preferences
      //deserialise
      DynamicJsonDocument responseDoc(capacity);
      String json = response;
      deserializeJson(responseDoc, json);
      String MAC = responseDoc["MAC"];
      saveMac(MAC);
      blinkForever();
    } else {
      Serial.println("error");
    }
  }
}

void saveMac(String mac) {
  preferences.begin("scads", false);
  preferences.putString("user_mac", mac);
  Serial.println("Saved mac to nvm");
  preferences.end();
}

void blinkForever() {
  while (1) {
    digitalWrite(LEDPin, 1);
    delay(500);
    digitalWrite(LEDPin, 0);
    delay(500);
  }
}

void socket_Connected(const char * payload, size_t length) {
  Serial.println("Socket.IO Connected!");
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, 1);
  delay(100);
  digitalWrite(LEDPin, 0);
  delay(100);
}

void socket_sendMac(const char * payload, size_t length) {
  Serial.println("GOT MAC REQUEST");
  const size_t capacity = JSON_OBJECT_SIZE(1) + 50;
  DynamicJsonDocument doc(capacity);
  doc["macAddress"] = WiFi.macAddress();
  String bodyReq;
  serializeJson(doc, bodyReq);
  Serial.println(bodyReq);
  webSocket.emit("mac", bodyReq.c_str());
}

void socket_event(const char * payload, size_t length) {
  Serial.print("got message: ");
  Serial.println(payload);
}

void socket_msg(const char * payload, size_t length) {
  Serial.println("got msg");
  const size_t capacity = JSON_OBJECT_SIZE(2) + 50;
  DynamicJsonDocument incomingDoc(capacity);
  deserializeJson(incomingDoc, payload);
  const char* recMacAddress = incomingDoc["macAddress"];
  const char* recData = incomingDoc["data"];
  Serial.print("I got a message from ");
  Serial.println(recMacAddress);
  Serial.print("Which is ");
  Serial.println(recData);
  String testt = String(recData);
  if (testt.indexOf("hello") > -1) {
    LEDState = !LEDState;
  }

}

void checkLEDState() {
  digitalWrite(LEDPin, LEDState);
  const bool newState = digitalRead(buttonPin); // See if button is physically pushed
  if (!newState) {
    const size_t capacity = JSON_OBJECT_SIZE(2) + 50;
    DynamicJsonDocument doc(capacity);
    doc["macAddress"] = mac_address;
    doc["data"] = "hello";
    String sender;
    serializeJson(doc, sender);
    webSocket.emit("msg", sender.c_str());
    delay(500);
  }
}


void checkForUpdate() {
  WiFiClient client;

  httpUpdate.setLedPin(LEDPin, LOW);
  String updateHost = "http://" + (String)host + "/update";
  t_httpUpdate_return ret = httpUpdate.update(client, updateHost, VERSION);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("HTTP_UPDATE_FAILED Error");
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}
