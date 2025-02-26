#define LGFX_M5PAPER
#include <M5EPD.h>
#include <LovyanGFX.hpp>
#include <map>
#include "time.h"
#include "src/wifi_connection.hpp"
#include "src/weather_forecast.hpp"

LGFX gfx;
LGFX_Sprite sense_temp_sp(&gfx);
LGFX_Sprite sense_humi_sp(&gfx);
LGFX_Sprite rfc_sp(&gfx);
LGFX_Sprite temp_sp(&gfx);
LGFX_Sprite tomorrow_weather_sp(&gfx);


WiFiConnection wifi_connection;
WeatherForecast weather_forecast;

// 時刻設定
const char* ntpServer = "ntp.nict.jp";
const long  gmtOffset_sec = 3600 * 9;
const int   daylightOffset_sec = 0;

int w;
int h;
int totalDelay = 0;

//weather
std::map<int, String> weather_icon_file_map;

void setupWeatherIcon(void)
{
  weather_icon_file_map[CLOUDY] = "/cloudy.jpeg";
  weather_icon_file_map[SUNNY] = "/sunny.jpeg";
  weather_icon_file_map[RAINY] = "/rainly.jpeg";
  weather_icon_file_map[SUNNY_AND_CLOUDY] = "/sunnyandcloudy.jpeg";
  weather_icon_file_map[RAINY_AND_CLOUDY] = "/rainyandcloudy.jpeg";
}

void drawDate(void)
{
  struct tm timeInfo;
  char   timeString[30];

  getLocalTime(&timeInfo);
  sprintf(timeString, "%02d.%02d %02d:%02d"
          , timeInfo.tm_mon + 1
          , timeInfo.tm_mday
          , timeInfo.tm_hour
          , timeInfo.tm_min
         );

  gfx.startWrite();
  gfx.setCursor(10, 10);
  gfx.println(timeString);
  gfx.endWrite();
  gfx.display();
}

void setup(void)
{
  setupWeatherIcon();

  M5.begin();
  //  M5.begin(true, true, true, true, false, false);//custmized

  M5.SHT30.Begin();
  M5.RTC.begin();

  gfx.init();
  gfx.setRotation(1);

  w = gfx.width();
  h = gfx.height();

  gfx.setBrightness(50);
  gfx.setEpdMode(epd_mode_t::epd_quality);  // 高品質更新、白黒反転が一瞬起きる

  //gfx.setFont(&fonts::lgfxJapanGothic_40);
  gfx.setFont(&fonts::Font8);
  gfx.setTextColor(TFT_BLACK, TFT_WHITE);
  gfx.setTextSize(0.85);

  sense_temp_sp.setColorDepth(4);
  sense_temp_sp.createSprite(120, 80);
  sense_temp_sp.setFont(&fonts::Font8);

  sense_humi_sp.setColorDepth(4);
  sense_humi_sp.createSprite(120, 80);
  sense_humi_sp.setFont(&fonts::Font8);

  rfc_sp.setColorDepth(4);
  rfc_sp.createSprite(530, 110);
  rfc_sp.setFont(&fonts::lgfxJapanGothic_40);

  temp_sp.setColorDepth(4);
  temp_sp.createSprite(530, 90);
  temp_sp.setFont(&fonts::lgfxJapanGothic_40);

  tomorrow_weather_sp.setColorDepth(4);
  tomorrow_weather_sp.createSprite(80, 40);
  tomorrow_weather_sp.setFont(&fonts::lgfxJapanGothic_40);

  delay(1000);

  drawThermometerIcon();
  drawHumidityIcon();
  drawSenseTempAndHumid();

  wifi_connection.setupWiFi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  drawDate();

  if (weather_forecast.downloadWeatherForecast()) {
    drawWeather();
    drawRainFallChance();
    drawTemperature();
    drawTomorrow();
    drawTomorrowWeather();
    drawTomorrowRainFallChance();
  }
  wifi_connection.downWiFi();
}

void drawWeather(void)
{
  String weather = weather_forecast.getWeather();
  int weather_enum = weather_forecast.getWeatherEnum(weather);
  gfx.startWrite();
  gfx.drawJpgFile(SD, weather_icon_file_map[weather_enum].c_str(), 40, 100);
  gfx.endWrite();
  gfx.display();
}

void drawTomorrow(void)
{
  tomorrow_weather_sp.clear(TFT_WHITE);
  tomorrow_weather_sp.setTextColor(TFT_BLACK);
  tomorrow_weather_sp.setTextSize(0.65);
  tomorrow_weather_sp.drawString("明日", 0, 0);
  tomorrow_weather_sp.pushSprite(470, 365);
}

void drawTomorrowWeather(void)
{
  String weather = weather_forecast.getTomorrowWeather();
  int weather_enum = weather_forecast.getWeatherEnum(weather);
  gfx.startWrite();
  gfx.drawJpgFile(SD, weather_icon_file_map[weather_enum].c_str(), 470, 390, 0, 0, 0, 0, 0.3);
  gfx.endWrite();
  gfx.display();
}

