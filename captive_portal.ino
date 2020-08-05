/*
void captiveRoutes() {
  server.on("/settings", HTTP_GET, []() {
    server.send(200, "application/json", getSettingsAsJson());
  });

  server.on("/settings", HTTP_POST, []() {
    server.send(200, "application/json", setSettings());
  });

  server.on("/reset", HTTP_POST, []() {
    server.send(200, "application/json", clearSettings());
  });

  server.on("/scan", HTTP_GET, []() {
    server.send(200, "application/json", getScan());
  });

  //These prove that you have an operational internet connection - otherwise wouldn't try to resolve external servers etc
  server.on("/success.txt", HTTP_GET, []() {
    server.send(200, "text/plain", "fail");
  });

  server.on("/hotspot-detect.html", HTTP_GET, []() {
    streamFile("/index.html");
  });

  server.on("/generate_204", HTTP_GET, []() {
    streamFile("/index.html");
  });

  server.onNotFound([]() {
    String path = getPath();
    Serial.println(path);
    if (!streamFile(path)) {
      if (getContentType(path) == "text/html") {
        streamFile("/index.html");
      }
      else {
        server.send(404);
      }
    }
  });
}

String setSettings() {
  if (server.args() == 1) {
    StaticJsonDocument<1024> settingsJsonDoc;
    StaticJsonDocument<200> incomingJsonDoc;

    if (!deserializeJson(settingsJsonDoc, getSettingsAsJson())) {
      Serial.println(server.arg(0));
      if (!deserializeJson(incomingJsonDoc, server.arg(0))) {
        JsonObject settingsJsonObject = settingsJsonDoc.as<JsonObject>();
        JsonObject incomingJsonObject = incomingJsonDoc.as<JsonObject>();

        jsonDeepMerge(settingsJsonObject, incomingJsonObject);

        String settingsJsonAsString;
        serializeJson(settingsJsonObject, settingsJsonAsString);
        setSettings(settingsJsonAsString);
      }
    }
  }

  return (getSettingsAsJson());
}

void setSettings(String json) {
  preferences.begin("scads", false);
  preferences.putString("json", json);
  preferences.end();
}

String clearSettings() {
  setSettings("{}");

  return (getSettingsAsJson());
}

void jsonDeepMerge(JsonVariant dst, JsonVariantConst src) {
  if (src.is<JsonObject>()) {
    for (auto kvp : src.as<JsonObject>()) {
      jsonDeepMerge(dst.getOrAddMember(kvp.key()), kvp.value());
    }
  }
  else {
    dst.set(src);
  }
}

String getSettingsAsJson() {
  preferences.begin("scads", false);
  String settingsJsonAsString = preferences.getString("json", "{}");
  preferences.end();

  return (settingsJsonAsString);
}

void joinKnownNetworks() {
  JsonObject knownNetworks;
  StaticJsonDocument<1024> settingsJsonDoc;
  if (!deserializeJson(settingsJsonDoc, getSettingsAsJson())) {
    knownNetworks = settingsJsonDoc["network"];
    //TODO: in the end this will be an array of known networks...
    const char* ssid = knownNetworks["ssid"];
    const char* password = knownNetworks["password"];
    Serial.print(ssid);
    Serial.print('\t');
    Serial.println(password);
  }
}

String getScan() {
  String jsonString;

  StaticJsonDocument<1000> jsonDoc;
  getScanAsJson(jsonDoc);
  serializeJson(jsonDoc[0], jsonString);

  return (jsonString);
}

void getScanAsJson(JsonDocument& jsonDoc) {
  JsonArray networks = jsonDoc.createNestedArray();

  int n = WiFi.scanNetworks();
  n = (n > MAX_NETWORKS_TO_SCAN) ? MAX_NETWORKS_TO_SCAN : n;

  //Array is ordered by signal strength - strongest first
  for (int i = 0; i < n; ++i) {
    JsonObject network  = networks.createNestedObject();
    network["SSID"] = WiFi.SSID(i);
    network["BSSID"] = WiFi.BSSIDstr(i);
    network["RSSI"] = WiFi.RSSI(i);
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
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
  else if (filename.endsWith(".json")) return "";
  return "text/plain";
}

String getPath() {
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";

  return (path);
}

bool streamFile(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);

  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;
}

String getLocalAddress() {
  return (WiFi.macAddress());
}
*/
