#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "secrets.h"
#include "bitmaps.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

//OpenWeatherMap - Temp - get api key from https://openweathermap.org/ 
String openWeatherMapApiKey = openWeatherMap_API_KEY;
String city = "Singapore";
String countryCode = "SG";
String OpenWeatherServer = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
float temperature; String k_temperature; //Conversion from Kelvin to celcius

//Neopixel - Data Visualization
#define LED_PIN     5
#define LED_COUNT  15
#define BRIGHTNESS 25
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t central_pixel = 6;
uint8_t south_pixel = 3;
uint8_t north_pixel = 9;
uint8_t east_pixel = 12;
uint8_t west_pixel = 0;

//Partly Cloudy, Thundery Showers, Cloudy, Light Rain, Passing Showers, Moderate Rain, fair, Fair & Warm
//Yellow - Fair //White - Cloudy //Light blue - Rain //Dark Blue - Showers
//Check for these strings in the API recieved
String Fair = "Fair"; String Cloudy = "Cloudy"; String Rain = "Rain"; String Showers = "Showers";


//Weather Data Collection
typedef struct struct_message{
  uint8_t west[3];
  uint8_t east[3];
  uint8_t central[3];
  uint8_t south[3];
  uint8_t north[3];
};
struct_message WeatherData;

//TFT OLED SCREEN
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 
#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3c
#define TEXT_SIZE    2
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// HTTP Requests
// Check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
unsigned long timerDelay = 30 * 60 * 1000; //30Mins
String jsonBuffer;
JSONVar myObject;

//Misc
bool first_loop = true;
void(* resetFunc) (void) = 0; //declare reset function @ address 0, reset if unable to connect to Wifi for 10s


String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(serverName); //No need the wifi client for nea api, not too sure why its there
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    resetFunc(); //Reset
  }
  // Free resources
  http.end();

  return payload;
}

void PrintWeatherData(){
  Serial.println("West");
  for(int i = 0; i <3; i++){
    Serial.print(WeatherData.west[i]); Serial.print(" ");
  }
  Serial.println();

  Serial.println("East");
  for(int i = 0; i <3; i++){
    Serial.print(WeatherData.east[i]); Serial.print(" ");
  }
  Serial.println();

  Serial.println("Central");
  for(int i = 0; i <3; i++){
    Serial.print(WeatherData.central[i]); Serial.print(" ");
  }
  Serial.println();

  Serial.println("South");
  for(int i = 0; i <3; i++){
    Serial.print(WeatherData.south[i]); Serial.print(" ");
  }
  Serial.println();

  Serial.println("North");
  for(int i = 0; i <3; i++){
    Serial.print(WeatherData.north[i]); Serial.print(" ");
  }
  Serial.println();

}

void InitNeopixel(){
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
}

void UpdateNeoxpixel(){
  for( int i = central_pixel; i < central_pixel+3; i++){
      LightUpPixel(i,WeatherData.central[i - central_pixel],true);
  }

  for( int i = south_pixel; i < south_pixel+3; i++){
    LightUpPixel(i,WeatherData.south[i - south_pixel],true);
  }

  for( int i = north_pixel; i < north_pixel+3; i++){
    LightUpPixel(i,WeatherData.north[i - north_pixel],true);
    
  }

  for( int i = east_pixel; i < east_pixel+3; i++){
    LightUpPixel(i,WeatherData.east[i - east_pixel],true);
  }

  for( int i = west_pixel; i < west_pixel+3; i++){
    LightUpPixel(i,WeatherData.west[i - west_pixel],true);
  }  
  
}

void BlinkFirstPixels(){
  for(int i = 0; i < 15; i = i+ 3){
    strip.setPixelColor(i,strip.Color(0,0,0));
  }
  strip.show();

  if(WiFi.status() == WL_CONNECTED){
    DisplayDefault();
  }
  else DisplayNoWifi();

  delay(1000);

  LightUpPixel(central_pixel,WeatherData.central[0],false);
  LightUpPixel(north_pixel,WeatherData.north[0],false);
  LightUpPixel(east_pixel,WeatherData.east[0],false);
  LightUpPixel(west_pixel,WeatherData.west[0],false);
  LightUpPixel(south_pixel,WeatherData.south[0],false);
  strip.show();

  DisplayTemp();

  delay(1000);

  
  
}

