String remote_ssid = "";
String remote_pass = "";
String local_ssid = "";
String local_pass = "";
String remote_mac = "";

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    const char *htmlStart PROGMEM = "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"> <head> <meta charset=\"utf-8\"> <title>SCADS</title> </head> <body> <h1>Setup</h1> <h2 id=\"result\"></h2> <form action=\"/credentials\">";
    const char *htmlEnd PROGMEM = "<input type=\"submit\" name=\"submit\" value=\"submit\"></form></body></html>";
    const char *macFormStart PROGMEM = "<fieldset id=\"mac_address\"> <p>Your MAC address: <span id=\"local_mac\">";
    const char *macFormEnd PROGMEM = "</span></p><label for=\"remote_mac\">Remote MAC address</label><br><input type=\"text\" id=\"remote_mac\" name=\"remote_mac\"><br></fieldset>";
    const char *localWifiForm PROGMEM = "<fieldset id=\"local_wifi\"> <label for=\"local_ssid\">Your WiFi name</label><br><input type=\"text\" id=\"local_ssid\" name=\"local_ssid\"><br><label for=\"local_pass\">Your WiFi password</label><br><input type=\"password\" id=\"local_pass\" name=\"local_pass\"><br></fieldset>";
    const char *remoteWifiForm PROGMEM = "<fieldset id=\"remote_wifi\"> <label for=\"remote_ssid\">Remote WiFi name</label><br><input type=\"text\" id=\"remote_ssid\" name=\"remote_ssid\"><br><label for=\"remote_pass\">Remote WiFi password</label><br><input type=\"password\" id=\"remote_pass\" name=\"remote_pass\"><br></fieldset>";
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
          //remote_mac.toUpperCase(); /* Not needed */
          addToMacAddressJSON(remote_mac);
        }
        if (remote_pass != "" && remote_ssid != "" && local_ssid != "" && local_pass != "") {
          addToWiFiJSON(local_ssid, local_pass);
          addToWiFiJSON(remote_ssid, remote_pass);
          sendWifiCredentials();
          socket_server.textAll("RESTART");
          softReset();
        } else if (local_pass != "" && local_ssid != "") {
          addToWiFiJSON(local_ssid, local_pass);
          sendWifiCredentials();
          socket_server.textAll("RESTART");
          softReset();
        }

        request->send(200, "text/html", "<h1>Success! You can now disconnect from this network.</h1>");
      }
      else {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(htmlStart);
        switch (getDeviceStatus()) {
          case 0:
            Serial.println("Sending form for detached...");
            response->print(localWifiForm);
            response->print(macFormStart);
            response->print(generateID());
            response->print(macFormEnd);
            break;
          case 1:
            Serial.println("Sending form for pairing...");
            response->print(localWifiForm);
            response->print(remoteWifiForm);
            break;
          case 2:
            Serial.println("Sending form for paired...");
            response->print(localWifiForm);
            break;
        }
        response->print(htmlEnd);
        request->send(response);
      }
    }
};

int getDeviceStatus() {
  /*
     0: detached
     1: pairing
     2: paired
  */
  return connection;
}

void setupCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", apIP);
}
