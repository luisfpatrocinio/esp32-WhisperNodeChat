#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

#include "network_setup.h"
#include "websocket_handler.h"

// --- Global Objects (Definition) ---
// These are the actual objects. Other files refer to them via 'extern'.
DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// --- Global Settings ---
const char *ssid = "SALA DE CHAT (PODE ENTRAR)";
const IPAddress apIP(192, 168, 4, 1);

void setup()
{
    Serial.begin(115200);
    Serial.println("\nStarting Captive Portal Chat Server...");

    // Call the setup functions from our modules
    setupNetworkServices();
    startDnsTask();

    Serial.println("Setup complete. Deleting setup/loop task.");
}

void loop()
{
    // The loop is empty because all work is now handled by FreeRTOS tasks
    // and asynchronous server callbacks.
    vTaskDelete(NULL); // Delete the loop task to free up resources.
}
