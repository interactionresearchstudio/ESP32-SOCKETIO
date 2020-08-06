#define ROUTER_SSID "XXXX"
#define ROUTER_PASS "XXXX"

//#define HARDCODE_MAC
#define STAGING

#define VERSION "v0.1"
#define ESP32

#define DBG_OUTPUT_PORT Serial

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//socketIO 
#include <SocketIoClient.h>
/* HAS TO BE INITIALISED BEFORE WEBSOCKETSCLIENT LIB */
SocketIoClient webSocket;

//Local Websockets
#include <WebSocketsClient.h>


#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiAP.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "SPIFFS.h"


//Access Point credentials
String SCAD_SSID = "";
String SCAD_PASS = "Password";

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

Preferences preferences;

//local websockets server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//local websockets client
WebSocketsClient localSocket;


WiFiMulti wifiMulti;

bool isClient = false;
String mac_address = "";
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
      setupCaptivePortal();
      setupLocalServer();
    } else {
      //become client
      localSocket.begin("192.168.4.1", 80, "/ws");
      localSocket.onEvent(webSocketEvent);
      localSocket.setReconnectInterval(5000);
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

    //checkForUpdate();

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
    ws.cleanupClients();
    dnsServer.processNextRequest();
  } else if (isClient == true && setupFinished == false) {
    localSocket.loop();
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
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SCAD_SSID.c_str(), SCAD_PASS.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);
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

void checkLEDState() {
  digitalWrite(LEDPin, LEDState);
  const bool newState = digitalRead(buttonPin); // See if button is physically pushed
  if (!newState) {
    Serial.println("button send");
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
