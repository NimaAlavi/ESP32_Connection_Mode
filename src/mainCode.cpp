#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
    #include <AsyncTCP.h>
#else
    #include <ESP8266WiFi.h>
    #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "AP_Connection.h"
#include <EEPROM.h>

#define EEPROM_SIZE 30

AsyncWebServer AP_Server(80);
AsyncWebServer STA_Server(80);
const char* boardSSID     = "DannTech";
const char* boardPassword = "Nima7281";

// Variable to store the HTTP request
const char*  wifiSSID     = "wifiName";
const char*  wifiPassword = "wifiPass";
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

String wifiName, wifiPass;

const char index_html[] PROGMEM = R"rawliteral(
            <!DOCTYPE html>
            <html>
                <head><meta name="viewport" content="width=device-width, initial-scale=1\">
                    <style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
                    </style>
                </head>
                <body>
                    <h1>ESP32 Web Server</h1>
                    <h3>Enter Your wifi Name & Password</h3>
                    <form action="/get">
                        <input type="text" name= "wifiName" value= "WiFi Name"><br>
                        <input type="password" name="wifiPass" value= "WiFi Password"><br><br>
                        <input type="submit" value="Submit">
                    </form>
                </body>
            </html>
        )rawliteral";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void importRam(int address, String Mon){
    byte len = Mon.length();
    EEPROM.write(address,len);

    for (int i = 0; i < len; i++) {
    EEPROM.write(address + 1 + i, Mon[i]);
    }
}
String exportRam(int address){
  int len = EEPROM.read(address);
  char data[len + 1];

  for (int i = 0; i < len; i++){
    data[i] = EEPROM.read(address + 1 + i);
  }
  data[len] = '\0';

  return String(data);
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);

    if (EEPROM.read(0) != 0x0F){
        Serial.print("Setting up Access Point IP...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(boardSSID, boardPassword);

        Serial.println();
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());

        AP_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html);
        });
        AP_Server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request){
        if (request->hasParam("wifiName") && request->hasParam("wifiPass")){
            wifiName = request->getParam("wifiName")->value();
            Serial.println(wifiName);
            wifiPass = request->getParam("wifiPass")->value();
            Serial.println(wifiPass);
            request->send(200, "text/html", "HTTP GET request sent to your ESP on input field <br><a href=\"/\">Return to Home Page</a>");
            EEPROM.write(0, 0x0F);
            importRam(1, wifiName);
            importRam(10, wifiPass);
            EEPROM.commit();
        }
        });
        AP_Server.onNotFound(notFound);
        AP_Server.begin();
    }

    if (EEPROM.read(0) == 0x0F){
        wifiName = exportRam(1);
        wifiPass = exportRam(10);
        Serial.print("Setting up wifi-Mode...");
        WiFi.mode(WIFI_STA);

        // Configures static IP address
        if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
            Serial.println("STA Failed to configure");
        }
        // Connect to Wi-Fi network with SSID and password
        Serial.print("Connecting to ");
        Serial.println(wifiName);
        WiFi.begin(wifiName, wifiPass);
        while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        }
        // Print local IP address and start web server
        Serial.println("");
        Serial.println("WiFi connected.");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        STA_Server.onNotFound(notFound);
        STA_Server.begin();
    }
}

void loop(){
    
}