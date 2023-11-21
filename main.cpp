#include "DHT.h"          //Library for DHT11 Tempersture and humidty Sensor
#include "WiFi.h"         //Library for WiFi
#include <HTTPClient.h>   //Library for WiFi
#include <Wire.h>         //Library for BMP180
#include <Adafruit_BMP085.h>  //Library for BMP180
#include "ThingSpeak.h"
#include "SPIFFS.h"     //library for save data local memory
#include <vector>        //library for save data local memory

#define DHT11PIN 4      //Define pin 4 is DHT11 signal input pin
#define rainAnalogPin 35  //Define pin 35 is rainAnalog signal input pin
#define rainDigitalPin 34   //Define pin 34 is rainDig signal input pin
#define rainwaterLevelPin 32    //Define pin 32 is rainwater level measure signal input pin
#define LDRPIN 33     //Define pin 33 is LDR signal input pin
#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP  300        // Time ESP32 will go to sleep (in seconds)

RTC_DATA_ATTR int bootCount = 0;

// WiFi DETAILS
const char* ssid = "Redmi Note 8";         // wifi SSID
const char* password = "12345678";    // Wifi password

// Google script ID
String GOOGLE_SCRIPT_ID = "AKfycby87NBaa55-8paHMnfRA_YWFyuLzbf4VDs8OP_9cso9bg-WaD07GC3nxFgDHu3xUkV2nw";    // Google shwwt id
unsigned long thingsPeakID = 1872622; // thingSpeak id

using namespace std; //"SPIFFS.h library

const char *writeAPIKey = "IGGS23FWHNN29TU3"; // thingSpeak API
WiFiClient client;

int count = 0;

DHT dht(DHT11PIN, DHT11);
Adafruit_BMP085 bmp;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
void usedcharactors(){
  
  }
void listAllFiles() 
{
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("FILE: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
}

void setup() {
  
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  
  SPIFFS.begin(true);
  File tempfile;
  File humidityfile;

  
  dht.begin();  //Start DHT11

  char i = 0;
  char j = 0;
  char k = 0;
  char g = 0;
  char h = 0;
  char t=0;
  char StartupEmptypoint;
  char n = 80; //offline recorded array lenth
  char WifiConnectWaitingTime=20;
  //offline log arrays
  String temparray[n];
  String humidarray[n];
  String pressurearray[n];
  String ldrarray[n];

  delay(1000);
  
 if (!bmp.begin()) {
    Serial.println("Could not find the BMP180 sensor, check connections!!!");
    while (1) {}
  }
   //Rain Sensor
   pinMode(rainDigitalPin, INPUT);
   pinMode(rainwaterLevelPin, INPUT);
  
    // connect to WiFi
    Serial.println();
    Serial.print("Connecting to wifi: ");
    Serial.println(ssid);
    Serial.flush();
    WiFi.begin(ssid, password);

        float dhtHumidity = dht.readHumidity();
        float dhtTemperature = dht.readTemperature();
        float pressure = bmp.readPressure();
        int analogValueLDR = analogRead(LDRPIN);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");

      k++;
      if (k > WifiConnectWaitingTime) { // if waiting time
        Serial.println("\ndata recording Offline...");
        if(!(SPIFFS.exists("/tempc.txt")) && !(SPIFFS.exists("/humidity.txt")) && !(SPIFFS.exists("/pressure.txt")) && !(SPIFFS.exists("/ldr.txt"))){
          File tempfile = SPIFFS.open("/tempc.txt", FILE_WRITE);
          File humidityfile = SPIFFS.open("/humidity.txt", FILE_WRITE);
          File pressfile = SPIFFS.open("/pressure.txt", FILE_WRITE);
          File ldrfile = SPIFFS.open("/ldr.txt", FILE_WRITE);
  
          Serial.println("file write in write mode");
          // log to fiel
          tempfile.println(String(dhtTemperature));
          humidityfile.println(String(dhtHumidity));
          pressfile.println(String(pressure));
         ldrfile.println(String(analogValueLDR));

          tempfile.close();
          humidityfile.close();  
          pressfile.close();
          ldrfile.close(); 
          }else{
            File tempfilesecond = SPIFFS.open("/tempc.txt","a" );
            File humidityfilesecond = SPIFFS.open("/humidity.txt", "a");
            File pressfilesecond = SPIFFS.open("/pressure.txt","a" );
            File ldrfilesecond = SPIFFS.open("/ldr.txt", "a");
            tempfilesecond.println(String(dhtTemperature));
            humidityfilesecond.println(String(dhtHumidity));
            pressfilesecond.println(String(pressure));
            ldrfilesecond.println(String(analogValueLDR));
            Serial.println("file write in append mode");
            tempfilesecond.close();
            humidityfilesecond.close();
            pressfilesecond.close();
            ldrfilesecond.close();
          }
      k = 0;
      break;// when process is done leave while() loop
      }
    }
   ThingSpeak.begin(client); //assign the wifi into thingspeak

       //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  
}

