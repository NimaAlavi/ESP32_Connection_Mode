#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
    #include <AsyncTCP.h>
#else
    #include <ESP8266WiFi.h>
    #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
// #include <WebServer.h>
#include "AP_Connection.h"
#include <EEPROM.h>

#define EEPROM_SIZE 50

AsyncWebServer AP_Server(80);
AsyncWebServer STA_Server(80);

const char* boardSSID     = "DannTech";
const char* boardPassword = "Nima7281";

// Variable to store the HTTP request
const char*  wifiSSID     = "wifiName";
const char*  wifiPassword = "wifiPass";

IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

String wifiName, wifiPass;

const char AP_html[] PROGMEM = R"rawliteral(
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

const char STA_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            html {
                font-family: Helvetica;
                display: inline-block;
                margin: 0px auto;
                text-align: center;
            }
            
            h1 {
                margin-top: 50px;
            }
            
            .button1 {
                background-color: #0a960d;
                border: none;
                color: white;
                padding: 16px 40px;
                text-decoration: none;
                font-size: 30px;
                margin: 2px;
                cursor: pointer;
                border-radius: 68px;
            }
            
            .button1:hover {
                background-color: darkblue;
            }
            
            .button2 {
                background-color: #a50d0d;
                border: none;
                color: white;
                padding: 16px 40px;
                text-decoration: none;
                font-size: 30px;
                margin: 2px;
                cursor: pointer;
                border-radius: 68px;
            }
            
            .button2:hover {
                background-color: darkblue;
            }
        </style>
    </head>
    
    <body>
        <h1>DannTech Web Server</h1>
        <h3>Under Nima's Bed Light</h3>
        <form action="/get">
            <button id="myButton" name="underBedLight" class="button1" value="ON" onclick="sendRequest()">ON</button>
        </form>
        
        <script>
            function sendRequest() {
                var button = document.getElementById('myButton');
                var currentState = button.innerHTML.toUpperCase();
                var nextState = currentState === 'ON' ? 'off' : 'on';

                button.innerHTML = nextState.toUpperCase();
                button.className = nextState === 'on' ? 'button1' : 'button2';
                button.value = nextState === 'on' ? 'ON' : 'OFF';
                
                // Uncomment the lines below if you want to make a POST request
                // var xhr = new XMLHttpRequest();
                // xhr.open("POST", "/get", true);
                // xhr.send();
            }
        </script>
    </body>
</html>
        )rawliteral";

const int underBedLights = 27;

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

void LedHandleRoot(int ledPin, AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam("underBedLight")) {
        String state = request->getParam("underBedLight")->value();
        if (state == "ON") {
            digitalWrite(ledPin, HIGH);  // Turn on the LED
            message = "LED turned on";
            // Serial.println(message);
        } 
        else if (state == "OFF") {
            digitalWrite(ledPin, LOW);   // Turn off the LED
            message = "LED turned off";
            // Serial.println(message);
        }
        else {
            // message = "Invalid state parameter";
        }
    } 
    else {
        // message = "No state parameter provided";
    }

    request->send(200, "text/html", message + "<a href=\"/\">Return to Home Page</a>");
    // request->send(200, "text/html");
            
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    pinMode(underBedLights, OUTPUT);

    if (EEPROM.read(0) != 0x0F){
        Serial.print("Setting up Access Point IP...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(boardSSID, boardPassword);

        Serial.println();
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());

        AP_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", AP_html);
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
            importRam(20, wifiPass);
            EEPROM.commit();
        }
        });
        AP_Server.onNotFound(notFound);
        AP_Server.begin();
    }

    if (EEPROM.read(0) == 0x0F){
        wifiName = exportRam(1);
        wifiPass = exportRam(20);
        Serial.println("Setting up wifi-Mode...");
        WiFi.mode(WIFI_STA);

        // Configures static IP address
        if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
            Serial.println("STA Failed to configure");
        }
        // Connect to Wi-Fi network with SSID and password
        Serial.print("Connecting to ");
        Serial.println(wifiName);
        WiFi.begin(wifiName, wifiPass);

        int i = 0;
        while (WiFi.status() != WL_CONNECTED){
            delay(500);
            Serial.print(".");
            if(i==10){
                EEPROM.write(0, 0xF0);
                EEPROM.commit();   
                break;
            }
            i++;
        }
        if(EEPROM.read(0) == 0x0F){
            // Print local IP address and start web server
            Serial.println("");
            Serial.print("WiFi connected to ");
            Serial.println(wifiName);
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());

            STA_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send_P(200, "text/html", STA_html);
            });
            
            STA_Server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
                LedHandleRoot(underBedLights, request);
            });

            STA_Server.onNotFound(notFound);
            STA_Server.begin();
            Serial.println("Web server started");
        }
    }
}

void loop(){

}