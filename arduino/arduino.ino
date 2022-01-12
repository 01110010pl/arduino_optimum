// Biblioteki
#include "DHT.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

// Derektywy preprocesora
#define DHTPIN  4
#define DHTTYPE DHT11
#define REPORTING_PERIOD_MS 1000

// Zmienne
uint32_t tsLastReport = 0;
const int WIDTH_OLED = 128;
const int HEIGHT_OLED = 64;
const int BUTTON_NEXT = 39;
const int BUTTON_ACCEPT = 36;
const int BUTTON_EXIT = 34;
const int LED_RED = 18;
const int LED_GREEN= 19;
const int LED_BLUE = 23;
int delayms = 100;

// Inicjalizacja czujników
DHT dht(DHTPIN, DHTTYPE);
PulseOximeter pox;
Adafruit_SSD1306 OLEDscreen(WIDTH_OLED, HEIGHT_OLED, &Wire, -1);
HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;

// Lista funkcji
void intro();
void setup();
void puls();
void onBeatDetected();
void showOLED(String message, float sizeMessage = 1.5, float cursorX = 0.0, float cursorY = 0.0, int delayShow = 250, bool clearBegin = true);
void loop();
void checkBUTTON_NEXT();
void checkBUTTON_ACCEPT();
void checkBUTTON_EXIT();

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
    bool modeCWorkMode;
    Temperature()
    {
      temC = 0;
      temF = 0;
      humidity = 0;
      indHC = 0;
      indHF = 0;
      modeC = true;
      modeCWorkMode = true;
    }
    void getTemperature()
    {
      humidity = dht.readHumidity();
      temC = dht.readTemperature();
      temF = dht.readTemperature(true);
      if (isnan(humidity) || isnan(temC) || isnan(temF)) 
      {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }
      indHF = dht.computeHeatIndex(temF, humidity);
      indHC = dht.computeHeatIndex(temC, humidity, false);
    }
    void temperatureMode(bool workMode = false)
    {
      bool a = false;
      if(workMode) a = modeCWorkMode;
      else a = modeC;
      String x = "";
      if(a)
      {
        x = "Temperatura: \n" + String(temC) + "C\n";
        x += "Wilgotnosc: \n" + String(humidity) + "%\n";
        x += "Indeks ciepla: \n" + String(indHC) + " C";
      }
      else
      {
        x = "Temperatura: \n" + String(temF) + "F\n";
        x += "Wilgotnosc: \n" + String(humidity) + "%\n";
        x += "Indeks ciepla: \n" + String(indHF) + " F";
      }
      showOLED(x);
    }
};

class Pulse
{
  public:
    float heartRate;
    float sp02;
    void pulseMode()
    {
      String x = "Puls: ";
      x += String(heartRate);
      x += "bpm\nSp02: ";
      x += String(sp02);
      x + "%";
      showOLED(x);
    }
};

class Light
{
  public:
    bool turnOn;
    bool turnOnWorkMode;
    Light() 
    {
      turnOn = false;
    }
    void checkLight(bool workMode)
    {
      bool x = false;
      if(workMode)
      {
        x = turnOnWorkMode;
      }
      else x = turnOn;
      if(x == true)
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
    }
    void lightMode(bool workMode = false)
    {
      String x = "Tryb automatyczny swiatla\nStatus: ";
      bool a = false;
      if(workMode) a = turnOnWorkMode;
      else a = turnOn;
      if(a == false) x += "OFF";
      else x += "ON";
      showOLED(x);
    }
};

class MP3
{
  public:
    bool turnOn;
    int genre;
    int sizeFolders[4] = {0, 0, 0, 0};
    int turnOnWorkMode;
    int genreWorkMode;
    MP3()
    {
       turnOn = false;
       genre = 1;
       genreWorkMode = 1;
       turnOnWorkMode = false;
    }
    void checkActualMP3(bool workMode)
    {
      bool x = false;
      int y = 0;
      if(workMode)
      {
        x = turnOnWorkMode;
        y = genreWorkMode;
      }
      else
      {
        x = turnOn;
        y = genre;
      }
      if(x)
      {
        if (myDFPlayer.available()) 
        {
          if (myDFPlayer.readType()==DFPlayerPlayFinished) 
          {
            Serial.println(myDFPlayer.read());
            myDFPlayer.playLargeFolder(y, random(1, sizeFolders[y - 1]));
            Serial.println(myDFPlayer.readCurrentFileNumber());
            delay(500);
          }
        }
      }
    }
    void musicMode(bool workMode = false)
    {
        String x = "Rodzaj muzyki:\n";
        int y = 0;
        if(workMode) y = genreWorkMode;
        else y = genre;
        switch(y)
        {
          case 1: x += "PRACA KLASYCZNA"; break;
          case 2: x += "PRACA LOFI"; break;
          case 3: x += "RELAKS NATURA"; break;
          case 4: x += "RELAKS SOUNDTRACK"; break;
        }
        x += "\n";
        x += "Status odtwarzania:\n";
        bool a = false;
        if(workMode) a = turnOnWorkMode;
        else a = turnOn;
        if(a) x += "ODTWARZANIE";
        else x += "STOP";
        showOLED(x, 1.5, 0, 0, 250);
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
    }
};

