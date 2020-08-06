#define ROUTER_SSID "XXXX"
#define ROUTER_PASS "XXXX"

//#define HARDCODE_MAC
#define STAGING

#define VERSION "v0.1"
#define ESP32

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//socketIO
#include <SocketIoClient.h>
/* HAS TO BE INITIALISED BEFORE WEBSOCKETSCLIENT LIB */
SocketIoClient socketIO;

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
String scads_ssid = "";
String scads_pass = "Password";

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

Preferences preferences;

//local server
AsyncWebServer server(80);
AsyncWebSocket socket_server("/ws");

//local websockets client
WebSocketsClient socket_client;


WiFiMulti wifiMulti;

bool isClient = false;
String remote_macAddress = "";
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
  remote_macAddress = "TE:ST:TE:ST:TE:ST";
#else if
  //Check if device already has a pair macaddress
  preferences.begin("scads", false);
  remote_macAddress = preferences.getString("mac", "");
  Serial.println(remote_macAddress);
  preferences.end();
#endif

  if (remote_macAddress == "") {
    Serial.println("Scanning for available SCADS");
    scanningForSCADS();
    if (isClient == false) {
      Serial.println("No available SCAD Network, creating AP and server");
      createSCADSAP();
      setupCaptivePortal();
      setupLocalServer();
    } else {
      //become client
      socket_client.begin("192.168.4.1", 80, "/ws");
      socket_client.onEvent(webSocketEvent);
      socket_client.setReconnectInterval(5000);
    }
  } else {
    Serial.print("connected to:");
    Serial.println(remote_macAddress);
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
    socketIO.on("connect", socketIO_Connected);
    socketIO.on("event", socketIO_event);
    socketIO.on("send mac", socketIO_sendMac);
    socketIO.on("msg", socketIO_msg);
    socketIO.begin(host, port, path);

    setupFinished = true;
  }

}

void loop() {

  if (isClient == false) {
    socket_server.cleanupClients();
    dnsServer.processNextRequest();
  } else if (isClient == true && setupFinished == false) {
    socket_client.loop();
  }
  if (setupFinished == true) {
    socketIO.loop();
    checkLEDState();
  }
}

void scanningForSCADS() {
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
      scads_ssid = WiFi.SSID(i);
      if (scads_ssid.indexOf("SCADS-") > -1) {
        Serial.println("Found SCAD");
        isClient = true;
        wifiMulti.addAP(scads_ssid.c_str(), scads_pass.c_str());
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

void createSCADSAP() {
  //Creates Access Point for other device to connect to
  scads_ssid = "SCADS-" + String((unsigned long)ESP.getEfuseMac(), DEC);
  Serial.print("Wifi name:");
  Serial.println(scads_ssid);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(scads_ssid.c_str(), scads_pass.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);
}

void checkLEDState() {
  digitalWrite(LEDPin, LEDState);
  const bool newState = digitalRead(buttonPin); // See if button is physically pushed
  if (!newState) {
    socketIO_sendButtonPress();
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
