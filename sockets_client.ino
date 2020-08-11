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
      socket_client.sendTXT(getJSONMac().c_str());
      break;
    case WStype_TEXT:
      Serial.println("Text:");
      Serial.println((char *)payload);
      if ((String)(char *)payload == "RESTART") {
        softReset();
        Serial.println("i'm going to reset");
      } else {
        decodeData((char *)payload);
      }
      break;
  }
}
