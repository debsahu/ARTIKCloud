#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Hash.h>
#include <Ticker.h>
#include "version.h"
#include "secrets.h"

/////////////////////// Edit this area to your liking //////////////////////////
#ifndef PIO_PLATFORM
#define USE_PUBSUB      //uncomment if you want to use PubSubClient
#define USE_WIFIMANAGER //uncomment if you want to use WiFiManager
#define USE_ASYNC_WEBSERVER
#endif
#ifndef SECRET
#ifndef USE_WIFIMANAGER
#define WIFI_SSID "WIFISSID"
#define WIFI_PASS "WIFIPASSWORD"
#endif

char mqttCloudUsername[40] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
char mqttCloudPassword[40] = "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";

int8_t TIME_ZONE = -5; //NYC(USA): -5 UTC
//#define USE_SUMMER_TIME_DST  //uncomment to use DST
#endif
/////////////////////////////////////////////////////////////////////////////////

#ifdef USE_NEOPIXELBUS
#include <NeoPixelBrightnessBus.h> //https://github.com/Makuna/NeoPixelBus
#else
#include <Adafruit_NeoPixel.h> //https://github.com/adafruit/Adafruit_NeoPixel
#define PIN 3
#endif

#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson/releases/download/v6.8.0-beta/ArduinoJson-v6.8.0-beta.zip
#ifdef USE_ASYNC_WEBSERVER
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer
#else
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WebSocketsServer.h> //https://github.com/Links2004/arduinoWebSockets
#endif
#include "indexhtml.h"
#if (defined(USE_WIFIMANAGER) and defined(USE_ASYNC_WEBSERVER))
#include <ESPAsyncDNSServer.h> //https://github.com/devyte/ESPAsyncDNSServer
#include <ESPAsyncUDP.h>
#include <ESPAsyncWiFiManager.h> //https://github.com/alanswx/ESPAsyncWiFiManager
AsyncDNSServer dns;
#elif (defined(USE_WIFIMANAGER) and !defined(USE_ASYNC_WEBSERVER))
  #include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#endif

#ifdef ARDUINO_ESP8266_RELEASE_2_4_2
extern "C"
{
#include "libb64/cdecode.h"
}
#endif

const uint16_t PixelCount = 16;
char mqttCloudClientName[] = "Ring Lights";
char HOSTNAME[] = "Artik Cloud Lights";

////////////////////////////////////////////////
///// Begin ARTIK Stuff /////
const char mqttCloudServer[] = "api.artik.cloud";
int mqttCloudPort = 8883;
char mqttCloudPubTopic[64];
char mqttCloudSubTopic[64];

