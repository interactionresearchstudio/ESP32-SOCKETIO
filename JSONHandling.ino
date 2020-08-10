
//JSON Data handling

void decodeData(const char* data) {
  const size_t capacity = JSON_OBJECT_SIZE(3) + 40;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, (const char*)data);
  if (doc.containsKey("MAC")) {
    connection = paired;
    String MAC = doc["MAC"];
    Serial.println("I received a MAC address");
    Serial.println(MAC);
    if (MAC != NULL) {
      //save to preferences
      addToMacAddressJSON(MAC);
    } else {
      Serial.println("remote MAC address incorrect");
    }
  }
  if (doc.containsKey("SSID")) {
    String remoteSSID = doc["SSID"];
    Serial.println("I received a SSID");
    Serial.println(remoteSSID);
    if (remoteSSID != NULL) {
      if (doc.containsKey("PASS")) {
        String remotePASS = doc["PASS"];
        Serial.println("I received a Password");
        Serial.println(remotePASS);
        addToWiFiJSON(remoteSSID, remotePASS);
        if (remotePASS != NULL) {
          //save to preferences
        } else {
          Serial.println("remote pass empty");
        }
      }
    } else {
      Serial.println("remote ssid empty");
    }
  }
}

String getJSONMac() {
  preferences.begin("scads", false);
  String requestBody = preferences.getString("mac", "");
  return (requestBody);
}

String getJSONWifi() {
  preferences.begin("scads", false);
  String requestBody = preferences.getString("wifi", "");
  return (requestBody);
}

void addToMacAddressJSON(String addr) {
  preferences.begin("scads", false);
  String macAddressList = preferences.getString("mac", "");
  if (macAddressList != NULL) {
    const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 10;
    DynamicJsonDocument addresses(capacity);
    deserializeJson(addresses, macAddressList);
    JsonArray mac = addresses["mac"];
    for ( int i = 0; i < mac.size(); i++) {
      if (mac[i] == addr) {
        Serial.println("mac address already in list");
        break;
      }
      if (mac[mac.size() - 1] != addr) {
        mac.add(addr);
        Serial.print("adding ");
        Serial.print(addr);
        Serial.println(" to the address list");
        serializeJson(addresses, macAddressList);
        preferences.putString("mac", macAddressList);
      }
    }
  } else {
    const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 10;
    DynamicJsonDocument addresses(capacity);
    JsonArray macArray = addresses.createNestedArray("mac");
    macArray.add(addr);
    serializeJson(addresses, macAddressList);
    preferences.putString("mac", macAddressList);
    Serial.print("creating json object and adding the local mac");
    Serial.print(addr);
    Serial.println(" to the address list");
  }
  preferences.end();
}

void addToWiFiJSON(String newSSID, String newPassword) {
  preferences.begin("scads", false);
  String wifilist = preferences.getString("wifi", "");
  if (wifilist != NULL) {
    const size_t capacity = 2 * JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 150;
    DynamicJsonDocument addresses(capacity);
    deserializeJson(addresses, wifilist);
    JsonArray ssids = addresses["ssid"];
    JsonArray passwords = addresses["password"];
    for ( int i = 0; i < ssids.size(); i++) {
      if (ssids[i] == newSSID) {
        Serial.println("wifi credentials already in list");
        break;
      }
      if (ssids[ssids.size() - 1] != newSSID) {
        ssids.add(newSSID);
        passwords.add(newPassword);
        Serial.print("adding ");
        Serial.print(newSSID);
        Serial.println(" to the wifi list");
        serializeJson(addresses, wifilist);
        preferences.putString("wifi", wifilist);
      }
    }
  } else {
    const size_t capacity = 2 * JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 150;
    DynamicJsonDocument addresses(capacity);
    JsonArray ssidArray = addresses.createNestedArray("ssid");
    ssidArray.add(newSSID);
    JsonArray passwordArray = addresses.createNestedArray("password");
    passwordArray.add(newPassword);
    serializeJson(addresses, wifilist);
    preferences.putString("wifi", wifilist);
    Serial.print("creating json object and adding the local wificredentials");
    Serial.print(wifilist);
    Serial.println(" to the wifi list");
  }
  preferences.end();
}

String getRemoteMacAddress(int clientID) {
  String _macAddressJson = getJSONMac();
  const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 10;
  DynamicJsonDocument addresses(capacity);
  deserializeJson(addresses, _macAddressJson);
  JsonArray macs = addresses["mac"];
  return (macs[clientID]);
}