class WorkMode
{
  public:
    bool turnOn;
    bool temperature;
    bool pulse;
    bool music;
    bool light;
    int side;
    bool workModeMenu;
    WorkMode()
    {
      turnOn = false;
      temperature = false;
      pulse = false;
      music = false;
      light = false;
      side = 1;
    }
};

// Inicjalizacja obiektów czujników
Temperature temp;
Menu menu;
MP3 mp3;
Light light;
Pulse pulse;
WorkMode workmode;

// Setup
void setup() 
{
  Serial.begin(115200);

  // Piny
  pinMode(BUTTON_ACCEPT, INPUT_PULLUP);
  pinMode(BUTTON_EXIT, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Puls
  if (!pox.begin()) 
  {
    Serial.println("PULSE FAILED");
    exit(0);
  } 
  else Serial.println("PULSE SUCCESS");
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // Temperatura
  dht.begin();

  // Ekran
  if(!OLEDscreen.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  {
    Serial.println("OLED SCREEN FAILED");
    exit(0);
  }
  OLEDscreen.setTextColor(WHITE);
  OLEDscreen.cp437(true);
  OLEDscreen.clearDisplay();

  // MP3
  mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17);
  if (!myDFPlayer.begin(mySoftwareSerial)) 
  {
    Serial.println(myDFPlayer.readType(), HEX);
    Serial.println("MP3 PLAYER FAILED");
    exit(0);
  }
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(0);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  for(int i=0; i<4; i++) mp3.sizeFolders[i] = myDFPlayer.readFileCountsInFolder(i + 1);
  intro();
}

// Loop
void loop() 
{
  // Sprawdzenie przycisków
  checkBUTTON_NEXT();
  checkBUTTON_ACCEPT();
  checkBUTTON_EXIT();
  
  // MP3
  mp3.checkActualMP3(workmode.turnOn);

  // Sprawdzenie trybu światła
  light.checkLight(workmode.turnOn);

  // Aktualizacja temperatury
  temp.getTemperature();

  if(menu.side2 == 1)
  {
    switch(menu.side1)
    {
      case 1: showOLED("PULS", 1, 10, 10); break;
      case 2: showOLED("TEMPERATURA", 1, 10, 10); break;
      case 3: showOLED("SWIATLO", 1, 10, 10); break;
      case 4: showOLED("MUZYKA", 1, 10, 10); break;
      case 5: showOLED("TRYB PRACY", 1, 10, 10); break;
    }
  }
  else
  {
    switch(menu.side1)
    {
      case 1: pulse.pulseMode(); break;
      case 2: temp.temperatureMode(); break;
      case 3: light.lightMode(); break;
      case 4: mp3.musicMode(); break;
      case 5:
      {
        switch(workmode.side)
        {
          case 1:
          {
            showOLED("PRACA");
            break;
          }
          case 2:
          {
            pulse.pulseMode();
            break;
          }
          case 3:
          {
            temp.temperatureMode(!workmode.turnOn);
            break;
          }
          case 4:
          {
            light.lightMode(!workmode.turnOn);
            break;
          }; //light
          case 5: 
          {
            mp3.musicMode(!workmode.turnOn);
            break;
          } //music
        }
      }
    }
  }
}

// Metody
void onBeatDetected()
{
    Serial.println("Wykryto puls!");
}

void showOLED(String message, float sizeMessage, float cursorX, float cursorY , int delayShow, bool clearBegin)
{
  if(clearBegin) OLEDscreen.clearDisplay();
  OLEDscreen.setTextSize(sizeMessage);
  OLEDscreen.setCursor(cursorX, cursorY);
  OLEDscreen.print(message);
  OLEDscreen.display();
  delay(delayShow);
}

void intro()
{
    String brand = "OPTIMUM";
    char message[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
    message[0] = brand[0];
    int counter = 1;
    while(true)
    {
      showOLED(message, 2, 25, 25, 200);
      message[counter] = brand[counter];
      counter += 1;
      if(counter == 8) break;
    }
    showOLED("by POGGERS TEAM", 1, 22, 45, 1000, false);
    OLEDscreen.clearDisplay();
}

void checkBUTTON_NEXT()
{
  if(digitalRead(BUTTON_NEXT) == HIGH)
  {
    if(menu.side2 == 1)
    {
      if(menu.side1 == 5) menu.side1 = 1;
      else menu.side1 += 1;
    }
    else
    {
      switch(menu.side1)
      {
        case 2:
        {
          if(temp.modeC) temp.modeC = false;
          else temp.modeC = true;
          break;
        }
        case 4:
        {
          if(mp3.turnOn == false)
          {
            if(mp3.genre == 4) mp3.genre = 1;
            else mp3.genre += 1;
          }
          break;
        }
        case 5:
        {
          if(workmode.side == 5) workmode.side = 1;
          else workmode.side += 1;
        }
      }
    }
  
  }
}

void checkBUTTON_ACCEPT()
{
  if(digitalRead(BUTTON_ACCEPT) == HIGH)
  {
    if(menu.side2 == 1) menu.side2 = 2;
    else
    {
      switch(menu.side1)
      {
        case 3:
        {
          if(light.turnOn == true) light.turnOn = false;
          else light.turnOn = true;
          break;
        }
        case 4:
        {
          if(mp3.turnOn == true)
          {
            mp3.turnOn = false;
            myDFPlayer.pause();
            myDFPlayer.volume(0);
          }
          else
          {
            mp3.turnOn = true;
            myDFPlayer.volume(30);
            myDFPlayer.playLargeFolder(mp3.genre, random(1, mp3.sizeFolders[mp3.genre - 1]));
          }
          break;
        }
        case 5:
        {
          switch(workmode.side)
          {
            case 1: break;
            case 2:
            {
              if(workmode.pulse)
              {
                 workmode.pulse = false;
                 showOLED("SLEDZENIE PULSU WYLACZONE!");
                 break;
              }
              else
              {
                workmode.pulse =  true;
                showOLED("SLEDZENIE PULSU WLACZONE!");
                break;
              }
            }
            case 3:
            {
              if(workmode.temperature)
              {
                workmode.temperature = false;
                showOLED("SLEDZENIE TEMPERATURY WYLACZONE!", 1, 0, 0, 1000);
                break;
              }
              else
              {
                workmode.temperature =  true;
                showOLED("SLEDZENIE TEMPERATURY WLACZONE!", 1, 0, 0, 1000);
                break;
              }
            }
            case 4:
            {
              if(workmode.light)
              {
                workmode.light = false;
                showOLED("SLEDZENIE SWIATLA WYLACZONE!", 1, 0, 0, 1000);
        
              }
              else
              {
                workmode.light =  true;
                showOLED("SLEDZENIE SWIATLA WLACZONE!", 1, 0, 0, 1000);
                
              }
              if(light.turnOnWorkMode) light.turnOnWorkMode = false;
              else light.turnOnWorkMode = true;
              break;
            }
            case 5:
            {
              if(workmode.music)
              {
                workmode.music = false;
                mp3.turnOnWorkMode = false;
                showOLED("MUZYKA WYLACZONA!", 1, 0, 0, 1000);        
              }
              else
              {
                workmode.music =  true;
                mp3.turnOnWorkMode = true;
                showOLED("MUZYKA WLACZONA!", 1, 0, 0, 1000);   
              }
              
              break;
            }
          }
        }
      }
    }
  }
}

void checkBUTTON_EXIT()
{
  if(digitalRead(BUTTON_EXIT) == HIGH)
  {
    if(menu.side2 == 2)
    {
      if(menu.side1 == 5)
      {
        if(workmode.side == 1)
        {
          menu.side2 = 1;
        }
        else
        {
          switch(workmode.side)
          {
            case 3:
            {
              if(temp.modeCWorkMode) temp.modeCWorkMode = false;
              else temp.modeCWorkMode = true;
              break;
            }
            case 5:
            {
              if(!mp3.turnOnWorkMode)
              {
                if(mp3.genreWorkMode == 4) mp3.genreWorkMode = 1;
                else mp3.genreWorkMode += 1;
              }
              break;
            }
          }
        }
      }
      else
      {
        menu.side2 = 1;
      }
    }
    else
    {
      if(menu.side1 == 1) menu.side1 = 5;
      else menu.side1 -= 1;
    }
  }
}
