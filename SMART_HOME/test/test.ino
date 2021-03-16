#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

//
//
//
//char ssid[32] = "";
//char password[32] = "";
//char link[100] = "";
//char mode[5] = ""; // cam bien hay dieu khien
//char name[32] = "";// ten thiet bi
//
//
//void setup() {
//  Serial.begin(9600);
//  
//  memset(status,0,32);
//  
//  
//  
//    char temp[5]; 
//    char  hum[5]; 
//    memset(temp,0,5); 
//    memset(hum,0,5); 
//    strcat(status,"temp: ");
//    float h =4/3; 
//     dtostrf(val, 4, 2, temp);
//    
//   
//}
//
//
//
//void loop() {
//
//
//}
void setup()
{
  int i;
  float val;
  char buff[3];

  Serial.begin(9600);
  val = 0.123;
  for (i = 0; i < 10; i++) {
    dtostrf(val, 5,3, buff);  //4 is mininum width, 6 is precision
   
    Serial.print("buff: ");
    Serial.println(sizeof(buff));
     Serial.println(buff[0]);
     Serial.println(buff[1]);
     Serial.println(buff[2]);
     Serial.println(buff[3]);
     val += 5.0;
 
 
  }
  
}

void loop() {
} 
