#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h> // Required for PROGMEM

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

#endif // WEB_PAGE_H