void loop() {

  char i = 0;
  char j = 0;
  char k = 0;
  char g = 0;
  char h = 0;
  char t=0;
  char StartupEmptypoint;
  char n = 80; //offline recorded array length
  char WifiConnectWaitingTime=25;
  //offline log arrays
  String temparray[n];
  String humidarray[n];
  String pressarray[n];
  String ldrarray[n];
  
  float dhtHumidity = dht.readHumidity();
  float dhtTemperature = dht.readTemperature();
  float pressure = bmp.readPressure();
  int analogValueLDR = analogRead(LDRPIN);
  int rainAnalogVal = analogRead(rainAnalogPin); 
  int rainDigitalVal = digitalRead(rainDigitalPin);
  float rainwaterLevelOld = analogRead(rainwaterLevelPin);

   if(isnan(dhtHumidity) || isnan(dhtTemperature)){
    Serial.println(F("Failed read DHT11 sensor!!!"));
    return;
    }

  //calibrated equation for Water Level messurement. EQ; y = 74.182x + 84.431;
  float rainwaterLevel = (74.182*(rainwaterLevelOld)) + 84.431;

  //calibrated equation for analog to lux converter for LDR. EQ; y' = 0.00010213x^2 - 0.4717x + 590.3023;
  float luxvalue = (0.00010213*(analogValueLDR*analogValueLDR))-(0.4717*analogValueLDR)+590.3023;

  ThingSpeak.setField(1,dhtTemperature);
  ThingSpeak.setField(2,dhtHumidity);
  ThingSpeak.setField(3,pressure);
  ThingSpeak.setField(4,luxvalue);
  int respons1 = ThingSpeak.writeFields(thingsPeakID,writeAPIKey);


  
 
     if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

            //if there exists spiff files they should upload first to the google sheet
        if ((SPIFFS.exists("/tempc.txt")) && (SPIFFS.exists("/humidity.txt"))) {
         
          File tempfile1 = SPIFFS.open("/tempc.txt");
          File humidityfile1 = SPIFFS.open("/humidity.txt");
          File pressfile1 = SPIFFS.open("/pressure.txt");
          File ldrfile1 = SPIFFS.open("/ldr.txt");
          vector<String> v1;
          vector<String> v2;
          vector<String> v3;
          vector<String> v4;
    
          while (tempfile1.available()) {
            v1.push_back(tempfile1.readStringUntil('\n'));
          }
    
          while (humidityfile1.available()) {
            v2.push_back(humidityfile1.readStringUntil('\n'));
          }
          while (pressfile1.available()) {
            v3.push_back(pressfile1.readStringUntil('\n'));
          }
    
          while (ldrfile1.available()) {
            v4.push_back(ldrfile1.readStringUntil('\n'));
          }
    
          tempfile1.close();
          humidityfile1.close();
          pressfile1.close();
          ldrfile1.close();
    //retrieve log data to array s
          for (String s1 : v1) {
            temparray[i] = s1;
            i++;
          }
    
          for (String s2 : v2) {
            humidarray[j] = s2;
            j++;
          }

          for (String s3 : v3) {
            pressarray[g] = s3;
            g++;
          }

          for (String s4 : v4) {
            ldrarray[h] = s4;
          }
          while (t <= n) {
            if(temparray[t]==0 && humidarray[t]==0 && pressarray[t]==0 && ldrarray[t]==0){
                Serial.println("\nArrays are empty....");
                StartupEmptypoint=t;
                break;
              }else{
                if (WiFi.status() == WL_CONNECTED) {
                  HTTPClient http;
                  Serial.println("offline data uploading... ");
                  Serial.print("temp :");
                  Serial.print(temparray[t]);
                  Serial.print("\t");
                  Serial.print("humid :");
                  Serial.print(humidarray[t]);
                  Serial.print("\n");
                  Serial.print("Press :");
                  Serial.print(pressarray[t]);
                  Serial.print("\t");
                  Serial.print("ldr :");
                  Serial.print(ldrarray[t]);
                  Serial.print("\n");
                  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?"+"Temperature=" + String(temparray[t]) + "&Humidity=" + String(humidarray[t])+"&Pressure=" + String(pressarray[t]) + "&LDR=" + String(ldrarray[t]);
                  // httprequest(String(tempC), String(humi), GOOGLE_SCRIPT_ID);
                  Serial.println("Making a request");
                  http.begin(url.c_str());
                  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
                  int httpCode = http.GET();
                  String payload;
                  if (httpCode > 0) { //Check for the returning code
                    payload = http.getString();
                    Serial.println(httpCode);
                    Serial.println(payload);
                  }
                  else {
                    Serial.println("Error on HTTP request");
                  }
                  http.end();
                }
              
              t++; 


              }
          }
          if (t == n || t==StartupEmptypoint) {//remove files
              Serial.println("Before remove");
              listAllFiles();
              SPIFFS.remove("/tempc.txt");
              SPIFFS.remove("/humidity.txt");
              Serial.println("After remove");
              listAllFiles();
              i = 0;
              j = 0;
            }
        }else{
              
            if(rainAnalogVal<=4000){
              Serial.print("Temperature: ");
              Serial.print(dhtTemperature);
              Serial.println("ºC ");
              Serial.print("Humidity: ");
              Serial.println(dhtHumidity);
              Serial.print("Pressure = ");
              Serial.print(pressure);
              Serial.println(" Pa");
              Serial.print("Lux Value = "); // LDR
              Serial.println(luxvalue); 
              Serial.print("Rain Level: ");
              Serial.println(rainwaterLevel);
              delay(100);
              String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?"+"Temperature=" + String(dhtTemperature) + "&Humidity=" + String(dhtHumidity)+ "&Pressure=" + String(pressure)+"&LDR=" + String(luxvalue)+ "&WDIr=" + String(rainwaterLevel)+ "&Rain=Light_Rain";
              Serial.println("Making a request");
              http.begin(url.c_str());
              http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
              int httpCode = http.GET();
              String payload;
              if (httpCode > 0) {
                payload = http.getString();
                Serial.println(httpCode);
                Serial.println(payload);
              }
              else {
                Serial.println("Error on HTTP request");
              }
              http.end();
            }
          

           else if(rainAnalogVal<=2000){
              Serial.print("Temperature: ");
              Serial.print(dhtTemperature);
              Serial.println("ºC ");
              Serial.print("Humidity: ");
              Serial.println(dhtHumidity);
              Serial.print("Pressure = ");
              Serial.print(pressure);
              Serial.println(" Pa");
              Serial.print("Lux Value = "); // LDR
              Serial.println(luxvalue); 
              Serial.print("Rain Level: ");
              Serial.println(rainwaterLevel);
              delay(100);
              String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?"+"Temperature=" + String(dhtTemperature) + "&Humidity=" + String(dhtHumidity)+ "&Pressure=" + String(pressure)+"&LDR=" + String(luxvalue)+ "&WDIr=" + String(rainwaterLevel)+ "&Rain=Midium_Rain";
              Serial.println("Making a request");
              http.begin(url.c_str()); //Specify the URL and certificate
              http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
              int httpCode = http.GET();
              String payload;
              if (httpCode > 0) { //Check for the returning code
                payload = http.getString();
                Serial.println(httpCode);
                Serial.println(payload);
              }
              else {
                Serial.println("Error on HTTP request");
              }
              http.end();
            }
          

           else if(rainAnalogVal<=3500){
              Serial.print("Temperature: ");
              Serial.print(dhtTemperature);
              Serial.println("ºC ");
              Serial.print("Humidity: ");
              Serial.println(dhtHumidity);
              Serial.print("Pressure = ");
              Serial.print(pressure);
              Serial.println(" Pa");
              Serial.print("Lux Value = "); // LDR
              Serial.println(luxvalue); 
              Serial.print("Rain Level: ");
              Serial.println(rainwaterLevel);
              delay(100);
              String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?"+"Temperature=" + String(dhtTemperature) + "&Humidity=" + String(dhtHumidity)+ "&Pressure=" + String(pressure)+"&LDR=" + String(luxvalue)+ "&WDIr=" + String(rainwaterLevel)+ "&Rain=Heavy_Rain";
              Serial.println("Making a request");
              http.begin(url.c_str()); //Specify the URL and certificate
              http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
              int httpCode = http.GET();
              String payload;
              if (httpCode > 0) { //Check for the returning code
                payload = http.getString();
                Serial.println(httpCode);
                Serial.println(payload);
              }
              else {
                Serial.println("Error on HTTP request");
              }
              http.end();
            }
          

           else{
              Serial.print("Temperature: ");
              Serial.print(dhtTemperature);
              Serial.println("ºC ");
              Serial.print("Humidity: ");
              Serial.println(dhtHumidity);
              Serial.print("Pressure = ");
              Serial.print(pressure);
              Serial.println(" Pa");
              Serial.print("Lux Value = "); // LDR
              Serial.println(luxvalue); 
              Serial.print("Rain Level: ");
              Serial.println(rainwaterLevel);
              delay(100);
              String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?"+"Temperature=" + String(dhtTemperature) + "&Humidity=" + String(dhtHumidity)+ "&Pressure=" + String(pressure)+"&LDR=" + String(luxvalue)+ "&WDIr=" + String(rainwaterLevel)+ "&Rain=No_Rain";
              Serial.println("Making a request");
              http.begin(url.c_str()); //Specify the URL and certificate
              http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
              int httpCode = http.GET();
              String payload;
              if (httpCode > 0) { //Check for the returning code
                payload = http.getString();
                Serial.println(httpCode);
                Serial.println(payload);
              }
              else {
                Serial.println("Error on HTTP request");
              }
              http.end();
            }
          }
     }
  //count++;
      WiFi.disconnect();
      
      Serial.println("WiFi Disconnectiong....");
      delay(1000);

     Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}