class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      request->send(200,"text/plain", "Hello World!");
    }
};

void setupCaptivePortal() {
  dnsServer.start(DNS_PORT, "*", apIP);
}
