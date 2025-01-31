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

/*
  water diverter valve 
  pinMode default mode is HIGH
  HIGH means the relay is open and the diverter valve is routing the water to the shower head 
  LOW means the relay is closed and the diverter valve is routing the water back to the main tank 

*/
// include webservers libraries and arduino json library
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include "constants.h"


struct ValveState {
  int pinState;
  String state;
  String detail;
};


// valve connection
int relayPIN = 27;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;           // your network key index number (needed only for WEP)

WebServer server(80);


void setup(void) {
  ValveState vs;
  server.enableCORS(true);
  pinMode(relayPIN, OUTPUT);
  digitalWrite(relayPIN, HIGH);
  vs.pinState = 1;
  vs.state = "OPENED";
  vs.detail = ValveOpenedMessage;
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

  server.on("/relaystatus", handleValveState);

  server.on(F("/togglerelay"), handleForm);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(1000);  // Wait for 1000 millisecond(s)
  int value = digitalRead(8);
  // Serial.println(value);
}

// web server fuctions

void handleForm() {
  ValveState vs;
  Serial.println(server.method());

  if (server.method() == HTTP_POST) {
    Serial.printf("relay status is :  %d \n\n", digitalRead(relayPIN));
    Serial.println(server.arg(0));

    if (server.arg(0) == "1") {
      digitalWrite(relayPIN, HIGH);
      vs.pinState = 1;
      vs.state = "OPENED";
      vs.detail = ValveOpenedMessage;

      Serial.printf("new relay status is :  %d \n\n", digitalRead(relayPIN));
      // server.send(200, "text/plain", "{\"status\":\"1\"}");
      server.send(200, "application/json", "{\"pinState\":\"" + String(vs.pinState) + "\",\"valveState\":\"" + vs.state + "\",\"detail\":\"" + vs.detail + "\"}");
    } else {
      digitalWrite(relayPIN, LOW);

      vs.pinState = 0;
      vs.state = "CLOSED";
      vs.detail = ValveClosedMessage;

      Serial.printf("new relay status is :  %d \n\n", digitalRead(relayPIN));
      // server.send(200, "text/plain", "{\"status\":\"0\"}");
      server.send(200, "application/json", "{\"pinState\":\"" + String(vs.pinState) + "\",\"valveState\":\"" + vs.state + "\",\"detail\":\"" + vs.detail + "\"}");
    }
  }
}

void handleValveState() {
  int relayStatus = digitalRead(relayPIN);
  ValveState vs;
  Serial.printf("relay status is :  %d", relayStatus);
  Serial.println("");
  if (relayStatus == 1) {
    vs.pinState = 1;
    vs.state = "OPENED";
    vs.detail = ValveOpenedMessage;
    // server.send(200, "text/plain", "1");
    server.send(200, "application/json", "{\"pinState\":\"" + String(vs.pinState) + "\",\"valveState\":\"" + vs.state + "\",\"detail\":\"" + vs.detail + "\"}");
  } else {
    vs.pinState = 0;
    vs.state = "CLOSED";
    vs.detail = ValveClosedMessage;
    server.send(200, "application/json", "{\"pinState\":\"" + String(vs.pinState) + "\",\"valveState\":\"" + vs.state + "\",\"detail\":\"" + vs.detail + "\"}");
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