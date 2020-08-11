

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hello World!");
    }
};

void setupCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", apIP);
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


String getDeviceStatus() {
  /*
     If we are currently pairing, return "pairing"
     If we have already paired, return "paired"
     If there is no other device in sight and we haven't paired, return "detached"
  */
  if (connection == pairing) {
    return "pairing";
  } else if (connection == paired) {
    return "paired";
  } else if (connection == detached) {
    return "detached";
  }
}