/**/
//Obtained by running: openssl s_client -showcerts -verify 5 -connect api.artik.cloud:8883
static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIElDCCA3ygAwIBAgIQAf2j627KdciIQ4tyS8+8kTANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0xMzAzMDgxMjAwMDBaFw0yMzAzMDgxMjAwMDBaME0xCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxJzAlBgNVBAMTHkRpZ2lDZXJ0IFNIQTIg
U2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
ANyuWJBNwcQwFZA1W248ghX1LFy949v/cUP6ZCWA1O4Yok3wZtAKc24RmDYXZK83
nf36QYSvx6+M/hpzTc8zl5CilodTgyu5pnVILR1WN3vaMTIa16yrBvSqXUu3R0bd
KpPDkC55gIDvEwRqFDu1m5K+wgdlTvza/P96rtxcflUxDOg5B6TXvi/TC2rSsd9f
/ld0Uzs1gN2ujkSYs58O09rg1/RrKatEp0tYhG2SS4HD2nOLEpdIkARFdRrdNzGX
kujNVA075ME/OV4uuPNcfhCOhkEAjUVmR7ChZc6gqikJTvOX6+guqw9ypzAO+sf0
/RR3w6RbKFfCs/mC/bdFWJsCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8C
AQAwDgYDVR0PAQH/BAQDAgGGMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYY
aHR0cDovL29jc3AuZGlnaWNlcnQuY29tMHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6
Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwN6A1
oDOGMWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RD
QS5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8v
d3d3LmRpZ2ljZXJ0LmNvbS9DUFMwHQYDVR0OBBYEFA+AYRyCMWHVLyjnjUY4tCzh
xtniMB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA0GCSqGSIb3DQEB
CwUAA4IBAQAjPt9L0jFCpbZ+QlwaRMxp0Wi0XUvgBCFsS+JtzLHgl4+mUwnNqipl
5TlPHoOlblyYoiQm5vuh7ZPHLgLGTUq/sELfeNqzqPlt/yGFUzZgTHbO7Djc1lGA
8MXW5dRNJ2Srm8c+cftIl7gzbckTB+6WohsYFfZcTEDts8Ls/3HB40f/1LkAtDdC
2iDJ6m6K7hQGrn2iWZiIqBtvLfTyyRRfJs8sjX7tN8Cp1Tm5gr8ZDOo0rwAhaPit
c+LJMto4JQtV05od8GiG7S5BNO98pVAdvzr508EIDObtHopYJeS4d60tbvVS3bR0
j6tJLp07kzQoH3jOlOrHvdPJbRzeXDLz
-----END CERTIFICATE-----
)EOF";
/**/
///// End ARTIK Stuff /////
#if (ARDUINOJSON_VERSION_MAJOR == 6 and ARDUINOJSON_VERSION_MINOR != 8)
#error "Install ArduinoJson v6.8.0-beta" ///// Install correct version of ArduinoJSON /////
#endif

#ifdef USE_SUMMER_TIME_DST
uint8_t DST = 1;
#else
uint8_t DST = 0;
#endif

#ifdef USE_NEOPIXELBUS
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(PixelCount); // Use RX Pin on ESP8266
NeoGamma<NeoGammaTableMethod> colorGamma;
#else
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PixelCount, PIN, NEO_GRB + NEO_KHZ800);
#endif
#ifdef USE_ASYNC_WEBSERVER
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
#else
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WebSocketsServer webSocket = WebSocketsServer(81);
#endif
WiFiClientSecure net;

#ifdef USE_PUBSUB
#ifndef PIO_PLATFORM
#define MQTT_MAX_PACKET_SIZE 512 //left here for Arduino IDE
#endif
#include <PubSubClient.h>
PubSubClient client(net);
#else
#include <MQTT.h>
MQTTClient client(512);
#endif

Ticker sendStat, sendWSStat, pFH;

struct
{
  uint8_t r = 255;
  uint8_t g = 255;
  uint8_t b = 255;
} color;

unsigned long lastMillis = 0;
boolean stateOn = false;
uint8_t level = 100;
const uint8_t c_MinBrightness = 8;
const uint8_t c_MaxBrightness = 255;
#ifdef USE_NEOPIXELBUS
RgbColor WHITE(color.r, color.g, color.b);
RgbColor BLUE(0, 0, 64);
RgbColor BLACK(0, 0, 0);
#endif
#if defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA2) or defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA3) or defined(ARDUINO_ESP8266_GIT_DESC)
//BearSSL::X509List cert(cacert);
time_t now, nowish = 1510592825;
void NTPConnect(void)
{
  Serial.print(F("Setting time using SNTP"));
  configTime(TIME_ZONE * 3600, DST * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(F("."));
    now = time(nullptr);
  }
  Serial.println(F("done!"));
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}
#endif

String sendStatusMsg(bool ARTIKCloud = false)
{
#if ARDUINOJSON_VERSION_MAJOR == 6
  StaticJsonDocument<256> jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();
#else
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
#endif
  if (ARTIKCloud)
  {
    if (stateOn)
      root["state"] = "ON";
    else
      root["state"] = "OFF";
  }
  else
    root["state"] = stateOn;
  root["level"] = level;
#if ARDUINOJSON_VERSION_MAJOR == 6
  JsonObject colorRGB = root.createNestedObject("colorRGB");
#else
  JsonObject &colorRGB = root.createNestedObject("colorRGB");
#endif
  colorRGB["r"] = color.r;
  colorRGB["g"] = color.g;
  colorRGB["b"] = color.b;
  String msg_str;
#if ARDUINOJSON_VERSION_MAJOR == 6
  serializeJson(root, msg_str);
#else
  root.printTo(msg_str);
#endif
  return msg_str;
}

