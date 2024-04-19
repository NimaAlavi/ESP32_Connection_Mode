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
#include <HTTPClient.h>
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
                    html {font-family: Arial; display: inline-block; text-align: center;}
                    h2 {font-size: 2.6rem;}
                    body {max-width: 600px; margin:0px auto; padding-bottom: 10px;}
                    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
                    .switch input {display: none}
                    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #a50d0d; border-radius: 34px}
                    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #ffffff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
                    input:checked+.slider {background-color: #0a960d}
                    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
                </style>
            </head>
            
            <body>
                <h1>DannTech Web Server</h1>
                <h3>Under Nima's Bed Light</h3>

                <p>Bed Light State <span id="state">%STATE%</span></p>
                %BUTTONPLACEHOLDER%
                <script>
                    function toggleCheckbox(element) {
                        var xhr = new XMLHttpRequest();
                        if(element.checked){ 
                            xhr.open("GET", "/update?state=1", true); 
                            document.getElementById("state").innerHTML = "ON";  
                        }
                        else { 
                            xhr.open("GET", "/update?state=0", true); 
                            document.getElementById("state").innerHTML = "OFF";      
                        }
                        xhr.send();
                    }
                </script>
            </body>
        </html>
        )rawliteral";

const int underBedLights = 13;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/html", "Not found<a href=\"/\">Return to Home Page</a>");
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

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

String outputState(){
    if(digitalRead(underBedLights)){
        return "checked";
    }
    else {
        return "";
    }
    return "";
}
// Replaces placeholder with button section in your web page
String processor(const String& var){
    if(var == "BUTTONPLACEHOLDER"){
        String buttons = "";
        String outputStateValue = outputState();
        buttons+= "<p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label></p>";
        return buttons;
    }
    if (var == "STATE"){
        if(digitalRead(underBedLights)){
            return "ON";
        }
        else {
            return "OFF";
        }
    }
    return String();
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    pinMode(underBedLights, OUTPUT);
    digitalWrite(underBedLights, LOW);

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
                request->send_P(200, "text/html", STA_html, processor);
            });
            
            // STA_Server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
            //     LedHandleRoot(underBedLights, request);
            // });
            STA_Server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
                String inputMessage;
                String inputParam;
                if (request->hasParam("state")) {
                    inputMessage = request->getParam("state")->value();
                    inputParam = "state";
                    digitalWrite(underBedLights, inputMessage.toInt());
                }
                else {
                    inputMessage = "No message sent";
                    inputParam = "none";
                }
                Serial.println(inputMessage.toInt());
                request->send(200, "text/plain", "OK");
            });

            STA_Server.onNotFound(notFound);
            STA_Server.begin();
            Serial.println("Web server started");
        }
    }
}

void loop(){
    //Send an HTTP POST request every 2 sec
    // if ((millis() - lastTime) > timerDelay) {
    //     digitalWrite(underBedLights, HIGH);
    // }

    // Serial.println(1);
}