void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
/*
  void handleMac(AsyncWebServerRequest * request) {
  //Receives client devices MAC and stores to preferences, then returns own MAC
  const size_t capacity = JSON_OBJECT_SIZE(1) + 38;
  DynamicJsonDocument doc(capacity);
  String json = server.arg("plain");
  deserializeJson(doc, json);
  String MAC = doc["MAC"];
  Serial.println(MAC);
  //save to preferences
  saveMac(MAC);
  DynamicJsonDocument returnDoc(capacity);
  returnDoc["MAC"] = WiFi.macAddress();
  String requestBody;
  serializeJson(returnDoc, requestBody);
  Serial.println(requestBody);
  request->send(200, "application/json", + requestBody);

  blinkForever();
  }
*/

bool exchangeMac(AsyncWebServerRequest *request, uint8_t *datas) {
  const size_t capacity = JSON_OBJECT_SIZE(1) + 38;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, (const char*)datas);
  String MAC = doc["MAC"];
  Serial.println("I JUST RECEIVED");
  Serial.println(MAC);
  if (MAC != NULL) {
    //save to preferences
    saveMac(MAC);
    DynamicJsonDocument returnDoc(capacity);
    returnDoc["MAC"] = WiFi.macAddress();
    String requestBody;
    serializeJson(returnDoc, requestBody);
    Serial.println(requestBody);
    return true;
  } else {
    return false;
  }
}