void sendMQTTStatusMsg(void)
{
  Serial.print(F("Sending["));
  Serial.print(mqttCloudPubTopic);
  Serial.print(F("] >> "));
  Serial.println(sendStatusMsg(true));
  client.publish(mqttCloudPubTopic, sendStatusMsg().c_str());
  sendStat.detach();
}

void sendWebSocketsStatusMsg(void)
{
  Serial.println();
#ifdef USE_ASYNC_WEBSERVER
  ws.textAll((char *)sendStatusMsg().c_str());
#else
  webSocket.broadcastTXT(sendStatusMsg().c_str());
#endif
  sendWSStat.detach();
}

bool writeStateFS()
{
  Serial.print(F("Saving cfg: "));
#if ARDUINOJSON_VERSION_MAJOR == 6
  StaticJsonDocument<256> jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();
#else
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
#endif
  json["state"] = (stateOn) ? 1 : 0;
  json["level"] = level;
#if ARDUINOJSON_VERSION_MAJOR == 6
  JsonObject colorRGB = json.createNestedObject("colorRGB");
#else
  JsonObject &colorRGB = json.createNestedObject("colorRGB");
#endif
  colorRGB["r"] = color.r;
  colorRGB["g"] = color.g;
  colorRGB["b"] = color.b;

  //      SPIFFS.remove("/lightstate.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
  File configFile = SPIFFS.open("/lightstate.json", "w");
  if (!configFile)
  {
    Serial.println("Failed!");
    return false;
  }
#if ARDUINOJSON_VERSION_MAJOR == 6
  serializeJson(json, Serial);
  serializeJson(json, configFile);
#else
  json.printTo(Serial);
  json.printTo(configFile);
#endif
  configFile.close();
  return true;
}

bool readStateFS()
{
  //if (resetsettings) { SPIFFS.begin(); SPIFFS.remove("/lightstate.json"); SPIFFS.format(); delay(1000);}
  if (SPIFFS.exists("/lightstate.json"))
  {
    Serial.print("Read cfg: ");
    File configFile = SPIFFS.open("/lightstate.json", "r");
    if (configFile)
    {
      size_t size = configFile.size(); // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
#if ARDUINOJSON_VERSION_MAJOR == 6
      StaticJsonDocument<200> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, buf.get());
      if (!error)
      {
        JsonObject json = jsonBuffer.as<JsonObject>();
        serializeJson(json, Serial);
#else
      DynamicJsonBuffer jsonBuffer(200);
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      if (json.success())
      {
        json.printTo(Serial);
#endif
        stateOn = static_cast<boolean>((int)json["state"]);
        level = json["level"];
#if ARDUINOJSON_VERSION_MAJOR == 6
        JsonObject colorRGB = json["colorRGB"];
#else
        JsonObject &colorRGB = json["colorRGB"];
#endif
        color.r = (uint8_t)colorRGB["r"];
        color.g = (uint8_t)colorRGB["g"];
        color.b = (uint8_t)colorRGB["b"];
        return true;
      }
      else
        Serial.println(F("Failed to parse JSON!"));
    }
    else
      Serial.println(F("Failed to open \"/lightstate.json\""));
  }
  else
    Serial.println(F("Couldn't find \"/lightstate.json\""));
  return false;
}

