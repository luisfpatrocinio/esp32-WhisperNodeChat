#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

// --- Portal Settings ---
const char *ssid = "SALA DE CHAT (PODE ENTRAR)"; // The name of the Wi-Fi network the ESP32 will create
const IPAddress apIP(192, 168, 4, 1);            // The IP address of the portal

// --- Global Objects ---
DNSServer dnsServer;
AsyncWebServer server(80); // Web server on port 80
AsyncWebSocket ws("/ws");  // WebSocket server on URI /ws

// --- FreeRTOS Task Handles ---
TaskHandle_t dnsTaskHandle = NULL;

// HTML for the chat page.
// Includes CSS for styling and JavaScript for WebSocket communication.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Chat Local</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; margin: 0; background-color: #f0f2f5; display: flex; justify-content: center; align-items: center; height: 100vh; }
    .chat-wrapper { width: 100%; height: 100%; display: flex; flex-direction: column; }
    #setup-container { text-align: center; padding: 20px; background-color: #fff; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); margin: auto; }
    #setup-container h1 { margin-top: 0; }
    #setup-container input { padding: 10px; border: 1px solid #ddd; border-radius: 4px; font-size: 16px; width: calc(100% - 22px); }
    #setup-container button { margin-top: 10px; padding: 10px 20px; border: none; background-color: #007bff; color: white; border-radius: 4px; font-size: 16px; cursor: pointer; }
    #chat-container { display: none; flex-direction: column; width: 100%; height: 100%; background-color: #fff; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
    #messages { flex: 1; overflow-y: auto; padding: 10px; display: flex; flex-direction: column; }
    .message { max-width: 70%; padding: 8px 12px; margin-bottom: 8px; border-radius: 18px; line-height: 1.4; word-wrap: break-word; }
    .message.own-message { background-color: #007bff; color: white; align-self: flex-end; }
    .message.other-message { background-color: #e4e6eb; color: #050505; align-self: flex-start; }
    .message strong { font-weight: 600; display: block; margin-bottom: 2px; font-size: 0.8em; color: #65676b; }
    .message.own-message strong { color: #f0f2f5; }
    .system-message { align-self: center; background-color: #f0f2f5; color: #65676b; font-size: 0.8em; padding: 4px 10px; border-radius: 10px; }
    #input-area { display: flex; padding: 10px; border-top: 1px solid #ddd; background-color: #f0f2f5; }
    #input-area input { flex: 1; padding: 10px; border: 1px solid #ccc; border-radius: 18px; outline: none; }
    #input-area button { margin-left: 10px; padding: 10px 15px; border: none; background-color: #007bff; color: white; border-radius: 18px; cursor: pointer; }
  </style>
</head>
<body>
  <div class="chat-wrapper">
    <div id="setup-container">
      <h1>Bem-vindo ao Chat</h1>
      <p>Digite seu nome para entrar:</p>
      <input type="text" id="username-input" placeholder="Seu nome..." maxlength="20" autocomplete="off">
      <button onclick="joinChat()">Entrar</button>
    </div>
    <div id="chat-container">
      <div id="messages"></div>
      <div id="input-area">
        <input type="text" id="message-input" placeholder="Digite uma mensagem..." autocomplete="off">
        <button onclick="sendMessage()">Enviar</button>
      </div>
    </div>
  </div>
  <script>
    let ws;
    let username = '';
    function joinChat() {
      username = document.getElementById('username-input').value.trim();
      if (!username) { alert('Por favor, digite um nome.'); return; }
      document.getElementById('setup-container').style.display = 'none';
      document.getElementById('chat-container').style.display = 'flex';
      document.getElementById('message-input').focus();
      initWebSocket();
    }
    function initWebSocket() {
      ws = new WebSocket('ws://' + window.location.host + '/ws');
      ws.onopen = (event) => { displaySystemMessage('Conectado ao chat!'); };
      ws.onclose = (event) => { displaySystemMessage('Desconectado. Tentando reconectar...'); setTimeout(initWebSocket, 2000); };
      ws.onmessage = (event) => {
        const data = JSON.parse(event.data);
        displayUserMessage(data.user, data.message);
      };
    }
    function sendMessage() {
      const messageInput = document.getElementById('message-input');
      const message = messageInput.value.trim();
      if (message && ws.readyState === WebSocket.OPEN) {
        const data = { user: username, message: message };
        ws.send(JSON.stringify(data));
        messageInput.value = '';
      }
    }
    function displayUserMessage(user, message) {
      const messagesDiv = document.getElementById('messages');
      const msgContainer = document.createElement('div');
      msgContainer.classList.add('message', user === username ? 'own-message' : 'other-message');
      if (user !== username) {
        msgContainer.innerHTML = `<strong>${escapeHTML(user)}</strong>`;
      }
      const p = document.createElement('p');
      p.textContent = message;
      msgContainer.appendChild(p);
      messagesDiv.appendChild(msgContainer);
      messagesDiv.scrollTop = messagesDiv.scrollHeight;
    }
    function displaySystemMessage(message) {
      const messagesDiv = document.getElementById('messages');
      const msgElement = document.createElement('div');
      msgElement.classList.add('system-message');
      msgElement.textContent = message;
      messagesDiv.appendChild(msgElement);
      messagesDiv.scrollTop = messagesDiv.scrollHeight;
    }
    function escapeHTML(str) { return str.replace(/[&<>"']/g, m => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'})[m]); }
    document.getElementById('username-input').addEventListener('keyup', (e) => { if (e.key === 'Enter') joinChat(); });
    document.getElementById('message-input').addEventListener('keyup', (e) => { if (e.key === 'Enter') sendMessage(); });
  </script>
</body>
</html>
)rawliteral";

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

// This class handles captive portal requests for external domains.
// It redirects any request for a domain other than our own to the root page.
class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    // We only want to handle requests to domains that are not our ESP32's IP.
    bool canHandle(AsyncWebServerRequest *request)
    {
        // If the host header is present and is not our IP, we can handle it.
        return (request->host().length() > 0 && !request->host().equals(WiFi.softAPIP().toString()));
    }

    // Handle the request by redirecting the client to the portal's root page.
    void handleRequest(AsyncWebServerRequest *request)
    {
        AsyncWebServerResponse *response = request->beginResponse(302);
        // Redirect to the root of our server.
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

void setup()
{
    Serial.begin(115200);
    Serial.println("\nStarting Captive Portal Chat Server...");

    // Configure the ESP32 as an Access Point.
    WiFi.softAP(ssid);
    delay(100);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    // Start the DNS server.
    dnsServer.start(53, "*", apIP);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Set up the WebSocket event handler.
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // Serve the main chat page at the root URL.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    // Add the Captive Portal handler. This is what triggers the auto-popup.
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

    // Start the web server.
    server.begin();
    Serial.println("HTTP and WebSocket server started");

    // Create the DNS server task and pin it to Core 0.
    xTaskCreatePinnedToCore(dnsTask, "DNSTask", 3072, NULL, 1, &dnsTaskHandle, 0);
}

void loop()
{
    // The loop is empty because all work is now handled by FreeRTOS tasks
    // and asynchronous server callbacks.
    vTaskDelete(NULL); // Delete the loop task to free up resources.
}