void drawThermometerIcon(void)
{
  gfx.startWrite();
  gfx.drawJpgFile(SD, "/thermometer.jpg", 470, 35);
  gfx.endWrite();
  gfx.display();
}

void drawHumidityIcon(void)
{
  gfx.startWrite();
  gfx.drawJpgFile(SD, "/humidity.jpg", 710, 35);
  gfx.endWrite();
  gfx.display();
}

void drawSenseTempAndHumid(void)
{
  M5.SHT30.UpdateData();
  float temp = M5.SHT30.GetTemperature();
  float humi = M5.SHT30.GetRelHumidity();
  Serial.printf("Temperatura: %d*C  Humedad: %d%%\r\n", (int)temp, (int)humi);
  sense_temp_sp.clear(TFT_WHITE);
  sense_temp_sp.setTextColor(TFT_BLACK);
  sense_temp_sp.drawNumber((int)temp, 0, 0);
  sense_temp_sp.pushSprite(570, 40);

  sense_humi_sp.clear(TFT_WHITE);
  sense_humi_sp.setTextColor(TFT_BLACK);
  sense_humi_sp.drawNumber((int)humi, 0, 0);
  sense_humi_sp.pushSprite(570 + 250, 40);
}

void drawRainFallChance(void)
{
  rfc_sp.clear(TFT_WHITE);
  rfc_sp.setTextColor(TFT_BLACK);

  String rfc00_06 = weather_forecast.getRainFallChance00_06() + "%";
  String rfc06_12 = weather_forecast.getRainFallChance06_12() + "%";
  String rfc12_18 = weather_forecast.getRainFallChance12_18() + "%";
  String rfc18_24 = weather_forecast.getRainFallChance18_24() + "%";

  rfc_sp.setTextSize(0.8);
  rfc_sp.drawString("00-06", 120 * 0, 0);
  rfc_sp.drawString("06-12", 120 * 1, 0);
  rfc_sp.drawString("12-18", 120 * 2, 0);
  rfc_sp.drawString("18-24", 120 * 3, 0);
  rfc_sp.setTextSize(1.3);
  rfc_sp.drawString(rfc00_06.c_str(), 120 * 0, 50);
  rfc_sp.drawString(rfc06_12.c_str(), 120 * 1, 50);
  rfc_sp.drawString(rfc12_18.c_str(), 120 * 2, 50);
  rfc_sp.drawString(rfc18_24.c_str(), 120 * 3, 50);
  rfc_sp.pushSprite(470, 160);
}

void drawTomorrowRainFallChance(void)
{
  rfc_sp.clear(TFT_WHITE);
  rfc_sp.setTextColor(TFT_BLACK);

  String rfc00_06 = weather_forecast.getTomorrowRainFallChance00_06() + "%";
  String rfc06_12 = weather_forecast.getTomorrowRainFallChance06_12() + "%";

  rfc_sp.setTextSize(0.8);
  rfc_sp.drawString("00-06", 120 * 0, 0);
  rfc_sp.drawString("06-12", 120 * 1, 0);
  rfc_sp.setTextSize(1.3);
  rfc_sp.drawString(rfc00_06.c_str(), 120 * 0, 50);
  rfc_sp.drawString(rfc06_12.c_str(), 120 * 1, 50);
  rfc_sp.pushSprite(670, 390);
}

void updateDrawTemperature(void)
{
  if(weather_forecast.temperatureExists()) {
    drawTemperature();
  }
}

void drawTemperature(void)
{
  temp_sp.clear(TFT_WHITE);
  temp_sp.setTextColor(TFT_BLACK);

  String max_temp = weather_forecast.getMaxTemperature() + "℃";
  String min_temp = weather_forecast.getMinTemperature() + "℃";

  temp_sp.setTextSize(0.65);
  temp_sp.drawString("最高", 0, 0);
  temp_sp.drawString("最低", 240, 0);
  temp_sp.setTextSize(1.4);
  temp_sp.drawString(max_temp.c_str(), 0, 25);
  temp_sp.drawString(min_temp.c_str(), 240, 25);
  temp_sp.pushSprite(470, 275);
}

void loop(void)
{
  totalDelay += 60 * 1000;
  delay(60 * 1000);
  drawDate();
  drawSenseTempAndHumid();

  // １0分に１回天気情報の更新
  if (totalDelay >= 10 * 60 * 1000) {
    totalDelay = 0;

    if (wifi_connection.setupWiFi() && weather_forecast.downloadWeatherForecast()) {
      drawWeather();
      drawRainFallChance();
      updateDrawTemperature();
      drawTomorrow();
      drawTomorrowWeather();
      drawTomorrowRainFallChance();
      wifi_connection.downWiFi();
    }else {
      ESP.restart();
    }
  }
  M5.update();
}
