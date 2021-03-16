#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>



/* Set these to your desired softAP credentials. They are not configurable at runtime */
#ifndef APSSID
#define APSSID "ESP8266"
#define APPSK  ""
#endif

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "esp8266";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;
void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("connRes: ");
  Serial.println(connRes);
}
/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  EEPROM.begin(512);
//   EEPROM.write(addr, 'a'); 
//char(EEPROM.read(addr));
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
  }
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}

// hien thi trang chính 
void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>HELLO WORLD!!</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
 
  Page += F(
            "<form method='get'action='wifisave'><input id='s'name='s'length=32 placeholder='SSID'><br/><input id='p'name='p'length=64 type='password'placeholder='password'><br/>"
            "<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
            "</body></html>");

  server.send(200, "text/html", Page);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Wifi config page handler */
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>Wifi config</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>"
      "<tr><td>SSID ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' placeholder='network' name='n'/>"
            "<br /><input type='password' placeholder='password' name='p'/>"
            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

void setup() {

  Serial.begin(9600);
  Serial.println();
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", handleRoot);
//  server.on("/wifi", handleWifi);
//  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start

}



void loop() {
//  if (connect) {
//    Serial.println("Connect requested");
//    connect = false;
//    connectWifi();
//    lastConnectTry = millis();
//  }
//  {
//    unsigned int s = WiFi.status();
//    if (s == 0 && millis() > (lastConnectTry + 60000)) {
//      /* If WLAN disconnected and idle try to connect */
//      /* Don't set retry time too low as retry interfere the softAP operation */
//      connect = true;
//    }
//    if (status != s) { // WLAN status change
//      Serial.print("Status: ");
//      Serial.println(s);
//      status = s;
//      if (s == WL_CONNECTED) {
//        /* Just connected to WLAN */
//        Serial.println("");
//        Serial.print("Connected to ");
//        Serial.println(ssid);
//        Serial.print("IP address: ");
//        Serial.println(WiFi.localIP());
//
//        // Setup MDNS responder
//        if (!MDNS.begin(myHostname)) {
//          Serial.println("Error setting up MDNS responder!");
//        } else {
//          Serial.println("mDNS responder started");
//          // Add service to MDNS-SD
//          MDNS.addService("http", "tcp", 80);
//        }
//      } else if (s == WL_NO_SSID_AVAIL) {
//        WiFi.disconnect();
//      }
//    }
//    if (s == WL_CONNECTED) {
//      MDNS.update();
//    }
//  }
//  // Do work:
//  //DNS
  dnsServer.processNextRequest();
//  //HTTP
  server.handleClient();
}


/////////////////////// custom/////////////////
/*#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>


#ifndef APSSID
#define APSSID "ESP8266"
#define APPSK  ""
#endif
int de =0; 
const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;


const char *myHostname = "esp8266";


char ssid[32] = "";
char password[32] = "";
char link[100] = "";
char mode[5] = ""; // cam bien hay dieu khien
char name[32] = "";// ten thiet bi
// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);


IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);



boolean connect;


unsigned long lastConnectTry = 0;


unsigned int status = WL_IDLE_STATUS;


boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}


String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


int loadCredentials() {
  int index = 0;
  memset(ssid, 0, 32);
  memset(password, 0, 32);
  memset(link, 0, 100);
  memset(mode, 0, 5);
  memset(name, 0, 32);

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
  for (int i = 0; i < 5; i++) {
    mode[i] = char(EEPROM.read(index));
    index++;
  }
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(link);
  Serial.println(name);
  Serial.println(mode);
  if(ssid[0] == 0 ){
    return 0;
    }else{
      return 1; 
      }
}

void saveCredentials() {

  EEPROM.begin(512);
  for (int i = 0 ; i < 210; i++) {
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
  EEPROM.write(index,  mode[0]);
  EEPROM.commit();
}

// hien thi trang chính
void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<center><h1>Design by Nguyen Tiem</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>type information to add decive ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }

  Page += F(
            "<form method='POST' action='wifisave'>"
            "<input id='s'name='s'length=32 placeholder='SSID'><br/>"
            "<input id='p'name='p'length=64 type='password'placeholder='password'><br/>"
            "<br /><input type='text' placeholder='http' name='link'/>"
            "<br /><input type='text' placeholder='name' name='name'/>"
            "<br /><input  type='checkbox' name='mode'/>"
            "<br /><input type='submit' value='Connect'/></form></center>"
            "</body></html>");

  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content lengt
}
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("s").toCharArray(ssid, sizeof(ssid) - 1);
  Serial.println(ssid);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  Serial.println(password);
  server.arg("link").toCharArray(link, sizeof(link) - 1);
  Serial.println(link);
  server.arg("name").toCharArray(name, sizeof(name) - 1);
  Serial.println(name);
  server.arg("mode").toCharArray(mode, sizeof(mode) -  1); //
  Serial.println(mode);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "text/plain", "ban da dang nhap thanh cong");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

int connectWifi() {
  int flag =0; 
  if(loadCredentials()){
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  /// neu connected 
   for(int i =1 ; i<50 ;i++) 
  {
   if((WiFi.status() == WL_CONNECTED)){
    flag =1;
    Serial.println("conenected wifi"); 
    break; 
    }
  }
  return flag; 
  } else{
    return 0; 
    }
  
  
 
 
}

void addDevice(){
   Serial.println();
  Serial.println("Configuring access point...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

 dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

 server.on("/", handleRoot);
  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
 
  server.begin(); // Web server start
 
  while(!loadCredentials()){
  dnsServer.processNextRequest();
  server.handleClient();
  }
 }
 
void removeDevice(){
  // xoa rom reset
  // 
  }

void act(){

  }
  
void setup() {
  Serial.begin(9600);
  
  if(connectWifi()){
  de =1; 
  }else{
    de =0; 
    
    }
    if(de =0){
      addDevice(); 
      de =1; 
      }
      
//    while(connectWifi()) 
 
}



void loop() {


}
*/
