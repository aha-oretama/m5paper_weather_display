#pragma once

#include <string>
#include <ArduinoJson.h>

enum {
  WEATHER_NOT_SET = 0,
  CLOUDY,
  SUNNY,
  RAINY,
  SNOW,
  SUNNY_AND_CLOUDY,
  RAINY_AND_CLOUDY,
};

class WeatherForecast
{
private:
    String endpoint;
    String region;
    String temperatureRegion;
    bool temperature_exists;
    String max_temperature;
    String min_temperature;
    String weather;
    String rain_fall_chance_00_06;
    String rain_fall_chance_06_12;
    String rain_fall_chance_12_18;
    String rain_fall_chance_18_24;
    String tomorrow_weather;
    String tomorrow_rain_fall_chance_00_06;
    String tomorrow_rain_fall_chance_06_12;

    String createJson(String json_string);
    bool getWeatherForecast(DynamicJsonDocument &doc);

public:
    WeatherForecast();
    ~WeatherForecast() = default;
    bool downloadWeatherForecast(void);
    String getWeather(void){ return weather; };
    String getTomorrowWeather(void){ return tomorrow_weather; };
    int getWeatherEnum(String weather);
    bool temperatureExists(void){ return temperature_exists; }
    String getMaxTemperature(void){ return max_temperature; };
    String getMinTemperature(void){ return min_temperature; };
    String getRainFallChance00_06(void){ return rain_fall_chance_00_06; };
    String getRainFallChance06_12(void){ return rain_fall_chance_06_12; };
    String getRainFallChance12_18(void){ return rain_fall_chance_12_18; };
    String getRainFallChance18_24(void){ return rain_fall_chance_18_24; };
    String getTomorrowRainFallChance00_06(void){ return tomorrow_rain_fall_chance_00_06; };
    String getTomorrowRainFallChance06_12(void){ return tomorrow_rain_fall_chance_06_12; };
};
