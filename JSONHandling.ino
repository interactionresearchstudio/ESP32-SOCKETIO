
//JSON Data handling

void decodeData(const char* data) {
  const size_t capacity = JSON_OBJECT_SIZE(3) + 40;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, (const char*)data);
  if (doc.containsKey("MAC")) {
    String MAC = doc["MAC"];
    Serial.println("I received a MAC address");
    Serial.println(MAC);
    if (MAC != NULL) {
      //save to preferences
      saveMac(MAC);
    } else {
      Serial.println("remote MAC address incorrect");
    }
  }
  if (doc.containsKey("SSID")) {
    String remoteSSID = doc["SSID"];
    Serial.println("I received a SSID");
    Serial.println(remoteSSID);
    if (remoteSSID != NULL) {
      //save to preferences
    } else {
      Serial.println("remote ssid empty");
    }
  }
  if (doc.containsKey("PASS")) {
    String remotePASS = doc["PASS"];
    Serial.println("I received a Password");
    Serial.println(remotePASS);
    if (remotePASS != NULL) {
      //save to preferences
    } else {
      Serial.println("remote pass empty");
    }
  }
}

String returnJSONMac() {
  const size_t capacity = JSON_OBJECT_SIZE(3) + 40;
  DynamicJsonDocument returnDoc(capacity);
  returnDoc["MAC"] = WiFi.macAddress();
  returnDoc["SSID"] = "testSsid";
  returnDoc["PASS"] = "testPass";
  String requestBody;
  serializeJson(returnDoc, requestBody);
  Serial.println(requestBody);
  return (requestBody);
}

void saveMac(String mac) {
  preferences.begin("scads", false);
  preferences.putString("mac", mac);
  Serial.println("Saved mac to nvm");
  preferences.end();
}
