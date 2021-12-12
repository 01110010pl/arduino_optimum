#include "DHT.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64 ;
#define DHTPIN 4
#define DHTTYPE DHT11
#define REPORTING_PERIOD_MS     1000
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3D

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);
PulseOximeter pox;

uint32_t tsLastReport = 0;
uint32_t tsLastReport2 = 0;

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
    for(;;);
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);
  dht.begin();
  pox.begin();
  screen.setTextColor(WHITE);
  screen.cp437(true);
}

void temperature_humidity()
{
  if(millis() - tsLastReport2 > 2000UL)
  {
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
  pulse();
  temperature_humidity();
}
