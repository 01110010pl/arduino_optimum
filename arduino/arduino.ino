// Biblioteki
#include "DHT.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Derektywy preprocesora
#define DHTPIN  4
#define DHTTYPE DHT11
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
    bool modeC;
    Temperature()
    {
      temC = 0;
      temF = 0;
      humidity = 0;
      indHC = 0;
      indHF = 0;
      modeC = true;
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
    bool intro;
  Menu()
  {
    side1 = 1;
    side2 = 1;
    intro = true;
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
  ekran.setCursor(0, 0);
  ekran.print(message);
  ekran.display();
  delay(250);
}

// Loop
void loop() 
{
  // Intro
  if(menu.intro)
  {
    String brand = "OPTIMUM";
    char message[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
    message[0] = brand[0];
    int counter = 1;
    while(true)
    {
      ekran.clearDisplay();
      ekran.setTextSize(2);
      ekran.setCursor(25, 25);
      ekran.print(message);
      ekran.display();
      delay(200);
      message[counter] = brand[counter];
      counter += 1;
      if(counter == 8) break;
    }
    delay(200);
    menu.intro = false;
    ekran.clearDisplay();
  }
  
  // Sprawdzenie przycisków
  if(digitalRead(BUTTON_NEXT) == HIGH)
  {
    if(menu.side2 == 1)
    {
      if(menu.side1 == 5) menu.side1 = 1;
      else menu.side1 += 1;
    }
  }
  if(digitalRead(BUTTON_ACCEPT) == HIGH)
  {
    if(menu.side2 == 1) menu.side2 = 2;
  }
  if(digitalRead(BUTTON_EXIT) == HIGH)
  {
    if(menu.side2 == 2) menu.side2 = 1;
    else
    {
      if(menu.side1 == 1) menu.side1 = 5;
      else menu.side1 -= 1;
    }
  }

  // Sprawdzenie trybu światła
  if(light.turnOn == true)
  {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
  }
  else
  {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
  }

  // AKtualizacja temperatury
  temp.humidity = dht.readHumidity();
  temp.temC = dht.readTemperature();
  temp.temF = dht.readTemperature(true);
  if (isnan(temp.humidity) || isnan(temp.temC) || isnan(temp.temF)) 
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  temp.indHF = dht.computeHeatIndex(temp.temF, temp.humidity);
  temp.indHC = dht.computeHeatIndex(temp.temC, temp.humidity, false);
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
  else
  {
    switch(menu.side1)
    {
      case 1:
      {
        String x = "1) Puls\nPuls: ";
        x += String(pulse.heartRate);
        x += "bpm\nSp02: ";
        x += String(pulse.sp02);
        x + "%";
        showLCD(x);
        break;
      }
      case 2:
      {
        if(digitalRead(BUTTON_NEXT) == HIGH)
        {
          if(temp.modeC) temp.modeC = false;
          else temp.modeC = true;
        }
        String x = "";
        if(temp.modeC)
        {
            x = "Temperatura: \n" + String(temp.temC) + "C\n";
            x += "Wilgotnosc: \n" + String(temp.humidity) + "%\n";
            x += "Indeks ciepla: \n" + String(temp.indHC) + " C";
          }
          else
          {
            x = "Temperatura: \n" + String(temp.temF) + "F\n";
            x += "Wilgotnosc: \n" + String(temp.humidity) + "%\n";
            x += "Indeks ciepla: \n" + String(temp.indHF) + " F";
          }
        showLCD(x);
        break;
      }
      case 3:
      {
        if(digitalRead(BUTTON_ACCEPT) == HIGH)
        {
          if(light.turnOn == true) light.turnOn = false;
          else light.turnOn = true;
        }
        String x = "Tryb automatyczny swiatla\nStatus: ";
        if(light.turnOn == false) x += "OFF";
        else x += "ON";
        showLCD(x);
        break;
      }
      case 4:
      {
        showLCD("D");
        break;
      }
      case 5:
      {
        showLCD("E");
        break;
      }
    }
  }
}