#ifdef USE_WIFIMANAGER
bool readConfigFS()
{
  //if (resetsettings) { SPIFFS.begin(); SPIFFS.remove("/config.json"); SPIFFS.format(); delay(1000);}
  if (SPIFFS.exists("/config.json"))
  {
    Serial.print(F("Read cfg: "));
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile)
    {
      size_t size = configFile.size(); // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
#if ARDUINOJSON_VERSION_MAJOR == 6
      StaticJsonDocument<200> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, buf.get());
      if (!error)
      {
        JsonObject json = jsonBuffer.as<JsonObject>();
        serializeJson(json, Serial);
#else
      DynamicJsonBuffer jsonBuffer(200);
      JsonObject &json = jsonBuffer.parseObject(buf.get());
      if (json.success())
      {
        json.printTo(Serial);
#endif
        strcpy(mqttCloudUsername, json["mqttCloudUsername"]);
        strcpy(mqttCloudPassword, json["mqttCloudPassword"]);
        return true;
      }
      else
        Serial.println(F("Failed to parse JSON!"));
    }
    else
      Serial.println(F("Failed to open \"/config.json\""));
  }
  else
    Serial.println(F("Couldn't find \"/config.json\""));
  return false;
}

bool writeConfigFS()
{
  Serial.print(F("Saving /config.json: "));
#if ARDUINOJSON_VERSION_MAJOR == 6
  StaticJsonDocument<200> jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();
#else
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
#endif
  json["mqttCloudUsername"] = mqttCloudUsername;
  json["mqttCloudPassword"] = mqttCloudPassword;
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile)
  {
    Serial.println(F("failed to open config file for writing"));
    return false;
  }
#if ARDUINOJSON_VERSION_MAJOR == 6
  serializeJson(json, Serial);
  serializeJson(json, configFile);
#else
  json.printTo(Serial);
  json.printTo(configFile);
#endif
  configFile.close();
  Serial.println(F("ok!"));
  return true;
}
#endif

#ifdef ARDUINO_ESP8266_RELEASE_2_4_2
bool writeCAFS()
{
  if (SPIFFS.exists("/ca.crt"))
    return true;
  Serial.print(F("Saving /ca.crt: "));
  File configFile = SPIFFS.open("/ca.crt", "w");
  if (!configFile)
  {
    Serial.println(F("failed to open config file for writing"));
    return false;
  }
  configFile.printf_P(cacert);
  configFile.close();
  Serial.println(F("ok!"));
  return true;
}
#endif

void setNeoPixels(uint8_t R = color.r, uint8_t G = color.g, uint8_t B = color.b)
{
  if (!stateOn)
#ifdef USE_NEOPIXELBUS
    strip.ClearTo(BLACK);
#else
    for (uint16_t i = 0; i < strip.numPixels(); i++)
      strip.setPixelColor(i, 0x000000);
#endif
  else
  {
#ifdef USE_NEOPIXELBUS
    RgbColor neoColor = colorGamma.Correct(RgbColor(color.r, color.g, color.b));
    strip.ClearTo(neoColor);
    strip.SetBrightness(level);
#else
    for (uint16_t i = 0; i < strip.numPixels(); i++)
      strip.setPixelColor(i, R, G, B);
    strip.setBrightness(level);
#endif
  }
#ifdef USE_NEOPIXELBUS
  //while(!strip.CanShow()) {yield();}
  strip.Show();
#else
  strip.show();
#endif
}

#ifdef USE_WIFIMANAGER
bool shouldSaveConfig = false;
void saveConfigCallback()
{
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}
#endif

void printFreeHeap()
{
  Serial.print(F("Free Heap: "));
  Serial.println(ESP.getFreeHeap());
}

bool processWebSocket(String payload)
{
#if ARDUINOJSON_VERSION_MAJOR == 6
  StaticJsonDocument<256> jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, payload);
  if (error)
  {
    Serial.print("parseObject() failed: ");
    Serial.println(error.c_str());
    return false;
  }
  JsonObject root = jsonBuffer.as<JsonObject>();
#else
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(payload);
  if (!root.success())
  {
    Serial.print("parseObject() failed: ");
    return false;
  }
