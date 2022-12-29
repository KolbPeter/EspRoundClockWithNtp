#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <Adafruit_BMP280.h>

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Wire.h>
#include <TimeLib.h>

#define BMP_SCK (15)
#define BMP_MISO (16)
#define BMP_MOSI (2)
#define BMP_CS (4)

#define LED_COUNT 60
#define LED_PIN 22

String PARAM_INPUT_1 = "light";
int maxLight = 15;
RgbColor black(0, 0, 0);
RgbColor zero(1,1,0);
RgbColor quarters(1,0,1);
RgbColor smalls(0,1,1);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

String collect_Info();
void getStats();
void getRequestHandler();
void setLeds(int red, int green, int blue, bool backlight);
void setLocalTime();

void setup();
void loop();

WebServer Server;
AutoConnect Portal(Server);
Adafruit_BMP280 sensor(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK);
int altitude = 235;
const float ALTMOD = 11.25; // 11,25Pa/méter hozzávetőlegesen 1000 méterig

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(LED_COUNT, LED_PIN);
RgbColor color(0, 0, 0);

/// @brief Creates a string that contains sensor data and current time.
/// @return String with sensor data and the current time.
String collect_Info()
{
  String retval = (String)sensor.readTemperature() + ";";
  retval += (String)((sensor.readPressure() + (ALTMOD * (float)altitude)) / 100) + ";";

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    return retval;
  }

  char timeinfoStringBuff[50]; // 50 chars should be enough
  strftime(timeinfoStringBuff, sizeof(timeinfoStringBuff), "%Y/%B/%d %H:%M:%S", &timeinfo);
  retval += timeinfoStringBuff;
  retval += ";";

  return retval;
}

/// @brief Method to call when the server get a request.
void getStats()
{
  Server.send(200, "text/plain", collect_Info());
}

/// @brief Method to handle server requests.
void getRequestHandler()
{
  if (Server.hasArg(PARAM_INPUT_1) == true)
  {
    maxLight = Server.arg(PARAM_INPUT_1).toInt();
  }

  Server.send(200, "text/plain", collect_Info());

  return;
}

/// @brief Method to set led strip values then show the led strip.
/// @param hour The hours to display on the clock.
/// @param min The minutes to display on the clock.
/// @param sec The seconds to display on the clock.
/// @param backlight Value indicating if backlight is neede for the clock.
void setLeds(int hour, int min, int sec, bool backlight)
{
  int roundClockHour = hour;
  if (roundClockHour >= 12)
  {
    roundClockHour = roundClockHour - 12;
  };
  roundClockHour = roundClockHour * 5;
  roundClockHour = roundClockHour + (min / 12);

  strip.ClearTo(black);

  if(backlight)
  {
    strip.SetPixelColor(0, zero);
    for (int i = 5; i < 60; i+=5)
    {
      if(i % 15 == 0)
      {
        strip.SetPixelColor(i, quarters);
      }else{
        strip.SetPixelColor(i, smalls);
      }
    }
  }

  RgbColor color = strip.GetPixelColor(roundClockHour);
  color.R = maxLight;
  strip.SetPixelColor(roundClockHour, color);

  color = strip.GetPixelColor(min);
  color.G = maxLight;
  strip.SetPixelColor(min, color);

  color = strip.GetPixelColor(sec);
  color.B = maxLight;
  strip.SetPixelColor(sec, color);

  strip.Show();
}

/// @brief Sets local time from an NTP server.
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
}

/// @brief Setup method.
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

  Server.on("/getStats", getStats);
  Server.on("/", HTTP_GET, getRequestHandler);

  strip.Begin();

  // init and get the time
  setLocalTime();
}

/// @brief Loop method.
void loop()
{
  time_t last = second(now());
  struct tm timeinfo;
  Portal.handleClient();

  int i = 0;
  do
  {
    if (second(now()) != last)
    {
      getLocalTime(&timeinfo);
      setLeds(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, false);
      last = second(now());
      i = 0;

      if(timeinfo.tm_hour == 3 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 0 )
      {
        setLocalTime();
      }
    }

    setLeds(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, i % 20 == 0);    
    i++;
  } while (true);
  
}
