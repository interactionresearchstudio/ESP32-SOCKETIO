String remote_ssid = "";
String remote_pass = "";
String local_ssid = "";
String local_pass = "";
String remote_mac = "";

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    const char *htmlStart PROGMEM = "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"> <head> <meta charset=\"utf-8\"> <title>SCADS</title> </head> <body> <h1>Setup</h1> <h2 id=\"result\"></h2> <form action=\"/credentials\">";
    
    const char *macFormStart PROGMEM = "<fieldset id=\"mac_address\"> <p>Your MAC address: <span id=\"local_mac\">";
    const char *macFormEnd PROGMEM = "</span></p><label for=\"remote_mac\">Remote MAC address</label><br><input type=\"text\" id=\"remote_mac\" name=\"remote_mac\"><br></fieldset>";
    const char *localWifiForm PROGMEM = "<fieldset id=\"local_wifi\"> <label for=\"local_ssid\">Your WiFi name</label><br><input type=\"text\" id=\"local_ssid\" name=\"local_ssid\"><br><label for=\"local_pass\">Your WiFi password</label><br><input type=\"password\" id=\"local_pass\" name=\"local_pass\"><br></fieldset>";
    const char *remoteWifiForm PROGMEM = "<fieldset id=\"remote_wifi\"> <label for=\"remote_ssid\">Remote WiFi name</label><br><input type=\"text\" id=\"remote_ssid\" name=\"remote_ssid\"><br><label for=\"remote_pass\">Remote WiFi password</label><br><input type=\"password\" id=\"remote_pass\" name=\"remote_pass\"><br></fieldset>";
    
    const char *htmlEnd PROGMEM = "<input type=\"submit\" name=\"submit\" value=\"submit\"></form></body></html>";
    
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      Serial.print("Requested in captive ");
      Serial.print(request->url());
      Serial.print(" type: ");
      Serial.println(request->method());
      
      if (request->url() == "/credentials") {
        setCredentials(request);
      }
      else if (request->url() == "/scan") {
        getScan(request);
      }
      else if (request->url() == "/id") {
        getID(request);
      }
      else if(SPIFFS.exists(request->url())) {
        sendFile(request, request->url());
      }
      else if(request->url().endsWith(".html") || request->url().endsWith("/")) {
        //renderPortal(request);
        sendFile(request, "/index.html");
      }
      else {
        request->send(404);
      }
    }

    void sendFile(AsyncWebServerRequest *request, String path) {
      Serial.println("handleFileRead: " + path);
    
      if (SPIFFS.exists(path)) {
        request->send(SPIFFS, path, getContentType(path));
      }
      else{
        request->send(404);
      }
    }

    String getContentType(String filename) {
      if (filename.endsWith(".htm")) return "text/html";
      else if (filename.endsWith(".html")) return "text/html";
      else if (filename.endsWith(".css")) return "text/css";
      else if (filename.endsWith(".js")) return "application/javascript";
      else if (filename.endsWith(".png")) return "image/png";
      else if (filename.endsWith(".gif")) return "image/gif";
      else if (filename.endsWith(".jpg")) return "image/jpeg";
      else if (filename.endsWith(".ico")) return "image/x-icon";
      else if (filename.endsWith(".xml")) return "text/xml";
      else if (filename.endsWith(".pdf")) return "application/x-pdf";
      else if (filename.endsWith(".zip")) return "application/x-zip";
      else if (filename.endsWith(".gz")) return "application/x-gzip";
      else if (filename.endsWith(".json")) return "application/json";
      return "text/plain";
    }

    void setCredentials(AsyncWebServerRequest *request) {
      Serial.println("Received credentials");
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isFile()) {
          Serial.printf("FILE[%s]: %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
        } else if (p->isPost()) {
          Serial.printf("POST[%s]: %s", p->name().c_str(), p->value().c_str());
        } else {
          Serial.printf("GET[%s]: %s", p->name().c_str(), p->value().c_str());
          Serial.println();
          String in = (String)p->name();
          Serial.println(in);
          if (in == "local_ssid") {
            Serial.println("got local ssid");
            local_ssid = (String)p->value();
          } else if (in ==  "local_pass") {
            Serial.println("got local pass");
            local_pass = (String)p->value();
          } else if (in == "remote_ssid") {
            Serial.println("got remote ssid");
            remote_ssid = (String)p->value();
          } else if (in == "remote_pass") {
            Serial.println("got remote pass");
            remote_pass = (String)p->value();
          } else if (in == "remote_mac") {
            Serial.println("got remote mac");
            remote_mac = (String)p->value();
          }
        }
        Serial.println();
      }
      if (remote_mac != "") {
        remote_mac.toUpperCase();
        addToMacAddressJSON(remote_mac);
      }
      if (remote_pass != "" && remote_ssid != "" && local_ssid != "" && local_pass != "") {
        local_ssid = checkSsidForSpelling(local_ssid);
        remote_ssid = checkSsidForSpelling(remote_ssid);
        addToWiFiJSON(local_ssid, local_pass);
        addToWiFiJSON(remote_ssid, remote_pass);
        sendWifiCredentials();
        socket_server.textAll("RESTART");
        softReset();
      } else if (local_pass != "" && local_ssid != "") {
        local_ssid = checkSsidForSpelling(local_ssid);
        addToWiFiJSON(local_ssid, local_pass);
        sendWifiCredentials();
        socket_server.textAll("RESTART");
        softReset();
      }
      request->send(200, "text/html", "<h1>Success! You can now disconnect from this network.</h1>");
    }

    void getScan(AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");

      response->print(getScanAsJsonString());
      request->send(response);
    }

    void getID(AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("application/json");

      response->print("{id:" + myID + "}");
      request->send(response);
    }

    void renderPortal(AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      response->print(htmlStart);
      
      switch (currentPairedStatus) {
        case unpaired:
          Serial.println("Sending form for unpaired...");
          response->print(localWifiForm);
          response->print(macFormStart);
          response->print(myID);
          response->print(macFormEnd);
          break;
        case pairing:
          Serial.println("Sending form for pairing...");
          response->print(localWifiForm);
          response->print(remoteWifiForm);
          break;
        case paired:
          Serial.println("Sending form for paired...");
          response->print(localWifiForm);
          break;
      }
      response->print(htmlEnd);
      request->send(response);
    }
};

void setupCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", apIP);
}
