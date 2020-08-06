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

void connectToWifi() {
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
