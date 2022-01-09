// Biblioteki
#include "DHT.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Derektywy preprocesora
#define DHTPIN  4
#define DHTTYPE DHT11   // DHT 11
#define REPORTING_PERIOD_MS 1000

// Klasy
class Temperature 
{
  public:
    float temC;
    float temF;
    float humidity;
    float indHC;
    float indHF;
    Temperature()
    {
      temC = 0;
      temF = 0;
      humidity = 0;
      indHC = 0;
      indHF = 0;
    }
};

class Pulse
{
  public:
    float heartRate;
    float sp02;
};

class Light
{
  public:
    bool turnOn;
    Light()
    {
      turnOn = false;
    }
};

class MP3
{
  public:
    bool turnOn;
    MP3()
    {
      turnOn = false;
    }
};

class Menu
{
  public:
    int side1;
    int side2;
  Menu()
  {
    side1 = 1;
    side2 = 1;
  }
};;

// Zmienne
uint32_t tsLastReport = 0;
const int SZEROKOSC = 128;
const int WYSOKOSC  = 64;
const int BUTTON_NEXT = 39;
const int BUTTON_ACCEPT = 36;
const int BUTTON_EXIT = 34;
const int LED_RED = 18;
const int LED_GREEN= 19;
const int LED_BLUE = 23;
Temperature temp;
Menu menu;
MP3 mp3;
Light light;
Pulse pulse;

// Inicjalizacja czujników
DHT dht(DHTPIN, DHTTYPE);
PulseOximeter pox;
Adafruit_SSD1306 ekran(SZEROKOSC, WYSOKOSC, &Wire, -1);

// Setup
void setup() 
{
  Serial.begin(115200);

  // PINY
  pinMode(BUTTON_ACCEPT, INPUT_PULLUP);
  pinMode(BUTTON_EXIT, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Puls
  if (!pox.begin()) 
  {
    Serial.println("FAILED");
    for(;;);
  } 
  else Serial.println("SUCCESS");
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // Temperatura
  dht.begin();

  // Ekran
  if(!ekran.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  {
    Serial.println("Błąd inicjalizacji wyświetlacza!");
    exit(0); // Nie idź dalej.
  }
  ekran.setTextColor(WHITE);
  ekran.cp437(true);
}

// Metody
void temperatura()
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

void onBeatDetected()
{
    Serial.println("WYkryto puls!");
}

void puls()
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

void showLCD(String message)
{
  ekran.clearDisplay();
  ekran.setTextSize(1.5);
  ekran.setCursor(10, 0);
  ekran.print(message);
  ekran.display();
  delay(250);
}

// Loop
void loop() 
{
  if(digitalRead(BUTTON_NEXT) == HIGH)
  {
    if(menu.side2 == 1)
    {
      if(menu.side1 == 5) menu.side1 = 1;
      else menu.side1 += 1;
    }
  }
  if(menu.side2 == 1)
  {
    switch(menu.side1)
    {
      case 1:
      {
        showLCD("1) Puls");
        break;
      }
      case 2:
      {
        showLCD("2) Temperatura");
        break;
      }
      case 3:
      {
        showLCD("3) Swiatlo");
        break;
      }
      case 4:
      {
        showLCD("4) Muzyka");
        break;
      }
      case 5:
      {
        showLCD("5) Tryb pracy");
        break;
      }
    }
  }
}