#endif
  boolean updateData = false;

  if (root.containsKey("state"))
  {
    stateOn = root["state"].as<bool>();
    updateData = true;
  }

  if (root.containsKey("color"))
  {
#if ARDUINOJSON_VERSION_MAJOR == 6
    JsonObject colorRGB = root["color"];
#else
    JsonObject &colorRGB = root["color"];
#endif
    color.r = (uint8_t)colorRGB["r"];
    color.g = (uint8_t)colorRGB["g"];
    color.b = (uint8_t)colorRGB["b"];
    updateData = true;
  }

  if (root.containsKey("level"))
  {
    level = map((uint8_t)root["level"], 0, 255, c_MinBrightness, c_MaxBrightness);
    updateData = true;
  }

  if (updateData)
  {
    Serial.println(writeStateFS() ? F(" yay") : F(" meh"));
    setNeoPixels();
    sendStat.attach(2, sendMQTTStatusMsg);
  }

  return true;
}

#ifndef USE_ASYNC_WEBSERVER
void serverSafeLoop()
{
  server.handleClient();
  webSocket.loop();
}
#endif

void connect_artik(void)
{
  Serial.print(F("checking wifi "));
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(F("."));
    delay(1000);
  }
  Serial.println(F(" connected!"));
#if defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA2) or defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA3) or defined(ARDUINO_ESP8266_GIT_DESC)
  BearSSL::X509List cert(cacert);
  net.setTrustAnchors(&cert);
#endif
  Serial.print(F("Connecting MQTT "));
  while (!client.connect(mqttCloudClientName, mqttCloudUsername, mqttCloudPassword))
  {
    Serial.print(".");
    delay(100);
#ifndef USE_ASYNC_WEBSERVER
    serverSafeLoop();
#endif
  }
  Serial.println(F(" connected!"));
  client.subscribe(mqttCloudSubTopic);
  sendStat.attach(2, sendMQTTStatusMsg);
}

#ifdef USE_PUBSUB
void messageReceivedPubSub(char *topic, byte *payload, unsigned int length)
{
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] << "));
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#else
void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming: [" + topic + "] << " + payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6
  DynamicJsonDocument jsonBuffer(512);
  DeserializationError error = deserializeJson(jsonBuffer, payload);
  if (error)
  {
    Serial.print(F("parseObject() failed: "));
    Serial.println(error.c_str());
  }
  JsonObject root = jsonBuffer.as<JsonObject>();
#else
  DynamicJsonBuffer jsonBuffer(512);
  JsonObject &root = jsonBuffer.parseObject(payload);
  if (!root.success())
  {
    Serial.print(F("parseObject() failed: "));
  }
#endif
  if (root.containsKey("actions"))
  {
    const char *actions_name = root["actions"][0]["name"];
    if (strcmp(actions_name, "setOn") == 0)
    {
      stateOn = true;
    }
    if (strcmp(actions_name, "setOff") == 0)
    {
      stateOn = false;
    }
    if (strcmp(actions_name, "setLevel") == 0)
    {
      level = map((uint8_t)root["actions"][0]["parameters"]["level"], 0, 255, c_MinBrightness, c_MaxBrightness);
    }
    if (strcmp(actions_name, "setColorRGB") == 0)
    {
#if ARDUINOJSON_VERSION_MAJOR == 6
      JsonObject colorRGB = root["actions"][0]["parameters"]["colorRGB"];
#else
      JsonObject &colorRGB = root["actions"][0]["parameters"]["colorRGB"];
#endif
      color.b = (uint8_t)colorRGB["blue"];
      color.g = (uint8_t)colorRGB["green"];
      color.r = (uint8_t)colorRGB["red"];
    }
    Serial.println(writeStateFS() ? F(" yay") : F(" meh"));
    setNeoPixels();
    sendWSStat.attach(1, sendWebSocketsStatusMsg);
  }
}

