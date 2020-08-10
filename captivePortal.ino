class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    const char *html PROGMEM = "<!DOCTYPE html><html lang=\"en\" dir=\"ltr\"> <head> <meta charset=\"utf-8\"> <title>SCADS</title> </head> <body> <h1>Setup</h1> <h2 id=\"result\"></h2> <form> <fieldset id=\"local_wifi\"> <label for=\"local_ssid\">Your WiFi name</label><br><input type=\"text\" id=\"local_ssid\" name=\"local_ssid\"><br><label for=\"local_pass\">Your WiFi password</label><br><input type=\"password\" id=\"local_pass\" name=\"local_pass\"><br></fieldset> <fieldset id=\"remote_wifi\"> <label for=\"remote_ssid\">Remote WiFi name</label><br><input type=\"text\" id=\"remote_ssid\" name=\"remote_ssid\"><br><label for=\"remote_pass\">Remote WiFi password</label><br><input type=\"password\" id=\"remote_pass\" name=\"remote_pass\"><br></fieldset> <fieldset id=\"mac_address\"> <p>Your MAC address: <span id=\"local_mac\"></span></p><label for=\"remote_mac\">Remote MAC address</label><br><input type=\"text\" id=\"remote_mac\" name=\"remote_mac\"><br></fieldset> </form> <button onclick=\"submitForm()\">Submit</button> <script>/* Server status: paired - Already paired with device. pairing - Currently pairing with device. detached - Not paired to any device and no device nearby. */ let mode=\"\"; function httpRequestAsync(url, type, data, callback){const xmlHttp=new XMLHttpRequest(); xmlHttp.timeout=4000; xmlHttp.onreadystatechange=function(){if (xmlHttp.readyState===4 && xmlHttp.status===200){callback(xmlHttp.responseText);}else if (xmlHttp.readyState===4 && xmlHttp.status !==200){callback(null);}}; xmlHttp.open(type, url, true); xmlHttp.send(data);}function submitForm(){let localSsid=document.getElementById(\"local_ssid\").value; let localPass=document.getElementById(\"local_pass\").value; let remoteSsid=document.getElementById(\"remote_ssid\").value; let remotePass=document.getElementById(\"remote_pass\").value; let remoteMac=document.getElementById(\"remote_mac\").value; let requestData={local_ssid: localSsid, local_pass: localPass, remote_ssid: remoteSsid, remote_pass: remotePass, remote_mac: remoteMac}; console.log(\"Request data: \"); console.log(requestData); httpRequestAsync('http://192.168.4.1/credentials', \"POST\", JSON.stringify(requestData), (res)=>{if (res){localWifiForm.style.display=\"none\"; remoteWifiForm.style.display=\"none\"; macAddressForm.style.display=\"none\"; document.getElementById(\"result\").innerHTML=\"Settings saved. You can now disconnect from this network.\";}else{document.getElementById(\"result\").innerHTML=\"Error. Could not connect to server.\";}});}let localWifiForm=document.getElementById(\"local_wifi\"); let remoteWifiForm=document.getElementById(\"remote_wifi\"); let macAddressForm=document.getElementById(\"mac_address\"); let localMacSpan=document.getElementById(\"local_mac\"); console.log(\"Hello\"); localWifiForm.style.display=\"none\"; remoteWifiForm.style.display=\"none\"; macAddressForm.style.display=\"none\"; httpRequestAsync('http://192.168.4.1/status', \"GET\", \"status\", (res)=>{console.log(res); if (res){mode=res; switch(mode){case \"paired\": localWifiForm.style.display=\"block\"; break; case \"pairing\": localWifiForm.style.display=\"block\"; remoteWifiForm.style.display=\"block\"; break; case \"detached\": localWifiForm.style.display=\"block\"; macAddressForm.style.display=\"block\"; httpRequestAsync('http://192.168.4.1/mac', \"GET\", \"mac\", (res)=>{if (res){document.getElementById(\"local_mac\").textContent=res;}else{document.getElementById(\"result\").innerHTML=\"Error. Could not connect to server.\";}}); break;}}else{document.getElementById(\"result\").innerHTML=\"Error. Could not connect to server.\";}}); </script> </body></html>";

    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {

      Serial.print("Requested in captive ");
      Serial.print(request->url());
      Serial.print(" type: ");
      Serial.println(request->method());

      if (request->method() == 1) {
        if (request->url() == "/status") {
          Serial.println("Received device status request");
          request->send(200, "text/plain", getDeviceStatus());
        }
        else if (request->url() == "/mac") {
          Serial.println("Received mac address request");
          request->send(200, "text/plain", WiFi.macAddress());
        }
        else {
          request->send_P(200, "text/html", html);
        }
      }
    }

    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!index) {
        Serial.printf("BodyStart: %u B\n", total);
      }
      for (size_t i = 0; i < len; i++) {
        Serial.write(data[i]);
      }
      if (index + len == total) {
        Serial.printf("BodyEnd: %u B\n", total);
      }

      if (request->url() == "/credentials") {
        Serial.println("Handle credentials here");
        request->send(200, "text/plain", "OK");
      }
    }
};

void setupCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", apIP);
}
