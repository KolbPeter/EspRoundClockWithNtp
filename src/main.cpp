#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <Adafruit_BMP280.h>

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Wire.h>
#include <TimeLib.h>

#include "time.h"
#include "index.h"

#define BMP_SCK (15)
#define BMP_MISO (16)
#define BMP_MOSI (2)
#define BMP_CS (4)

#define LED_COUNT 60
#define LED_PIN 22
#define BASE_YEAR 2000
String PARAM_INPUT_1 = "hours";
String PARAM_INPUT_2 = "mins";
String PARAM_INPUT_3 = "secs";
String PARAM_INPUT_4 = "light";
int maxLight = 150;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void rootPage();
String collect_Info();
void getStats();
void getRequestHandler();
void setLeds();
void setLeds(int red, int green, int blue);
void setLocalTime();

void setup();
void loop();

WebServer Server;
AutoConnect Portal(Server);
Adafruit_BMP280 sensor(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);
int altitude = 235;
const float ALTMOD = 11.25; //11,25Pa/méter hozzávetőlegesen 1000 méterig

time_t last = second(now());
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(LED_COUNT, LED_PIN);
RgbColor color(0, 0, 0);

void rootPage()
{
  String s = MAIN_page;
  Server.send(200, "text/html", s);
}

String collect_Info(){
  String retval = (String)sensor.readTemperature()+";";
  retval += (String)((sensor.readPressure() + (ALTMOD * (float)altitude)) / 100)+";";
  return retval;
}

void getStats()
{
  Server.send(200, "text/plane", collect_Info());
}

void getRequestHandler()
{
  if (Server.hasArg(PARAM_INPUT_1) == true){
    setTime(
      Server.arg(PARAM_INPUT_1).toInt(),
      minute(now()),
      second(now()),
      1,1,1);
  }
  if(Server.hasArg(PARAM_INPUT_2) == true){
    setTime(
      hour(now()),
      Server.arg(PARAM_INPUT_2).toInt(),
      second(now()),
      1,1,1);
  }
  if(Server.hasArg(PARAM_INPUT_3) == true){
    setTime(
      hour(now()),
      minute(now()),
      Server.arg(PARAM_INPUT_3).toInt(),
      1,1,1);
  }
  if(Server.hasArg(PARAM_INPUT_4) == true){
    maxLight = Server.arg(PARAM_INPUT_4).toInt();
  }
  if(
    Server.hasArg(PARAM_INPUT_1) == false
    && Server.hasArg(PARAM_INPUT_2) == false
    && Server.hasArg(PARAM_INPUT_3) == false
    && Server.hasArg(PARAM_INPUT_4) == false){
      Server.send(200, "text/plain", "No data received");
    return;
  }

  setLeds();
  rootPage();
}

void setLeds(){
  setLeds(hour(now()),minute(now()),second(now()));
}

void setLeds(int hour, int min, int sec){
  int roundClockHour = hour;
  if (roundClockHour >= 12){ roundClockHour = roundClockHour - 12; };
  roundClockHour = roundClockHour * 5;
  roundClockHour = roundClockHour + (minute(now()) / 12);

  for (int i=0;i<LED_COUNT;i++){
    if (i==roundClockHour){ color.R = maxLight; }else{ color.R = 0; };
    if (i==min){ color.G = maxLight; }else{ color.G = 0; };
    if (i==sec){ color.B = maxLight; }else{ color.B = 0; };
    strip.SetPixelColor(i, color);
  }
  strip.Show();
}

void setLocalTime()
{
  struct tm timeinfo;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  while (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time. Retrying in 3 seconds.");
    delay(3000);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year);
}

void setup()
{
  Serial.begin(9600);
  delay(100);
  sensor.begin();
  sensor.setSampling(
      Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
      Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
      Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
      Adafruit_BMP280::FILTER_X16,      /* Filtering. */
      Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  delay(1000);
  Serial.println("Adafruit connected!");
  if (Portal.begin())
  {
    Serial.println("HTTP server:" + WiFi.localIP().toString());
    char __SSID[WiFi.SSID().length() + 1];
    char __PASS[WiFi.psk().length() + 1];
    (WiFi.SSID()).toCharArray(__SSID, sizeof(__SSID));
    (WiFi.psk()).toCharArray(__PASS, sizeof(__PASS));
  }
  Server.on("/", rootPage);
  Server.on("/getStats", getStats);
  Server.on("/get", HTTP_GET, getRequestHandler);

  strip.Begin();

  //init and get the time
  setLocalTime();
}

void loop()
{
  Portal.handleClient();
  if (second(now())!=last){
    setLeds(hour(now()),minute(now()),second(now()));
    last=second(now());
  }
}