#include "WSServerHelper.h"

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("---------------------------"));
  Serial.println(F("Starting SPIFFs"));
  if (SPIFFS.begin())
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      if (Serial)
        Serial.printf("FS File: %s, size: %dB\n", fileName.c_str(), fileSize);
    }

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    if (Serial)
    {
      Serial.printf("FS Usage: %d/%d bytes\n", fs_info.usedBytes, fs_info.totalBytes);
      Serial.println(F("SPIFFs started"));
      Serial.println(F("---------------------------"));
    }
  }
  if (readStateFS())
    Serial.println(F(" yay!"));
  else
    writeStateFS();

#ifdef USE_WIFIMANAGER
  if (readConfigFS())
    Serial.println(F(" yay!"));
  else
    writeConfigFS();

  WiFi.hostname(HOSTNAME);
#if defined(USE_WIFIMANAGER) and defined(USE_ASYNC_WEBSERVER)
  AsyncWiFiManager wm(&server, &dns);
  AsyncWiFiManagerParameter custom_user("user", "ARTIK USERNAME", mqttCloudUsername, 40);
  AsyncWiFiManagerParameter custom_pass("pass", "ARTIK PASSWORD", mqttCloudPassword, 40);
#elif (defined(USE_WIFIMANAGER) and !defined(USE_ASYNC_WEBSERVER))
  WiFiManager wm;
  WiFiManagerParameter custom_user("user", "ARTIK USERNAME", mqttCloudUsername, 40);
  WiFiManagerParameter custom_pass("pass", "ARTIK PASSWORD", mqttCloudPassword, 40);
#endif
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.addParameter(&custom_user);
  wm.addParameter(&custom_pass);
  if (!wm.autoConnect(HOSTNAME))
  {
    Serial.println(F("failed to connect and hit timeout"));
    delay(3000);
    ESP.restart();
    delay(5000);
  }
  Serial.println(F("connected!"));
  strcpy(mqttCloudUsername, custom_user.getValue());
  strcpy(mqttCloudPassword, custom_pass.getValue());
#else
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print(F("Connecting to "));
  Serial.print(F(WIFI_SSID));
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("ok!"));
#endif

  strcpy(mqttCloudPubTopic, "/v1.1/messages/");
  strcat(mqttCloudPubTopic, mqttCloudUsername);
  strcpy(mqttCloudSubTopic, "/v1.1/actions/");
  strcat(mqttCloudSubTopic, mqttCloudUsername);

#ifdef USE_WIFIMANAGER
  if (shouldSaveConfig)
  {
    writeConfigFS();
    shouldSaveConfig = false;
  }
