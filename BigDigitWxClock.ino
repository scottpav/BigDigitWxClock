#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h> 
#include <BigNumbers_I2C.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Wire.h> 

const char *ssid     = "YOUR_SSID_HERE";
const char *password = "YOUR_WIFI_PASSWORD_HERE";
String openWeatherMapApiKey = "YOUR_API_KEY_HERE";
String city = "YOUR_CITY_NAME_HERE";
String countryCode = "US";
String jsonBuffer;
unsigned long lastTime = 0;
float tempKfloat;
String wxConditions = "";
float tempFfloat;
float tempHighFfloat;
float tempLowFfloat;
float tempHighfloat;
float tempLowfloat;
int tempF = 0;
unsigned long timerDelay = 10000;
int secondsOffset = 0;
bool connectedInit = false;
String weekDays[7]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "Sept", "October", "November", "December"};
byte x = 0;//x & y determines position of character on screen
byte y = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000);
LiquidCrystal_I2C lcd(0x3F, 20, 4);
BigNumbers_I2C bigNum(&lcd); // construct BigNumbers_I2C object, passing to it the name of our LCD object

void setup()
{
  timeClient.begin();
  lcd.begin(); // setup LCD rows and columns
  bigNum.begin(); // set up BigNumbers
  lcd.clear(); // clear display
  Serial.begin(115200);
}

void loop()
{
  timeClient.update();
  if ((millis() - lastTime) > timerDelay)
  {
    if(!connectedInit)
    {
      lcd.clear(); // clear display
      connectedInit = true;
    }
    updateWx();
    timerDelay = millis();
  }
  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  String weekDay = weekDays[timeClient.getDay()];
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;
  String currentDate = weekDay + " " + currentMonthName + " " + String(monthDay);
  String currentYearAlign = String(currentYear);
  delay(1000);
  ///////////////////////////////////////////////////////
  lcd.setCursor(0,0);
  lcd.print(currentDate);
  lcd.setCursor(16,0);
  lcd.print(currentYearAlign);
  bigNum.displayLargeInt(currentHour, x+3, y+1, 2, true);
  lcd.setCursor(9,1);
  lcd.print(".");
  lcd.setCursor(9,2);
  lcd.print(".");
  bigNum.displayLargeInt(currentMinute, x+10, y+1, 2, true);
  lcd.setCursor(16, 2);
  if(currentSecond < 10)
  {
   lcd.print("0");
   lcd.print(currentSecond);
  }
  else
  {
   lcd.print(currentSecond);
  }
  lcd.setCursor(4, 3);
  lcd.print(tempF);
  lcd.print((char)223);
  lcd.print("  ");
  lcd.print(wxConditions);
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
  http.begin(serverName);
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
  }
  // Free resources
  http.end();
  return payload;
}

void updateWx()
{
   if(WiFi.status()== WL_CONNECTED)
   {
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
    jsonBuffer = httpGETRequest(serverPath.c_str());
    Serial.println(jsonBuffer);
    JSONVar myObject = JSON.parse(jsonBuffer);
    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing1 input failed!");
      return;
    }
    tempKfloat = double(myObject["main"]["temp"]);
    wxConditions = myObject["weather"][0]["main"];
    tempFfloat = (tempKfloat - 273.15) * 9 / 5 + 32;//Convert from Kelvin to Fahrenheit
    tempHighFfloat = (tempHighfloat - 273.15) * 9 / 5 + 32;//Convert from Kelvin to Fahrenheit
    tempLowFfloat = (tempLowfloat - 273.15) * 9 / 5 + 32;//Convert from Kelvin to Fahrenheit
    Serial.print("TempKfloat: ");
    Serial.println(tempKfloat);
    Serial.print("WXCons: ");
    Serial.println(wxConditions);
    tempF = tempFfloat;
    Serial.print("TempKfloat: ");
    Serial.println(tempF);
  }
}
