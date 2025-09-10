#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "network_setup.h"
#include "websocket_handler.h" // Needs onWsEvent
#include "web_page.h"          // Needs index_html

// --- Portal Settings (kept here as they are network-specific) ---
extern const char *ssid;
extern const IPAddress apIP;

// Bring the global objects into this file's scope
extern DNSServer dnsServer;
extern AsyncWebServer server;
extern AsyncWebSocket ws;

// --- FreeRTOS Task Handle ---
TaskHandle_t dnsTaskHandle = NULL;

// This class handles captive portal requests for external domains.
class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        return (request->host().length() > 0 && !request->host().equals(WiFi.softAPIP().toString()));
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        AsyncWebServerResponse *response = request->beginResponse(302);
        response->addHeader("Location", String("http://") + WiFi.softAPIP().toString());
        request->send(response);
    }
};

// FreeRTOS task to process DNS requests.
void dnsTask(void *parameter)
{
    Serial.println("DNS task started on Core 0");
    for (;;)
    {
        dnsServer.processNextRequest();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setupNetworkServices()
{
    // Configure the ESP32 as an Access Point.
    WiFi.softAP(ssid);
    delay(100);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Start the DNS server.
    dnsServer.start(53, "*", apIP);

    // Set up WebSocket and Web Server handlers.
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

    // Start the web server.
    server.begin();
    Serial.println("HTTP and WebSocket server started");
}

void startDnsTask()
{
    xTaskCreatePinnedToCore(dnsTask, "DNSTask", 3072, NULL, 1, &dnsTaskHandle, 0);
}
