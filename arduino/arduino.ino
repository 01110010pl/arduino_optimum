// Biblioteki
#include "DHT.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>
#include <iostream>

// Derektywy preprocesora
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTPIN 4
#define DHTTYPE DHT11
#define REPORTING_PERIOD_MS     1000
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

// Zmienne
uint32_t tsLastReport = 0;
int counter = 1;

// Obiekty modułów
Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);
PulseOximeter pox;

void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup() 
{
  Serial.begin(115200);
  if(!screen.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    exit(0);
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);
  dht.begin();
  pox.begin();
  screen.setTextColor(WHITE);
  screen.cp437(true);
}

void temperature_humidity()
{
   delay(2000);
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   float f = dht.readTemperature(true);
   if (isnan(h) || isnan(t) || isnan(f)) 
   {
     Serial.println(F("Failed to read from DHT sensor!"));
     return;
   }
   float hif = dht.computeHeatIndex(f, h);
   float hic = dht.computeHeatIndex(t, h, false);
   Serial.print(F("Humidity: "));
   Serial.print(h);
   Serial.print(F("%  Temperature: "));
   Serial.print(t);
   Serial.print(F("°C "));
   Serial.print(f);
   Serial.print(F("°F  Heat index: "));
   Serial.print(hic);
   Serial.print(F("°C "));
   Serial.print(hif);
   Serial.println(F("°F"));
}

String newTemperatureHumidity(int y)
{
  delay(2000);
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   float f = dht.readTemperature(true);
   if (isnan(h) || isnan(t) || isnan(f)) 
   {
     return F("Failed to read from DHT sensor!");
   }
   float hif = dht.computeHeatIndex(f, h);
   float hic = dht.computeHeatIndex(t, h, false);
   if(y == 1)
   {
      return "Humidity: " + String(h) + "%";
   }
   else if(y == 2)
   {
      return "Temperature: \n  " + String(t) + " C\n  " + String(f) + " F";
   }
   else if(y == 3)
   {
      return "Heat index: \n  " + String(hic) + " C\n  " + String(hif) + " F";
   }
}

void pulse()
{
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
  {
       Serial.print("Heart rate:");
       Serial.print(pox.getHeartRate());
       Serial.print("bpm / SpO2:");
       Serial.print(pox.getSpO2());
       Serial.println("%");
       tsLastReport = millis();
   }
}

void loop() 
{
      if(counter > 3)
      {
        counter = 1;
      }
      screen.clearDisplay();
      screen.setTextSize(1);
      screen.setCursor(10, 0);
      screen.print(newTemperatureHumidity(counter));
      screen.display();
      counter++;
      delay(250);
}
