#include <ESP8266WiFi.h>                 // thu vien mang cho esp8266 
#include <WiFiClient.h>                  // 
#include <ESP8266WebServer.h>            // webserver 
#include <ESP8266mDNS.h>               // thu vien dinh nghia ten mien
#include <ESP8266HTTPUpdateServer.h>  // thu vien update

const char* ssid = "Nguyen Tiem";
const char* password = "58642110";

const char* host = "boot";
const char* updatePath = "/update";
const char* updateUsername = "nguyentiem";     //password
const char* updatePassword = "11111111";      // password

ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
//-----------------------------------------//
const char MainPage[] PROGMEM = R"=====(
  <!DOCTYPE html> 
  <html>
   <head> 
       <title>OTA-Nguyen Van Tiem</title> 
       <style> 
          body{
            text-align: center;
          }
       </style>
       <meta name="viewport" content="width=device-width,user-scalable=0" charset="UTF-8">
   </head>
   <body> 
      <div>
        <img src='https://dienthongminhesmart.000webhostapp.com/firmware_ota.jpg' height='200px' width='330px'>
      </div>
      <div>
        <button onclick="window.location.href='/update'">UPLOAD FIRMWARE</button><br><br>
//        <a href='http://bit.ly/EsmartChannel'>XEM THÊM NHỮNG VIDEO MỚI NHẤT</a>
      </div>
      <script>
      </script>
   </body> 
  </html>
)=====";

void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting programs...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  MDNS.begin(host);     // multicast DNS
  MDNS.addService("http", "tcp", 80);

  httpUpdater.setup(&webServer, updatePath, updateUsername, updatePassword); // 
  webServer.on("/",[]{
    String s = MainPage;
    webServer.send(200,"text/html",s);
  });
  webServer.begin();
  Serial.println("Web Server is started!");

  //=========Chương trình Chính=====//
  pinMode(2,OUTPUT);
  //============End=================//
}

void loop(void){
  MDNS.update();
  webServer.handleClient();
  //====Chương trình Chính==========//
  digitalWrite(2,!digitalRead(2));
  delay(1000);
  //=========End====================//
}
