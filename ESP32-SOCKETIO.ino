#define ROUTER_SSID "XXXX"
#define ROUTER_PASS "XXXX"

//#define HARDCODE_MAC
#define STAGING

#define VERSION "v0.1"
#define ESP32set

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
bool isBlinking = false;
bool readyToBlink = false;
unsigned long blinkTime;
int blinkDuration = 200;

bool buttonDebounce;
unsigned long buttonPressTime;

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
      setupSocketClientEvents();
    }
  } else {
    Serial.print("connected to:");
    Serial.println(remote_macAddress);
    //connect to router to talk to server
    connectToWifi();
    //checkForUpdate();
    setupSocketIOEvents();
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
    testButtonHandler();
    ledHandler();
  }
}

void blinkDevice() {
  if (readyToBlink == false) {
    readyToBlink = true;
  }
}

void ledHandler() {
  if (readyToBlink == true && isBlinking == false) {
    isBlinking = true;
    blinkTime = millis();
    digitalWrite(LEDPin, 1);
  }
  if (millis() - blinkTime > blinkDuration && isBlinking == true) {
    digitalWrite(LEDPin, 0);
    isBlinking = false;
    readyToBlink = false;
  }
}

void testButtonHandler() {
  const bool buttonState = digitalRead(buttonPin);
  if (!buttonState && buttonDebounce == false) {
    buttonPressTime = millis();
    buttonDebounce = true;
    socketIO_sendButtonPress();
  }
  if (buttonState && buttonDebounce == true && millis() - buttonPressTime > 500) {
    buttonDebounce = false;
  }
}
