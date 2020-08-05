void postDataToServer() {
  //Sends Server device MAC address and receives Server device MAC address in return, then saves to preferences.
  HTTPClient http;
  if (wifiMulti.run() == WL_CONNECTED) {
    http.begin("http://192.168.4.1/MAC/");
    http.addHeader("Content-Type", "application/json");

    const size_t capacity = JSON_OBJECT_SIZE(1) + 38;
    DynamicJsonDocument retDoc(capacity);
    retDoc["MAC"] = WiFi.macAddress();
    String requestBody;
    serializeJson(retDoc, requestBody);
    Serial.println(requestBody);
    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      //Save to preferences
      //deserialise
      DynamicJsonDocument responseDoc(capacity);
      String json = response;
      deserializeJson(responseDoc, json);
      String MAC = responseDoc["MAC"];
      saveMac(MAC);
      blinkForever();
    } else {
      Serial.println("error");
    }
  }
}

void socket_Connected(const char * payload, size_t length) {
  Serial.println("Socket.IO Connected!");
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, 1);
  delay(100);
  digitalWrite(LEDPin, 0);
  delay(100);
}

void socket_sendMac(const char * payload, size_t length) {
  Serial.println("GOT MAC REQUEST");
  const size_t capacity = JSON_OBJECT_SIZE(1) + 50;
  DynamicJsonDocument doc(capacity);
  doc["macAddress"] = WiFi.macAddress();
  String bodyReq;
  serializeJson(doc, bodyReq);
  Serial.println(bodyReq);
  webSocket.emit("mac", bodyReq.c_str());
}

void socket_event(const char * payload, size_t length) {
  Serial.print("got message: ");
  Serial.println(payload);
}

void socket_msg(const char * payload, size_t length) {
  Serial.println("got msg");
  const size_t capacity = JSON_OBJECT_SIZE(2) + 50;
  DynamicJsonDocument incomingDoc(capacity);
  deserializeJson(incomingDoc, payload);
  const char* recMacAddress = incomingDoc["macAddress"];
  const char* recData = incomingDoc["data"];
  Serial.print("I got a message from ");
  Serial.println(recMacAddress);
  Serial.print("Which is ");
  Serial.println(recData);
  String testt = String(recData);
  if (testt.indexOf("hello") > -1) {
    LEDState = !LEDState;
  }

}
