
# esp32-WhisperNodeChat

A standalone communication node using an ESP32. It creates a Wi-Fi network and hosts a private, local chat room accessible via a captive portal.

This project is a learning exercise to explore the capabilities of the ESP32, including Wi-Fi Access Point management, WebSockets for real-time communication, and FreeRTOS for multitasking.

## ✨ Features

-   **Wi-Fi Access Point:** Creates its own Wi-Fi network. No router needed.
    
-   **Offline Chat:** Allows users on the same network to chat without an internet connection.
    
-   **Captive Portal:** Automatically opens the chat page on devices upon connection.
    
-   **Real-Time Communication:** Uses WebSockets for instant message delivery.
    
-   **Multitasking:** Leverages FreeRTOS to handle DNS and web server tasks efficiently on different CPU cores.
    

## 🚀 How to Use

1.  Power on the ESP32.
    
2.  On your phone or computer, connect to the Wi-Fi network named **"Chat-Local-ESP32"**.
    
3.  The chat page should open automatically in your browser. If not, open a browser and try to navigate to any website.
    
4.  Enter a username and start chatting with other connected users.
    

## 🛠️ Getting Started

This project was developed using [PlatformIO](https://platformio.org/ "null") with Visual Studio Code.

## ⚙️ Hardware

-   An ESP32 development board.
    

## 💻 Software & Dependencies

1.  Clone this repository.
    
2.  Open the project in VSCode with the PlatformIO extension installed.
    
3.  The required library will be installed automatically based on the `platformio.ini` file:
    
    -   `esphome/ESPAsyncWebServer-esphome`
        
4.  Build and upload the project to your ESP32.
    

## 📄 License

This project is licensed under the MIT License. Feel free to use it in your projects -- just make sure to credit **Luis Felipe Patrocinio** ([GitHub](https://github.com/luisfpatrocinio "null")). See the [LICENSE](https://www.google.com/search?q=./LICENSE "null") file for full details.

_This is a learning project and is not intended for production use._