void LightUpPixel(int pixel_no, int weather, bool animate){
  uint8_t r,g,b;
  
  switch(weather){
    case 0:
      r = 100; g = 100; b = 0; //yellow
      break;
    case 1:
      r = 100; g = 100; b = 100; //white
      break;
    case 2:
      r = 0; g = 100; b = 100; //light blue
      break;
    case 3:
      r = 0; g = 0; b = 200; //blue
      break;
  }


  strip.setPixelColor(pixel_no,strip.Color(r,g,b));
  strip.show();
  if (animate)  delay(100); //Add some animation during update
  
}

void init_TFT(){
  bool res = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  if(!res){
    Serial.println("Unable to init OLED TFT");
    while(1); 
  }
  
  
  display.display();
  delay(1000);
  
  display.clearDisplay();
  display.setTextSize(TEXT_SIZE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(0, 0);
  display.invertDisplay(1);
}

void DisplayNoWifi(){
  display.clearDisplay();
  delay(50);
  
  display.drawBitmap(0, 0,  NoWifi, 128, 64, WHITE);


  display.display();  
}

void DisplayDefault(){
  display.clearDisplay();
  delay(50);

  display.drawBitmap(0, 0,  WeatherStation, 128, 64, WHITE);
  
  display.display();  
}

void DisplayTemp(){
  display.clearDisplay();
  display.fillScreen(SSD1306_WHITE);
  
  delay(50);
  display.setCursor(0, 0);

  display.print(temperature);
  display.print(" ");
  display.print((char)247);
  display.println("C");
 

  display.display();
}

int WeatherToInt(JSONVar item){
  //Convert to int depending on the word in the forecast
  String jsonString = JSON.stringify(item);
  
  if(jsonString.indexOf(Fair) > 0) return 0;
  else if (jsonString.indexOf(Cloudy) > 0) return 1;
  else if (jsonString.indexOf(Rain) > 0) return 2;
  else if (jsonString.indexOf(Showers) > 0) return 3;
  else{
    Serial.println("Unable to find");
    return 0; //Average
  }
  
}

void UpdateWeatherData(){
  if(WiFi.status()== WL_CONNECTED){
    String serverPath = "https://api.data.gov.sg/v1/environment/24-hour-weather-forecast";

    jsonBuffer = httpGETRequest(serverPath.c_str());
    Serial.println(jsonBuffer);
    myObject = JSON.parse(jsonBuffer);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }
  
    Serial.print("JSON object = ");
    Serial.println(myObject);


    for(int i = 0; i < 3; i++){ //for diff periods
      WeatherData.west[i]    = WeatherToInt( myObject["items"][0]["periods"][i]["regions"]["west"]    );
      WeatherData.east[i]    = WeatherToInt( myObject["items"][0]["periods"][i]["regions"]["east"]   );
      WeatherData.central[i] = WeatherToInt( myObject["items"][0]["periods"][i]["regions"]["central"] );
      WeatherData.south[i]   = WeatherToInt( myObject["items"][0]["periods"][i]["regions"]["south"]   );
      WeatherData.north[i]   = WeatherToInt( myObject["items"][0]["periods"][i]["regions"]["north"]   );
    }

    
   
  }
  else {
    Serial.println("WiFi Disconnected");
    DisplayNoWifi();
  }  
}

void updateTemp(){
  jsonBuffer = httpGETRequest(OpenWeatherServer.c_str());
  JSONVar myObject = JSON.parse(jsonBuffer);
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  k_temperature = JSON.stringify(myObject["main"]["temp"]);
  temperature = k_temperature.toFloat() - 273.15;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); //TFT
  WiFi.begin(ssid, password);

  Serial.println("Connecting");
  int time_now = millis();

  InitNeopixel();
  init_TFT();

  DisplayNoWifi();

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(millis() - time_now >= 20000) resetFunc(); //30s
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  DisplayDefault();

}



void loop() {
  // Send an HTTP GET request
  if ( ((millis() - lastTime) > timerDelay) || first_loop){
    // Check WiFi connection status
    first_loop = false;
    
    UpdateWeatherData();
    updateTemp();

    delay(100);
    
    UpdateNeoxpixel();

    PrintWeatherData();
        
    lastTime = millis();
  }

  BlinkFirstPixels();

  //Pasue prog after data retrieved
}
