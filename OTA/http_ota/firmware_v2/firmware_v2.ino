#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
const char* ssid = "Nguyen Tiem";
const char* password = "58642110";
const char *currentVersion = "1.0";
const char *serverUrl = "http://192.168.1.21:3000/firmware.bin";

void checkUpdate(){
   t_httpUpdate_return ret = ESPhttpUpdate.update(serverUrl, currentVersion);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(),
                    ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
  }
void setup() {
  Serial.begin(115200);
   pinMode(2, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.print("ESP8266 http update, current version: ");
  Serial.println(currentVersion);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  checkUpdate();
}
void loop() {
 
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
  delay(200);  
}
