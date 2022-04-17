#include <WiFi.h>
#include <HTTPClient.h>
#include "weather_forecast.hpp"

WeatherForecast::WeatherForecast(void)
{
    this->endpoint = "https://www.jma.go.jp/bosai/forecast/data/forecast/130000.json";
    this->region = "東京地方";
    this->temperatureRegion = "東京";
}

bool WeatherForecast::getWeatherForecast(DynamicJsonDocument &doc)
{
    Serial.println("getWeatherForecast");//debug
    bool ret = true;
    if((WiFi.status() == WL_CONNECTED)){
        HTTPClient http;
        http.begin(this->endpoint.c_str());
        int http_code = http.GET();
        if (http_code > 0) {
            deserializeJson(doc, http.getString());
        } else {
            Serial.println("Error on HTTP request");
            ret = false;
        }
        http.end();
    }
    return ret;
}

bool WeatherForecast::downloadWeatherForecast(void)
{
    Serial.println("downloadWeatherForecast");//debug
    if(!WiFi.isConnected()){
        this->is_downloaded_weather = false;
        return false;
    }

    DynamicJsonDocument weather_info(20000);

    if(!getWeatherForecast(weather_info)){
        this->is_downloaded_weather = false;
        return false;
    }

    // 二日間、週間天気の並び順
    JsonArray twoDayWeathers = weather_info[0]["timeSeries"].as<JsonArray>();
    // 天気、降水確率、気温の順
    JsonArray weatherAreas = twoDayWeathers[0]["areas"].as<JsonArray>();
    JsonArray chanceOfRainAreas = twoDayWeathers[1]["areas"].as<JsonArray>();
    JsonArray temperatureAreas = twoDayWeathers[2]["areas"].as<JsonArray>();

    // 天気
    for (JsonVariant areas : weatherAreas) {
        if(areas["area"]["name"] == this->region){
            // 今日、明日、明後日の順
            String w = areas["weathers"][0];
            this->weather = w;
            String tw = areas["weathers"][1];
            this->tomorrow_weather = tw;
        }
    }

    // 降水確率
    for (JsonVariant chanceOfRain : chanceOfRainAreas) {
        if(chanceOfRain["area"]["name"] == this->region){
            // 今日の0時ではなく、現在時刻から明日の18~24まで
            // そのため現在時刻によってpops数が変化する
            int size = chanceOfRain["pops"].size();
            if (size - 8 >= 0) {
                String rain = chanceOfRain["pops"][size-8];
                this->rain_fall_chance_00_06 = rain;
            }else{
                this->rain_fall_chance_00_06 = "--";
            }
            if (size - 8 + 1 >= 0) {
                String rain = chanceOfRain["pops"][size-8+1];
                this->rain_fall_chance_06_12 = rain;
            }else{
                this->rain_fall_chance_06_12 = "--";
            }
            if (size - 8 + 2 >= 0) {
                String rain = chanceOfRain["pops"][size-8+2];
                this->rain_fall_chance_12_18 = rain;
            }else{
                this->rain_fall_chance_12_18 = "--";
            }
            if (size - 8 + 3 >= 0) {
                String rain = chanceOfRain["pops"][size-8+3];
                this->rain_fall_chance_18_24 = rain;
            }else{
                this->rain_fall_chance_18_24 = "--";
            }
            if (size - 8 + 4 >= 0) {
                String rain = chanceOfRain["pops"][size-8+4];
                this->tomorrow_rain_fall_chance_00_06 = rain;
            }else{
                this->tomorrow_rain_fall_chance_00_06 = "--";
            }
            if (size - 8 + 5 >= 0) {
                String rain = chanceOfRain["pops"][size-8+5];
                this->tomorrow_rain_fall_chance_06_12 = rain;
            }else{
                this->tomorrow_rain_fall_chance_06_12 = "--";
            }
        }
    }

    // 気温
    for (JsonVariant temperature : temperatureAreas) {
        if(temperature["area"]["name"] == this->temperatureRegion){
           // 今日の朝の最低、日中の最高、明日の朝の最低、日中の最高の順
           String min_t = temperature["temps"][0];
           this->min_temperature = min_t;
           String max_t = temperature["temps"][1];
           this->max_temperature = max_t;
        }
    }

    this->is_downloaded_weather = true;
    return true;
}

int WeatherForecast::getWeatherEnum(String weather)
{
    if(weather.indexOf("雨") != -1){
        if(weather.indexOf("くもり") != -1){
            return RAINY_AND_CLOUDY;
        }else{
            return RAINY;
        }
    }else if(weather.indexOf("晴") != -1){
        if(weather.indexOf("くもり") != -1){
            return SUNNY_AND_CLOUDY;
        }else{
            return SUNNY;
        }
    }else if(weather.indexOf("雪") != -1){
        return SNOW;
    }else if(weather.indexOf("くもり") != -1){
        return CLOUDY;
    }
    return WEATHER_NOT_SET;
}
