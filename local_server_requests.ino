//Local socket server

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {

  if (type == WS_EVT_CONNECT) {

    Serial.println("Websocket client connection received");
    client->text("MAC");
    Serial.println(client->id());

  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("Client disconnected");
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len) {
      //the whole message is in a single frame and we got all of it's data
      Serial.println(client->id());
      if (info->opcode == WS_TEXT) {
        data[len] = 0;
        Serial.println((const char*)data);
        decodeData((const char*)data);
        client->text(returnJSONMac().c_str());
      }
    }
  }
}

void setupLocalServer() {
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  server.begin();
  Serial.println("Local Socket server started");
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//Client local socket

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected!");
      break;
    case WStype_TEXT:
      Serial.println("Text:");
      Serial.println((const char *)payload);
      String pay = (const char *)payload;
      if (pay == "MAC") {
        localSocket.sendTXT(returnJSONMac().c_str());
      } else {
        decodeData((const char*)payload);
      }
      break;
  }
}

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
