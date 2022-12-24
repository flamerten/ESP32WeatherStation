# ESP32WeatherStation

This project uses the NEA Weather Forecast API provided by  api.data.gov.sg to visualize the weather forecast for the next 24 hours. Open Weather Map is also used to check the temperature, which is displayed on an OLED. Both APIs are updated every 30mins.

WS2812B LEDs are used to visualize the weather through the changing of colours.

!["Map Visualization"](src/non_lit.jpg "Map Visualization")

## Update

On 14/11/2022 the API was fixed

As of 13/11/2022 the API seems to have broken, with the response being
   
     {
        "items": [
          {}
        ],
        "api_info": {
          "status": "healthy"
        }
      }
  
My Program shows this by showing red leds.
