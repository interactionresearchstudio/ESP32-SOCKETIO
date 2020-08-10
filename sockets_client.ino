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
        socket_client.sendTXT(getJSONMac().c_str());
      } else if (pay == "RESTART") {
        ESP.restart();
      } else {
        Serial.println("data format error");
      }
      break;
  }
}
