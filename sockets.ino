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
  socket_server.onEvent(onWsEvent);
  server.addHandler(&socket_server);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  
  server.on("/credentials", HTTP_POST, [](AsyncWebServerRequest * request) {
    // TODO - Get credentials here.
    request->send(200, "text/plain", "OK");
  });

  server.on("/mac", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", WiFi.macAddress());
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", getDeviceStatus());
  });
  
  server.begin();
  Serial.println("Local Socket server started");
}

void resetBoards() {
  socket_server.textAll("RESTART");
  //Not sure if this is needed
  long softReset = millis();
  while (millis() - softReset < 1000) {
  }
  ESP.restart();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//Client local socket

void setupSocketClientEvents() {
  socket_client.begin("192.168.4.1", 80, "/ws");
  socket_client.onEvent(webSocketEvent);
  socket_client.setReconnectInterval(5000);
}

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
        socket_client.sendTXT(returnJSONMac().c_str());
      } else if (pay == "RESTART") {
        ESP.restart();
      } else {
        decodeData((const char*)payload);
      }
      break;
  }
}

String getDeviceStatus() {
  /*
   * If we are currently pairing, return "pairing"
   * If we have already paired, return "paired"
   * If there is no other device in sight and we haven't paired, return "detached"
   */
  return "pairing";
}
