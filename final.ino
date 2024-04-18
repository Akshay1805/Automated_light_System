#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"

// Set to true to define Relay as Normally Open (NO)
#define RELAY_NO    true

// Set number of relays
#define NUM_RELAYS  5

// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {5, 4, 14, 12, 13};

// Replace with your network credentials
const char* ssid = "HOLA";
const char* password = "hola1234";

const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Web Server</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        text-align: center;
      }
      .switch-container {
        display: flex;
        flex-direction: column;
        align-items: center; /* Centers items horizontally */
      }
      .switch {
        margin-bottom: 10px; /* Adds some spacing between buttons */
      }
      .switch input {
        opacity: 0;
        width: 0;
        height: 0;
      }
      .slider {
        position: relative;
        display: inline-block;
        width: 60px;
        height: 34px;
        background-color: #ccc;
        -webkit-transition: 0.4s;
        transition: 0.4s;
      }
      .slider:before {
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: white;
        -webkit-transition: 0.4s;
        transition: 0.4s;
      }
      input:checked + .slider {
        background-color: #2196f3;
      }
      input:focus + .slider {
        box-shadow: 0 0 1px #2196f3;
      }
      input:checked + .slider:before {
        -webkit-transform: translateX(26px);
        -ms-transform: translateX(26px);
        transform: translateX(26px);
      }
      .slider.round {
        border-radius: 34px;
      }
      .slider.round:before {
        border-radius: 50%;
      }
    </style>
  </head>
  <body>
    <h1>Web Server</h1>
    <div class="switch-container">
      <p>On/Off</p>
      <label class="switch">
        <!-- Add onclick event to call toggleLight() function -->
        <input id="onOffButton" type="checkbox" onchange="toggleLight()" />
        <span class="slider round"></span>
      </label>
      <p>Automate</p>
      <label class="switch">
        <input id="automateButton" type="checkbox" onchange="toggleAutomation()" />
        <span class="slider round"></span>
      </label>
    </div>

    <script>
      var automateState = false; // Global variable to track automation state

      function toggleLight() {
        var onOffButton = document.getElementById("onOffButton");
        automateButton.checked = false;
        // Send request to server to toggle light
        var xhttp = new XMLHttpRequest();
        var url = "/light?state=" + (onOffButton.checked ? "1" : "0");
        xhttp.open("GET", url, true);
        xhttp.send();

        var xhttp = new XMLHttpRequest();
        var url = "/automation?state=" + ("0");
        xhttp.open("GET", url, true);
        xhttp.send();
      }

      function toggleAutomation() {
        var automateButton = document.getElementById("automateButton");

        // Update automation state
        automateState = automateButton.checked;

        // Send request to server to toggle automation
        var xhttp = new XMLHttpRequest();
        var url = "/automation?state=" + (automateState ? "1" : "0");
        xhttp.open("GET", url, true);
        xhttp.send();

        var xhttp = new XMLHttpRequest();
        var url = "/light?state=" + ("0");
        xhttp.open("GET", url, true);
        xhttp.send();

      }
    </script>
  </body>
</html>

)rawliteral";

int auto_flag = 0;
int on_off_flag = 0;

int LED = 2;       
int MOTION_SENSOR = 0;
unsigned long lastTriggerTime = 0;
const unsigned long delayTime = 5000; // 5 seconds delay
#define lighsensor A0

bool flag1 = false;
bool flag2 = false;

void setup() {
  Serial.begin(115200);
  
  // Set all relays to off when the program starts - if set to Normally Open (NO), the relay is off when you set the relay to HIGH
  for(int i=0; i<NUM_RELAYS; i++){
    pinMode(relayGPIOs[i], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i], LOW);
    }
  }
  
  pinMode(MOTION_SENSOR, INPUT);  
  pinMode(LED, OUTPUT);           
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/light", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String state = request->getParam("state")->value();
    if(state == "1") {
      on_off_flag = 1; // Turn on LED
      digitalWrite(LED, HIGH);
    } else {
      on_off_flag = 0; // Turn off LED
      digitalWrite(LED, LOW);
    }
    Serial.println(on_off_flag);
    request->send(200, "text/plain", "OK");
  });

  server.on("/automation", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String state = request->getParam("state")->value();
    if(state == "1") {
      // Enable automation
      auto_flag = 1; // Set the automation flag
       // Turn on the LED initially
    } else {
      // Disable automation
      auto_flag = 0; // Clear the automation flag
       // Turn off the LED
    }
    Serial.println(auto_flag);
    request->send(200, "text/plain", "OK");
  });

  server.on("/toggleFlag1", HTTP_GET, [](AsyncWebServerRequest *request){
    // Toggle flag 1
    flag1 = !flag1;
    request->send(200, "text/plain", "OK");
  });

  server.on("/toggleFlag2", HTTP_GET, [](AsyncWebServerRequest *request){
    // Toggle flag 2
    flag2 = !flag2;
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
 

  if(on_off_flag==1){
    digitalWrite(LED, HIGH);
  }
if (auto_flag==1){
 
  int lightsensorvalue = analogRead(lighsensor);
  int motionsensorValue = digitalRead(MOTION_SENSOR);

  Serial.println("Motion : ");
  Serial.println(motionsensorValue);
  Serial.println("Light : ");
  Serial.println(lightsensorvalue);


  if (motionsensorValue == 1) {
    if(lightsensorvalue>624){
      digitalWrite(LED, HIGH); // Turn on LED
      lastTriggerTime = millis(); // Update last trigger time
    } 
  } else if (millis() - lastTriggerTime > delayTime) {
    digitalWrite(LED, LOW); // Turn off LED
  }
}
}
