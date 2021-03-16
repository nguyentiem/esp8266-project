#include <ESP8266WiFi.h>
const char* ssid = "Nguyen Tiem";
const char* password = "58642110";
const char* host = "192.168.1.21"; //IP des Java-Servers
const int serverPort = 88; //Port des Java-Servers (ServerSocket)
void setup() {
 Serial.begin(9600); //Kontrollausgabe aktivieren
// delay(800);
 Serial.println();
 Serial.print("connect to wifi: ");
 Serial.print(ssid);

 WiFi.begin(ssid, password);
 /*Solange keine Verbindung zu einem AccessPoint (AP) aufgebaut wurde*/
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("connected.");
 Serial.print(" IP ");
 Serial.println(WiFi.localIP());
 /*Signalstärke des AP*/
 long rssi = WiFi.RSSI();
 Serial.print("(RSSI) to AP:");
 Serial.print(rssi);
 Serial.println(" dBm");
}
void loop() {
 WiFiClient client;

 if (!client.connect(host, serverPort)) {
 Serial.print("X");
 return;
 }

 client.println("esp8266 hello");
 delay(100);
 /*Echo vom Java-Server lesen und ausgeben*/
 String line = client.readStringUntil('\n');
 Serial.print(line);
 Serial.println();
 /*Verbindung zum Java-Server schliessen*/
 
 client.flush();
 client.stop();
 
}
