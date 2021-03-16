#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <Hash.h>
int temPin =A0; 
ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;
float tem =0; 
#define USE_SERIAL Serial
String output;
void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  Serial.println(type);
    switch(type) {
        case sIOtype_DISCONNECT:
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
            USE_SERIAL.printf("[IOc] get event: %s\n", payload);
            break;
        case sIOtype_ACK:
            USE_SERIAL.printf("[IOc] get ack: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_ERROR:
            USE_SERIAL.printf("[IOc] get error: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_EVENT:
            USE_SERIAL.printf("[IOc] get binary: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_ACK:
            USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
            hexdump(payload, length);
            break;
    }
}

void setup() {
   
    USE_SERIAL.begin(9600);

   pinMode(temPin, INPUT); 
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    // disable AP
//    if(WiFi.getMode() & WIFI_AP) {
//        WiFi.softAPdisconnect(true);
//    }

  
 WiFi.begin("Nguyen Toi", "11111111");
 /*Solange keine Verbindung zu einem AccessPoint (AP) aufgebaut wurde*/
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("connected.");
 Serial.print(" IP ");
 Serial.println(WiFi.localIP());
    // server address, port and URL
    socketIO.begin("192.168.0.106", 3000);

    // event handler
//    socketIO.onEvent(socketIOEvent);
     
}

unsigned long messageTimestamp = 0;
void loop() {
  
    socketIO.loop();
    
    uint64_t now = millis();

    if(now - messageTimestamp > 2000) {
       tem = analogRead(temPin)/3-15;
     output="[\"message\","; 
        output+=String(tem)+String("]");  
        messageTimestamp = now;
        USE_SERIAL.println(output);
        socketIO.sendEVENT(output);
    }
}
