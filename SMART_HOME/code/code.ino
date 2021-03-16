#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include "DHT.h"
#define DHTPIN 5 // what digital pin we're connected to
#define DHTTYPE DHT11 // DHT 11
#ifndef APSSID
#define APSSID "ESP8266"
#define APPSK  ""
#endif
#define USE_SERIAL Serial

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

int pin = 2;
char mode = 'N'; /// = a/d
const char *myHostname = "esp8266";

char ssid[32] = "";  // ten wifi
char password[32] = ""; // passwifi
char link[100] = ""; // ten http
char name[32] = "";// ten thiet bi
char status[32] = "";

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
WiFiUDP Udp;
unsigned int localUdpPort = 88;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
//char  replyPacket[] = "esp8266 UDP hi!";  // a reply string to send back
ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  Serial.println(type);
  switch (type) {
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

/** Load ten wifi- pass -  link - name - mode -*/
void control() {
  memset(status, 0, 32);
  EEPROM.begin(512);
  mode = (char)(EEPROM.read(196));
  pin = (int)(EEPROM.read(197));
  if (mode == 'D') { // su dung thu vien
    DHT dht(pin, DHTTYPE);
    dht.begin();
    char temp[5];
    char  hum[5];
    memset(temp, 0, 5);
    memset(hum, 0, 5);
    strcat(status, "hum ");
    float h = dht.readHumidity();
    dtostrf(h, 5, 2, hum);
    strcat(status, hum);
    strcat(status, "temp ");
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    dtostrf(t, 5, 2, temp);
    strcat(status, temp);
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) ) {
      Serial.println("Failed to read from DHT sensor!");
      strcpy(status, "error");
    }
  } else {

    if (pin == 4) {
      pinMode(pin, INPUT);//
      int temp = digitalRead(pin);
      itoa(temp, status, 10);
      return;
    }
    if (pin == 65 || pin == 97) { // doc analog;
      pin = -1;
      int temp = analogRead(A0);
      /// conert to status

      itoa(temp, status, 10);
      return;

    }
    if (pin == 2) {
      pinMode(pin, OUTPUT);
      return;
    }
  }
}


void setDevice(int onoff) {
  /// decode status
  digitalWrite(pin, onoff);
}

void connectToHttp() {
  socketIO.begin(link, 3000);
  socketIO.onEvent(socketIOEvent);
}


void push() {
  uint64_t now = millis();
  uint64_t messageTimestamp;
  if (now - messageTimestamp > 2000) {
   
    String output =""; 
    output = "[\"message\",";
    output += String(status) + String("]");
    messageTimestamp = now;
    USE_SERIAL.println(output);
    socketIO.sendEVENT(output);
  }
}

void clear() {
  memset(ssid, 0, 32);
  memset(password, 0, 32);
  memset(link, 0, 100);
  memset(name, 0, 32);
  memset(status, 0, 32);
}

int readRom() {
  int index = 0;
  clear();
  EEPROM.begin(512);
  for (int i = 0; i < 32; i++) {
    ssid[i] = char(EEPROM.read(index));
    index++;
  }
  for (int i = 0; i < 32; i++) {
    password[i] = char(EEPROM.read(index));
    index++;
  }
  for (int i = 0; i < 100; i++) {
    link[i] = char(EEPROM.read(index));
    index++;
  }
  for (int i = 0; i < 32; i++) {
    name[i] = char(EEPROM.read(index));
    index++;
  }
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(link);
  Serial.println(name);

  if (ssid[0] == 0 ) {
    return 0;
  } else {
    return 1;
  }
}
/** Store WLAN credentials to EEPROM */
void saveRom() {

  EEPROM.begin(512);
  for (int i = 0 ; i < 196; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.begin(512);
  int index = 0;
  for (int i = 0; i < 32; i++) {
    EEPROM.write(index,  ssid[i]);
    index++;
  }
  for (int i = 0; i < 32; i++) {
    EEPROM.write(index,  password[i]);
    index++;
  }
  for (int i = 0; i < 100; i++) {
    EEPROM.write(index,  link[i]);
    index++;
  }
  for (int i = 0; i < 32; i++) {
    EEPROM.write(index,  name[i]);
    index++;
  }
  EEPROM.commit();
}

void clearRom() {
  EEPROM.begin(512);
  for (int i = 0 ; i < 196; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void connectWifi() {
  WiFi.softAPdisconnect (true);
  readRom();
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("connRes: ");
  Serial.println(connRes);
}

void softReset() {
  ESP.restart();
}

void configMode() {
  WiFi.mode(WIFI_AP);
  Serial.println();
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

int  recv() {

  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    return 1;
  } else {
    return 0;
  }
}

int send(char* replyPacket) {
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(replyPacket);
  Udp.endPacket();
}

void setup() {
  Serial.begin(9600);
  // cau hinh wifi


}


void loop() {
  socketIO.loop();

}
