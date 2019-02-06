#include <pgmspace.h>

#ifndef PIO_PLATFORM
  #define USE_PUBSUB             //uncomment if you want to use PubSubClient
  #define USE_WIFIMANAGER        //uncomment if you want to use WiFiManager
  #define USE_ASYNC_WEBSERVER    //comment if you want to use inbuilt web-server
#endif

#define SECRET

#ifndef USE_WIFIMANAGER
  #define WIFI_SSID "WIFI_SSID"
  #define WIFI_PASS "WIFI_PASSWORD"
#endif
char mqttCloudUsername[40] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
char mqttCloudPassword[40] = "YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY";

int8_t TIME_ZONE = -5; //NYC(USA): -5 UTC
//#define USE_SUMMER_TIME_DST  //uncomment to use DST
