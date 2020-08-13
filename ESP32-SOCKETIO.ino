#define EXTERNAL_BUTTON 23
#define EXTERNAL_LED1 21
#define EXTERNAL_LED2 19
#define EXTERNAL_LED3 18

bool led2Toggle = true;

#define LED3TIMEON 30000
long led3PrevTime;
bool led3IsPressed = false;


int connection = 0;

#define VERSION "v0.2"

#define ESP32set

#define WIFICONNECTTIMEOUT 60000

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
String scads_pass = "blinkblink";

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

Preferences preferences;

//local server
AsyncWebServer server(80);
AsyncWebSocket socket_server("/ws");

//local websockets client
WebSocketsClient socket_client;
byte webSocketClientID;


WiFiMulti wifiMulti;

//state handling variables
bool isClient = false;
String wifiCredentials = "";
String macCredentials = "";
bool hasPairedMac = false;
bool setupFinished = false;


/// Led Settings ///
int onBoardLed = 2;
bool isBlinking = false;
bool readyToBlink = false;
unsigned long blinkTime;
int blinkDuration = 200;

//Button Settings
int buttonPin = 0;
bool buttonDebounce;
unsigned long buttonPressTime;

//reset timers
bool isResetting = false;
unsigned long resetTime;
int resetLength = 4000;

String myID = "";

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
  setupPins();

  //create 10 digit ID
  myID = generateID();

  //Check if device already has a pair macaddress
  preferences.begin("scads", false);
  wifiCredentials = preferences.getString("wifi", "");
  macCredentials = preferences.getString("mac", "");
  preferences.end();
  Serial.println("Stored wifi and mac addresses");
  Serial.println(macCredentials);
  Serial.println(wifiCredentials);
  if (macCredentials != "") {
    if (getMacJSONSize() < 2) {
      //check it has a paired mac address
      hasPairedMac = false;
      Serial.println("Already have local mac address in preferences, but nothing else");
    } else {
      hasPairedMac = true;
      connection = 2;
      Serial.println("Already has paired mac address");
    }
  } else {
    hasPairedMac = false;
    Serial.println("setting up JSON database for mac addresses");
    preferences.clear();
    addToMacAddressJSON(myID);
  }
  if (hasPairedMac == false || wifiCredentials == "") {
    Serial.println("Scanning for available SCADS");
    scanningForSCADS();
    if (isClient == false) {
      //become server
      createSCADSAP();
      setupCaptivePortal();
      setupLocalServer();
    } else {
      //become client
      setupSocketClientEvents();
    }
  } else {
    Serial.print("List of Mac addresses:");
    Serial.println(macCredentials);
    //connect to router to talk to server
    if (wifiCredentials != "") {
      connectToWifi(wifiCredentials);
      setupSocketIOEvents();
      setupFinished = true;
      Serial.println("setup complete");
    } else {
      Serial.println("wifi no credentials");
    }
  }
}

void loop() {

  if (isClient == false) {
    //Local Server Mode
    socket_server.cleanupClients();
    dnsServer.processNextRequest();
    checkReset();
  }
  if (isClient == true && setupFinished == false) {
    //Local Client Mode
    socket_client.loop();
    checkReset();
  } else if (setupFinished == true) {
    //Connected Mode
    socketIO.loop();
    buttonHandler();
    ledHandler();
    led3Handler();
  }
}