#endif

  Serial.print(F("Local IP:   "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Gateway IP: "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(F("SubNet IP:  "));
  Serial.println(WiFi.subnetMask());

#ifdef USE_NEOPIXELBUS
  strip.Begin();
  strip.ClearTo(BLUE);
  strip.Show();
#else
  strip.begin();
  setNeoPixels(0, 0, 64);
#endif

  printFreeHeap();

#ifdef ARDUINO_ESP8266_RELEASE_2_4_2
  writeCAFS();
  File ca = SPIFFS.open("/ca.crt", "r");
  net.loadCACert(ca);
#endif
#if defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA2) or defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA3) or defined(ARDUINO_ESP8266_GIT_DESC)
  NTPConnect();
  //BearSSL::X509List cert(cacert);
  //net.setTrustAnchors(&cert);
  //net.setInsecure();
#endif
  printFreeHeap();
#ifdef USE_PUBSUB
  client.setServer(mqttCloudServer, mqttCloudPort);
  client.setCallback(messageReceivedPubSub);
#else
  client.begin(mqttCloudServer, mqttCloudPort, net);
  client.onMessage(messageReceived);
#endif

#ifndef USE_ASYNC_WEBSERVER
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.print(F(" HTTP server starting "));
  server.on("/", HTTP_GET, [&] {
    server.sendHeader("Content-Encoding", "gzip", true);
    server.send_P(200, PSTR("text/html"), index_htm_gz, index_htm_gz_len);
  });
  server.on("/status", HTTP_GET, [&] {
    server.send(200, "application/json", sendStatusMsg());
  });
  server.on("/version", HTTP_GET, [&] {
    server.send(200, "application/json", SKETCH_VERSION);
  });
  server.on("/restart", HTTP_GET, [&]() {
    Serial.println(F("/restart"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", "<META http-equiv='refresh' content='15;URL=/'><body align=center><H4>Restarting...</H4></body>");
    shouldReboot = true;
  });
  server.on("/reset_wifi", HTTP_GET, [&]() {
    Serial.println(F("/reset_wlan"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", "<META http-equiv='refresh' content='15;URL=/'><body align=center><H4>Resetting WLAN and restarting...</H4></body>");
#if defined(USE_WIFIMANAGER) and defined(USE_ASYNC_WEBSERVER)
    AsyncWiFiManager wm(&server, &dns);
    wm.resetSettings();
#elif defined(USE_WIFIMANAGER) and !defined(USE_ASYNC_WEBSERVER)
      WiFiManager wm;
      wm.resetSettings();
#endif
    shouldReboot = true;
  });
  server.on("/restart", []() {
    Serial.println(F("/restart"));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "restarting...");
    shouldReboot = true;
  });
  server.onNotFound(handleNotFound);
  httpUpdater.setup(&server);
#else
  Serial.print(F("Async HTTP server starting "));
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.on("/upload", HTTP_GET, handleUpload);
  Serial.print(F("."));
  server.on("/upload", HTTP_POST, processUploadReply, processUpload);
  Serial.print(F("."));
  server.on("/update", HTTP_GET, handleUpdate);
  Serial.print(F("."));
  server.on("/update", HTTP_POST, processUpdateReply, processUpdate);
  Serial.print(F("."));
  server.on("/", HTTP_GET, handleRoot);
  Serial.print(F("."));
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", sendStatusMsg());
  });
  server.on("/version", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", SKETCH_VERSION);
  });
  Serial.print(F("."));
  server.on("/reset_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println(F("/reset_wlan"));
    request->send(200, "text/html", "<META http-equiv='refresh' content='15;URL=/'><body align=center><H4>Resetting WLAN and restarting...</H4></body>");
#if defined(USE_WIFIMANAGER) and defined(USE_ASYNC_WEBSERVER)
    AsyncWiFiManager wm(&server, &dns);
    wm.resetSettings();
#elif defined(USE_WIFIMANAGER) and !defined(USE_ASYNC_WEBSERVER)
      WiFiManager wm;
      wm.resetSettings();
#endif
    shouldReboot = true;
  });
  Serial.print(F("."));
  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println(F("/restart"));
    request->send(200, "text/html", "<META http-equiv='refresh' content='15;URL=/'><body align=center><H4>Restarting...</H4></body>");
    shouldReboot = true;
  });
  Serial.print(F("."));
  server.onNotFound(handleNotFound);
  Serial.print(F("."));
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
#endif
  server.begin();
  Serial.print(F(" done!\n"));
  printFreeHeap();

  connect_artik();
  printFreeHeap();

  setNeoPixels();
  Serial.println(F("NeoPixel Set!"));
  printFreeHeap();
  pFH.attach(10, printFreeHeap);
}

void loop()
{
#if defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA2) or defined(ARDUINO_ESP8266_RELEASE_2_5_0_BETA3) or defined(ARDUINO_ESP8266_GIT_DESC)
  now = time(nullptr);
#endif
  if (!client.connected())
    connect_artik();
  else
    client.loop();

#ifdef USE_NEOPIXELBUS
  if (strip.CanShow())
    strip.Show();
#else
  strip.show();
#endif

#ifndef USE_ASYNC_WEBSERVER
  serverSafeLoop();
#endif

  if (shouldReboot)
  {
    Serial.println(F("Rebooting..."));
    delay(100);
    ESP.restart();
  }
}
