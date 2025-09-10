#include "websocket_handler.h"

// Bring the global 'ws' object into this file's scope
extern AsyncWebSocket ws;

// Handles WebSocket events
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
        {
            data[len] = 0; // Null-terminate the string
            Serial.printf("Received message from client #%u: %s\n", client->id(), (char *)data);

            // Broadcast the received message to all connected clients
            ws.textAll((char *)data);
        }
        break;
    }
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}
