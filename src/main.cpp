#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "DHTesp.h"
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>

// void ICACHE_RAM_ATTR ON_OFF();

const char *ssid = "Wifi Rumah";
const char *password = "123456789987654321";
const char *mqtt_server = "ee.unsoed.ac.id";
const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycbxd_pQTqVVkPkaixGvgaaypAU2UNuRTzBvMA0vswkGz7xg0XGo";

DHTesp dht;
ESP8266WebServer server(80);
WiFiClient espClient;
WiFiClientSecure nodemcuClient;
PubSubClient client(espClient);

const uint16_t kIrLed = 4;
IRDaikinESP ac(kIrLed); 

int suhuRlain[2];
int statRlain[2];
int nyalaRlain[2];
int statSystem;
int nyalaSystem;
const byte Pintombol = 0;

const int led = 16;
int statlampu;
unsigned long waktu;
unsigned long waktuAC;
unsigned long lastAC = 0;
unsigned long lastMQTT = 0;
unsigned long lastGSHEET = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int statLED;
float humidity, temperature;
String nilai;
char pesan[100];

void readsenDHT11()
{
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
}
void led_on()
{
  // server.send(200, "text/plain", "LED Hidup");
  digitalWrite(led, 0);
  statlampu = 1;
  Serial.println("LED Hidup");
}
void led_off()
{
  // server.send(200, "text/plain", "LED Padam");
  digitalWrite(led, 1);
  statlampu = 0;
  Serial.println("LED Padam");
}
void handleRoot()
{
  if (server.hasArg("lampu"))
  {
    
    if (server.arg("lampu") == "1")
    {
      led_on();
    }
    else if (server.arg("lampu") == "0")
    {
      led_off();
    }
  }
  String IP = WiFi.localIP().toString();
  String html;
  html += "<html>";
  html += "<head>";
  html += "<title>Smart Home Kecil Kecilan</title>";
  html += "<meta http-equiv=\"refresh\" content=\"2\">";
  html += "</head>";
  html += "<body>";
  html += "<h3>NodeMCU 1<br>";
  html += "</h3>";
  html += "ChipID = ";
  html += String(ESP.getFlashChipId()).c_str();
  html += "<br>";
  html += "IP = ";
  html += IP;
  html += "<h3>Sensor Suhu dan Kelembaban";
  html += "</h3>";
  readsenDHT11();
  html += "<p>Pembacaan suhu : ";
  html += temperature;
  html += "</p>";
  html += "<p>Pembacaan kelembaban : ";
  html += humidity;
  html += "</p>";
  html += "<h3>Kendali Lampu";
  html += "</h3>";
  if (statlampu == true)
    html += "<p>Status Lampu : NYALA BOR</p>";
  else
    html += "<p>Status Lampu : MATI BOR</p>";
  html += "</h3>";
  html += "<p>Kontrol Lampu : <a href=\"/?lampu=1\"><button type='button'>ON!</button></a>&nbsp;<a href=\"/?lampu=0\"> <button type='button'>OFF</button> </a>";
  html += "</p>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

void handleNotFound()
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char buff_p[length];
  String msg_p;
  if(strcmp(topic, "iot19202/kelompok_8/suhuAgung")==0)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      buff_p[i] = (char)payload[i];
    }
    Serial.println();
    buff_p[length] = '\0';
    msg_p = String(buff_p);
    suhuRlain[0] = msg_p.toInt(); // to Int
  }
  else if(strcmp(topic, "iot19202/kelompok_8/suhuAom")==0)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      buff_p[i] = (char)payload[i];
    }
    Serial.println();
    buff_p[length] = '\0';
    msg_p = String(buff_p);
    suhuRlain[0] = msg_p.toInt(); // to Int
  }
  else if(strcmp(topic, "iot19202/kelompok_8/statAgung")==0)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    if ((char)payload[0] == '1') {
      statRlain[0] = 1;
    } 
    else if((char)payload[0] == '0') {
      statRlain[0] = 0;
    }
  }
  else if(strcmp(topic, "iot19202/kelompok_8/statAom")==0)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    if ((char)payload[0] == '1') {
      statRlain[1] = 1;
    } 
    else if((char)payload[0] == '0') {
      statRlain[1] = 0;
    }
  }
  else if(strcmp(topic, "iot19202/kelompok_8/nyalaAgung")==0)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    if ((char)payload[0] == '1') {
      nyalaRlain[0] = 1;
    } 
    else if((char)payload[0] == '0') {
      nyalaRlain[0] = 0;
    }
  }
  else if(strcmp(topic, "iot19202/kelompok_8/nyalaAom")==0)
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    if ((char)payload[0] == '1') {
      nyalaRlain[1] = 1;
    } 
    else if((char)payload[0] == '0') {
      nyalaRlain[1] = 0;
    }
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe("iot19202/kelompok_8/suhuAgung");
      client.subscribe("iot19202/kelompok_8/suhuAom");
      client.subscribe("iot19202/kelompok_8/statAgung");
      client.subscribe("iot19202/kelompok_8/statAom");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendData(int tem, int hum, int nyala, int waktu)
{
  String NoNode = "node2";
  Serial.print("connecting to ");
  Serial.println(host);
  if (!nodemcuClient.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String string_nyalaAC;
  String string_waktu_nyalaAC = String (waktu, DEC);
  if (nyala == 1)
    string_nyalaAC =  "Menyala";
  else
    string_nyalaAC =  "Mati";
  String url = "/macros/s/" + GAS_ID + "/exec?" + NoNode +"&temperature=" + string_temperature + "&humidity=" + string_humidity + "&statAC=" + string_nyalaAC + "&waktu_nyalaAC=" + string_waktu_nyalaAC;
  Serial.print("requesting URL: ");
  Serial.println(url);

  nodemcuClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (nodemcuClient.connected()) {
    String line = nodemcuClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = nodemcuClient.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  
} 
void set_OTA()
{
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void nyalakanAC()
{
  if (lastAC == 0)
  {
    waktuAC = millis();
  }
  nyalaSystem = 1;
  ac.on();
  ac.setFan(1);
  ac.setMode(kDaikinCool);
  ac.setTemp(25);
}

void matikanAC()
{
  nyalaSystem = 0;
  ac.off();
  lastAC = (millis() - waktuAC)/1000;
}

void set_wifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266"))
  {
    Serial.println("MDNS responder started");
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  digitalWrite(16, HIGH);
  statLED = 0;

  server.on("/", handleRoot);

  server.on("/led_hidup", led_on);

  server.on("/led_mati", led_off);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void  ICACHE_RAM_ATTR ON_OFF()
{
  if (statSystem == 0)
    statSystem = 1;
  else if (statSystem == 1)
    statSystem = 0;
}

void setup(void)
{
  
  pinMode(Pintombol, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Pintombol), ON_OFF, FALLING);
  
  ac.begin();
  nodemcuClient.setInsecure();
  dht.setup(2, DHTesp::DHT11);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  set_wifi();
  set_OTA();
  Serial.println("Ready");
  Serial.println(WiFi.localIP());
  statSystem = 0;
}

void loop(void)
{ 
  ArduinoOTA.handle();
  if(statSystem == 1)
  {
    waktu = millis();
    int total_statSystem;
    int total_nyalaSystem;

    MDNS.update();
    server.handleClient();
    
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();

    total_statSystem = statSystem;
    for (int i = 0; i < 2; i++) {
        total_statSystem += statRlain[i];
    }

    total_nyalaSystem = nyalaSystem;
    for (int i = 0; i < 2; i++) {
        total_nyalaSystem += nyalaRlain[i];
    }

    if (total_statSystem == 3 && (suhuRlain[0] > 25 || suhuRlain[1] > 25 || temperature > 25))
    {
      if (suhuRlain[0] < temperature && suhuRlain[1] < temperature && (suhuRlain[0] < 24 || suhuRlain[1] < 24)
      && nyalaSystem == 0)
      {
        nyalakanAC();
      }
      else if (temperature < 24 && total_nyalaSystem > 1)
      {
        matikanAC();
        lastAC = 0;
      }
      if (total_nyalaSystem < 2 && ((suhuRlain[0] < temperature && suhuRlain[1] > temperature) || (suhuRlain[0] > temperature && suhuRlain[1] < temperature)))
      {
        nyalakanAC();
      }
    }
    else if (total_statSystem < 3)
    {
      nyalakanAC();
    }

    if (waktu - lastMQTT > 2000)
    {
      lastMQTT = waktu;
      nilai = "";
      nilai += String(statSystem);
      Serial.print("Publish status system : ");
      Serial.println(nilai);
      nilai.toCharArray(pesan, sizeof(pesan));
      client.publish("iot19202/kelompok_8/statAkhdan", pesan);

      nilai = "";
      nilai += String(nyalaSystem);
      Serial.print("Publish nyala system : ");
      Serial.println(nilai);
      nilai.toCharArray(pesan, sizeof(pesan));
      client.publish("iot19202/kelompok_8/nyalaAkhdan", pesan);
      
      readsenDHT11();

      nilai = "";
      nilai += String(temperature);
      nilai += " Derajat";
      Serial.print("Publish suhu: ");
      Serial.println(nilai);
      nilai.toCharArray(pesan, sizeof(pesan));
      client.publish("iot19202/kelompok_8/suhuAkhdan", pesan);

      nilai = "";
      nilai += String(humidity);
      nilai += "%";
      Serial.print("Publish kelembaban: ");
      Serial.println(nilai);
      nilai.toCharArray(pesan, sizeof(pesan));
      client.publish("iot19202/kelompok_8/kelembabanAkhdan", pesan);
    }

    if (waktu - lastGSHEET > 30000)
    {
      lastGSHEET = waktu;
      lastAC = (millis() - waktuAC)/1000;
      sendData(temperature, humidity, nyalaSystem, lastAC);
    }
  }
  if(statSystem == 0)
  {
    matikanAC();
    
  } 
}
