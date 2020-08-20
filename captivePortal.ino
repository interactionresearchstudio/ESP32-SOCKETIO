class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      //we can handle anything!
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      Serial.print("handleRequest: ");
      Serial.println(request->url());

      if (request->method() == HTTP_GET) {
        if (request->url() == "/credentials") getCredentials(request);
        else if (request->url() == "/scan")   getScan(request);
        else if (SPIFFS.exists(request->url())) sendFile(request, request->url());
        else if (request->url().endsWith(".html") || request->url().endsWith("/") || request->url().endsWith("generate_204")) {
          sendFile(request, "/index.html");
        }
        else {
          request->send(404);
        }
      }
    }

    void handleBody(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      Serial.print("handleBody: ");
      Serial.println(request->url());

      if (request->method() == HTTP_POST) {
        if (request->url() == "/credentials") {
          String json = "";
          for (int i = 0; i < len; i++)  json += char(data[i]);

          StaticJsonDocument<1024> settingsJsonDoc;
          if (!deserializeJson(settingsJsonDoc, json)) {
            setCredentials(settingsJsonDoc.as<JsonObject>());
            request->send(200, "text/html", "<h1>Success! You can now disconnect from this network.</h1>");
          }
        }
        else {
          request->send(404);
        }
      }
    }

    void sendFile(AsyncWebServerRequest * request, String path) {
      Serial.println("handleFileRead: " + path);

      if (SPIFFS.exists(path)) {
        request->send(SPIFFS, path, getContentType(path));
      }
      else {
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

    void getCredentials(AsyncWebServerRequest *request) {
      Serial.println("getCredentials");
      AsyncResponseStream *response = request->beginResponseStream("application/json");

      StaticJsonDocument<1024> settingsJsonDoc;
      settingsJsonDoc["local_mac"] = myID;
      settingsJsonDoc["local_ssid"] = "";
      settingsJsonDoc["local_pass_len"] = 0; //local_pass.length;
      settingsJsonDoc["remote_ssid"] = "";
      settingsJsonDoc["remote_pass_len"] = 0; //remote_pass.length;
      settingsJsonDoc["remote_mac"] = getRemoteMacAddress(1);
      settingsJsonDoc["local_paired_status"] = getCurrentPairedStatusAsString();
      Serial.println(getCurrentPairedStatusAsString());

      String jsonString;
      serializeJson(settingsJsonDoc, jsonString);
      response->print(jsonString);

      request->send(response);
    }
    
    void setCredentials(JsonVariant json) {
      Serial.println("setCredentials");

      String local_ssid = json["local_ssid"].as<String>();
      String local_pass = json["local_pass"].as<String>();
      String remote_ssid = json["remote_ssid"].as<String>();
      String remote_pass = json["remote_pass"].as<String>();
      String remote_mac = json["remote_mac"].as<String>();
      
      if (remote_mac != "") addToMacAddressJSON(remote_mac);

      if (remote_pass != "" && remote_ssid != "" && local_ssid != "" && local_pass != "") {
        local_ssid = checkSsidForSpelling(local_ssid);
        remote_ssid = checkSsidForSpelling(remote_ssid);
        addToWiFiJSON(local_ssid, local_pass);
        addToWiFiJSON(remote_ssid, remote_pass);
        sendWifiCredentials();
        socket_server.textAll("RESTART");
        softReset();
      }
      else if (local_pass != "" && local_ssid != "" && remote_ssid == "" && remote_pass == "") {
        local_ssid = checkSsidForSpelling(local_ssid);
        addToWiFiJSON(local_ssid, local_pass);
        sendWifiCredentials();
        socket_server.textAll("RESTART");
        softReset();
      }
    }

    void getScan(AsyncWebServerRequest * request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");

      response->print(getScanAsJsonString());
      request->send(response);
    }
};

void setupCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", apIP);
}
