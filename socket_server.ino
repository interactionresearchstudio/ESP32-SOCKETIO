//Local socket server
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {

  if (type == WS_EVT_CONNECT) {
    connection = pairing;
    Serial.println("Websocket client connection received");
    client->text("MAC");
    webSocketClientID = client->id();
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
        client->text(getJSONMac().c_str());
      }
    }
  }
}

void sendWifiCredentials() {
  //socket_server.text(webSocketClientID, (char*)text);
 // okay while we only have 1 client
 // socket_server.textAll(getWifiJSON());
}

void setupLocalServer() {
  socket_server.onEvent(onWsEvent);
  server.addHandler(&socket_server);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
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
