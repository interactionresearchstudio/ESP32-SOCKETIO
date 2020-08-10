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

//const char *rootHtml PROGMEM = "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"> <head> <meta charset=\"utf-8\"> <title>SCADS</title> </head> <body> <h1>Setup</h1> <h2 id=\"result\"></h2> <form> <fieldset id=\"local_wifi\"> <label for=\"local_ssid\">Your WiFi name</label><br><input type=\"text\" id=\"local_ssid\" name=\"local_ssid\"><br><label for=\"local_pass\">Your WiFi password</label><br><input type=\"password\" id=\"local_pass\" name=\"local_pass\"><br></fieldset> <fieldset id=\"remote_wifi\"> <label for=\"remote_ssid\">Remote WiFi name</label><br><input type=\"text\" id=\"remote_ssid\" name=\"remote_ssid\"><br><label for=\"remote_pass\">Remote WiFi password</label><br><input type=\"password\" id=\"remote_pass\" name=\"remote_pass\"><br></fieldset> <fieldset id=\"mac_address\"> <p>Your MAC address: <span id=\"local_mac\"></span></p><label for=\"remote_mac\">Remote MAC address</label><br><input type=\"text\" id=\"remote_mac\" name=\"remote_mac\"><br></fieldset> </form> <button onclick=\"submitForm()\">Submit</button> <script>/* Server status: paired - Already paired with device. pairing - Currently pairing with device. detached - Not paired to any device and no device nearby. */ let mode=\"\"; function httpRequestAsync(url, type, data, callback){const xmlHttp=new XMLHttpRequest(); xmlHttp.onreadystatechange=function(){if (xmlHttp.readyState===4 && xmlHttp.status===200){callback(xmlHttp.responseText);}else if (xmlHttp.readyState===4 && xmlHttp.status !==200){callback(null);}}; xmlHttp.open(type, url, true); xmlHttp.send(data);}function submitForm(){let localSsid=document.getElementById(\"local_ssid\").value; let localPass=document.getElementById(\"local_pass\").value; let remoteSsid=document.getElementById(\"remote_ssid\").value; let remotePass=document.getElementById(\"remote_pass\").value; let remoteMac=document.getElementById(\"remote_mac\").value; let requestData={local_ssid: localSsid, local_pass: localPass, remote_ssid: remoteSsid, remote_pass: remotePass, remote_mac: remoteMac}; console.log(\"Request data: \"); console.log(requestData); httpRequestAsync('http://192.168.4.1/credentials', \"POST\", JSON.stringify(requestData), (res)=>{if (res){localWifiForm.style.display=\"none\"; remoteWifiForm.style.display=\"none\"; macAddressForm.style.display=\"none\"; document.getElementById(\"result\").innerHTML=\"Settings saved. You can now disconnect from this network.\";}else{document.getElementById(\"result\").innerHTML=\"Error. Could not connect to server.\";}});}let localWifiForm=document.getElementById(\"local_wifi\"); let remoteWifiForm=document.getElementById(\"remote_wifi\"); let macAddressForm=document.getElementById(\"mac_address\"); let localMacSpan=document.getElementById(\"local_mac\"); console.log(\"Hello\"); localWifiForm.style.display=\"none\"; remoteWifiForm.style.display=\"none\"; macAddressForm.style.display=\"none\"; httpRequestAsync('http://192.168.4.1/status', \"GET\", \"status\", (res)=>{console.log(res); if (res){mode=res; switch(mode){case \"paired\": localWifiForm.style.display=\"block\"; break; case \"pairing\": localWifiForm.style.display=\"block\"; remoteWifiForm.style.display=\"block\"; break; case \"detached\": localWifiForm.style.display=\"block\"; macAddressForm.style.display=\"block\"; httpRequestAsync('http://192.168.4.1/mac', \"GET\", \"mac\", (res)=>{if (res){document.getElementById(\"local_mac\").textContent=res;}else{document.getElementById(\"result\").innerHTML=\"Error. Could not connect to server.\";}}); break;}}else{document.getElementById(\"result\").innerHTML=\"Error. Could not connect to server.\";}}); </script> </body></html>";

void setupLocalServer() {
  socket_server.onEvent(onWsEvent);
  server.addHandler(&socket_server);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  
//  server.on("/credentials", HTTP_POST, [](AsyncWebServerRequest * request) {
//    Serial.println("Received credentials.");
//    // TODO - Get credentials here.
//    request->send(200, "text/plain", "OK");
//  });
//
//  server.on("/mac", HTTP_GET, [](AsyncWebServerRequest * request) {
//    Serial.println("Received mac address request");
//    request->send(200, "text/plain", WiFi.macAddress());
//  });
//
//  server.on("/status", HTTP_GET, [](AsyncWebServerRequest * request) {
//    Serial.println("Received device status request");
//    request->send(200, "text/plain", getDeviceStatus());
//  });

//  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
//    Serial.println("Received root request");
//    request->send_P(200, "text/html", rootHtml);
//  });
  
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
