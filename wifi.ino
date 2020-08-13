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

void connectToWifi(String credentials) {

  String _wifiCredentials = credentials;
  const size_t capacity = 2 * JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 150;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, _wifiCredentials);
  JsonArray ssid = doc["ssid"];
  JsonArray pass = doc["password"];
  if (ssid.size() > 0) {
    for (int i = 0; i < ssid.size(); i++) {
      wifiMulti.addAP(checkSsidForSpelling(ssid[i]).c_str(), pass[i]);
    }
  } else {
    Serial.println("issue with wifi credentials, creating access point");
  }

  Serial.println("Connecting to Router");

  long wifiMillis = millis();
  while ((wifiMulti.run() != WL_CONNECTED)) {
    if (millis() - wifiMillis > WIFICONNECTTIMEOUT) {
      Serial.println("Wifi connect failed, Please try your details again in the captive portal");
      preferences.begin("scads", false);
      preferences.putString("wifi", "");
      preferences.end();
      ESP.restart();
    }
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String checkSsidForSpelling(String incomingSSID) {
  int n = WiFi.scanNetworks();
  int currMatch = 255;
  int prevMatch = currMatch;
  int matchID;
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
    Serial.println("can't find any wifi in the area");
    return incomingSSID;
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.println(WiFi.SSID(i));
      currMatch = levenshteinIgnoreCase(incomingSSID.c_str(), WiFi.SSID(i).c_str()) < 3;
      if (levenshteinIgnoreCase(incomingSSID.c_str(), WiFi.SSID(i).c_str()) < 3) {
        if (currMatch < prevMatch) {
          prevMatch = currMatch;
          matchID = i;
        }
      }
    }
    if (prevMatch != 255) {
      Serial.println("Found a match!");
      return WiFi.SSID(matchID);
    } else {
      Serial.println("can't find any wifi that are close enough matches in the area");
      return incomingSSID;
    }
  }
}
