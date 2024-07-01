/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// include webservers libraries and arduino json library
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"
// Include DallasTemperature and One wire
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 4
float waterTemp;
float desiredWaterTemp = 100.00;
char tp = 'F';
int seconds = 0;
float tf;


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// init json doc
JsonDocument jsonData;
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
// valve connection

int relayPIN = 26;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;           // your network key index number (needed only for WEP)

WebServer server(80);

void setup(void) {
  server.enableCORS(true);
  pinMode(relayPIN, OUTPUT);
  digitalWrite(relayPIN, LOW);
  // start serial port
  Serial.begin(115200);
  // set the wifi mode
  WiFi.mode(WIFI_STA);
  // start the wifi
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.printf("Connected to: %s - IP address: %s", ssid, WiFi.localIP());
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/relaystatus", []() {
    int relayStatus = digitalRead(relayPIN);

    Serial.printf("relay status is :  %d", relayStatus);
    Serial.println("");
    if (relayStatus == 1) {
      server.send(200, "text/plain", "1");
    } else {
      server.send(200, "text/plain", "0");
    }
  });
  
  server.on(F("/togglerelay"), handleForm);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(10000);  // Wait for 1000 millisecond(s)
  seconds += 1;
  int value = digitalRead(8);
  // Serial.println(value);
  tf = getWaterTemp();
  Serial.println(tf);
  if (tf > 0) {
    if (tf <= desiredWaterTemp) {

      Serial.println("Shower is Cold");
    } else {

      Serial.println("Shower is HOT");
    }
  } else {
    Serial.println("Error: Could not read temperature data");
  }
}



float getWaterTemp() {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  // Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();  // Send the command to get temperatures
  // Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float tempC = sensors.getTempCByIndex(0);
  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print(tempC);
    Serial.print("°C --- ");
    float tempinFahrenheit = DallasTemperature::toFahrenheit(tempC);
    Serial.print(DallasTemperature::toFahrenheit(tempC));  // Converts tempC to Fahrenheit
    Serial.print("°F");
    Serial.println();
    return tempC, tempinFahrenheit;
  } else {
    Serial.println("Error: Could not read temperature data");
    return 0.00;
  }
  return 0.00;
}

char getPreferredTemp(char c) {
  switch (c) {
    case 'F':
      Serial.println("your temp preference is F");
      break;
    case 'C':
      Serial.println("your temp preference is C");
      break;
  }
}
// web server fuctions

void handleForm() {
  Serial.println(server.method());
  
  if (server.method() == HTTP_POST) {

    Serial.printf("relay status is :  %d \n\n", digitalRead(relayPIN));

    Serial.println(server.arg(0));
    if (server.arg(0) == "1") {
      digitalWrite(relayPIN, HIGH);
      Serial.printf("new relay status is :  %d \n\n", digitalRead(relayPIN));
      server.send(200, "text/plain", "{\"status\":\"1\"}");
    } else {
      digitalWrite(relayPIN, LOW);
      Serial.printf("new relay status is :  %d \n\n", digitalRead(relayPIN));
      server.send(200, "text/plain", "{\"status\":\"0\"}");
    }
  }
}
void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(404, "text/plain", message);
}