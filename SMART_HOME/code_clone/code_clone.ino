#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "DHT.h"

#define DHTPIN 5 // what digital pin we're connected to

#define DHTTYPE DHT11 // DHT 11

#ifndef APSSID
#define APSSID "ESP8266"
#define APPSK  ""
#endif

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

/** Load ten wifi- pass -  link - name - mode -*/
void setPin() {
  EEPROM.begin(512);
  mode = (char)(EEPROM.read(196));
  pin = (int)(EEPROM.read(197));
  if (mode == D) { // su dung thu vien
    DHT dht(pin, DHTTYPE);
    dht.begin();
  } else {

    if (pin == 4) {
      pinMode(pin, INPUT);// 
      return;
    }
    if (pin == 65 || pin == 97) { // doc analog;
      pin = -1;
      return;

    }
    if (pin == 2) {
      pinMode(pin, OUTPUT);
      return;
    }
  }
}

void readSensor() {
  
  memset(status,0,32);
  if (mode == 'D') {
    char temp[5]; 
    char  hum[5]; 
    memset(temp,0,5); 
    memset(hum,0,5); 
    strcat(status,"hum ");
    float h = dht.readHumidity();
     dtostrf(h, 5,2, hum);
      strcat(status,hum);
      strcat(status,"temp ");
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
     dtostrf(t, 5,2, temp);
      strcat(status,temp);
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      memcpy(status,"error"); 
    }  
  }
  else {
    if (pin <= -1) {
      int temp = analogRead(A0);
      /// conert to status
      
      itoa(temp, status, 10);
    }
    else {
      int temp = digitalRead(pin);
       itoa(temp, status, 10);
      ////
    }
  }
}

void setDevice(int onoff) {
  /// decode status  
  digitalWrite(pin,onoff); 
}

void pushDataHttp(){
   
  }
  
void getDataHttp(){

  }

void control(){
     
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
  //   EEPROM.write(addr, 'a');
  //char(EEPROM.read(addr));

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
///////////////////////////
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
    // receive incoming UDP packets
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



}



void loop() {


}