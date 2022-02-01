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
const int LED_GREEN = 19;
const int LED_BLUE = 23;

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
    
    Temperature()
    {
      temC = 0;
      temF = 0;
      humidity = 0;
      indHC = 0;
      indHF = 0;
      modeC = true;
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

    void temperatureMode()
    {
      String x = "";
      if(modeC)
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
    Light() 
    {
      turnOn = false;
    }
    
    void checkLight()
    {
      if(turnOn)
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
    void lightMode()
    {
      String x = "Tryb swiatla\nStatus: ";
      if(turnOn) x += "ON";
      else x += "OFF";
      showOLED(x);
    }
};

class MP3
{
  public:
    bool turnOn;
    int genre;
    int sizeFolders[4] = {0, 0, 0, 0};
    MP3()
    {
       turnOn = false;
       genre = 1;
    }
    void checkActualMP3()
    {
      if(turnOn)
      {
        if (myDFPlayer.available()) 
        {
          if (myDFPlayer.readType() == DFPlayerPlayFinished) 
          {
            Serial.println(myDFPlayer.read());
            myDFPlayer.playLargeFolder(genre, random(1, sizeFolders[genre - 1]));
            Serial.println(myDFPlayer.readCurrentFileNumber());
            delay(500);
          }
        }
      }
    }
    void musicMode(bool workMode = false)
    {
        String x = "Rodzaj muzyki:\n";
        
        switch(genre)
        {
          case 1: x += "PRACA KLASYCZNA"; break;
          case 2: x += "PRACA LOFI"; break;
          case 3: x += "RELAKS NATURA"; break;
          case 4: x += "RELAKS SOUNDTRACK"; break;
        }
        x += "\n";
        x += "Status odtwarzania:\n";
        if(turnOn) x += "ODTWARZANIE";
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
    int choosenModes[4] = {false, false, false, false}; // puls, temperatura, światło, muzyka
    int index;
    bool turnOffMenu;
    uint32_t timeLoop;
    WorkMode()
    {
      turnOn = false;
      temperature = false;
      pulse = false;
      music = false;
      light = false;
      index = 0;
      timeLoop = 0;
      turnOffMenu = false;
    }
};

enum MODES
{
  PULSE = 0,
  TEMPERATURE,
  LIGHT,
  MUSIC 
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
  if (!pox.begin()) Serial.println("PULSE FAILED");
  else Serial.println("PULSE SUCCESS");
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // Temperatura
  dht.begin();

  // Ekran
  if(!OLEDscreen.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED SCREEN FAILED");
  else Serial.println("OLED SCREEN SUCCESS");
  
  OLEDscreen.setTextColor(WHITE);
  OLEDscreen.cp437(true);
  OLEDscreen.clearDisplay();

  // MP3
  mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17);
  delay(5000);
  if (!myDFPlayer.begin(mySoftwareSerial)) Serial.println("MP3 PLAYER FAILED");
  else Serial.println("MP3 PLAYER SUCCESS");
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
  if(!workmode.turnOffMenu)
  {
    if(workmode.turnOn)
    {
      if(millis() - workmode.timeLoop > 5000)
      {
        for(int i=0; i<4; i++)
        {
          if((workmode.index != i)&&(workmode.choosenModes[i] == true))
          {
            workmode.index = i;
            break;
          }
        }
        workmode.timeLoop = millis();
      }
      switch(workmode.index)
      {
        case 0:
        {
          menu.side1 = 2;
          break;
        }
        case 1:
        {
          menu.side1 = 3;
          break;
        }
        case 2:
        {
          menu.side1 = 4;
          break;
        }
        case 3:
        {
          menu.side1 = 5;
          break;
        }
      }
    }

    // MP3
    mp3.checkActualMP3();

    // Sprawdzenie trybu światła
    light.checkLight();

    // Aktualizacja temperatury
    temp.getTemperature();

    if(menu.side2 == 1)
    {
      switch(menu.side1)
      {
        case 1:
        {
          showOLED("TRYB PRACY", 1, 35, 25);
          showOLED("Wcisnij START!", 1, 25, 40, 500, false);
          break;
        }
        case 2: pulse.pulseMode(); break;
        case 3: temp.temperatureMode(); break;
        case 4: light.lightMode(); break;
        case 5: mp3.musicMode(); break;
      }
    }
  }
  else
  {
    showOLED("Czy chcesz zakonczyc tryb pracy?", 1, 5, 5, 1, true);
    showOLED("CZERWONY - TAK", 1, 5, 35, 1, false);
    showOLED("NIEBIESKI - NIE", 1, 5, 45, 1000, false);
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
    if(workmode.turnOffMenu)
    {
      workmode.turnOffMenu = false;
    }
    else
    {
      if(menu.side2 == 1)
      {
        if(menu.side1 == 5) menu.side1 = 1;
        else menu.side1 += 1;
      }
    }
  }
}

void checkBUTTON_ACCEPT()
{
  if(digitalRead(BUTTON_ACCEPT) == HIGH)
  {
    if(!workmode.turnOn)
    {
      switch(menu.side1)
      {
        case 1:
        {
          int checkFalses = -1;
          for(int i=0; i<4; i++)
          {
            if(workmode.choosenModes[i] != false)
            {
              checkFalses = i;
              break;
            }
          }
          if(checkFalses != -1)
          {
            for(int i=0; i<4; i++)
            {
              if(workmode.choosenModes[i] == true)
              {
                workmode.index = i;
                break;
              }
            }
            workmode.turnOn = true;
          }
          else
          {
            showOLED("NIE WYBRANO", 1, 30, 25, 0, true);
            showOLED("ZADNEGO TRYBU!", 1, 23, 50, 1500, false);
          }
          break;
        }
        case 2:
        {
          if(!workmode.pulse) workmode.pulse = true;
          else workmode.pulse = false;
          if(workmode.pulse)
          {
            showOLED("TRYB PULSU", 1, 30, 25, 1);
            showOLED("WLACZONO!", 1, 35, 40, 1500, false);
            workmode.choosenModes[PULSE] = true;
            OLEDscreen.clearDisplay();
          }
          else
          {
            showOLED("TRYB PULSU", 1, 30, 25, 1);
            showOLED("WYLACZONO!", 1, 32, 40, 1500, false);
            workmode.choosenModes[PULSE] = false;
            OLEDscreen.clearDisplay();
          }
          break;
        }
        case 3:
        {
          if(!workmode.temperature) workmode.temperature = true;
          else workmode.temperature = false;
          if(workmode.temperature)
          {
            showOLED("TRYB TEMPERATURY", 1, 15, 25, 1);
            showOLED("WLACZONO!", 1, 35, 40, 500, false);
            workmode.choosenModes[TEMPERATURE] = true;
            OLEDscreen.clearDisplay();
          }
          else
          {
            showOLED("TRYB TEMPERATURY", 1, 15, 25, 1);
            showOLED("WYLACZONO!", 1, 32, 40, 500, false);
            workmode.choosenModes[TEMPERATURE] = false;
            OLEDscreen.clearDisplay();
          }
          break;
        }
        case 4:
        {
          if(!workmode.light) workmode.light = true;
          else workmode.light = false;
          if(workmode.light)
          {
            showOLED("TRYB SWIATLA", 1, 25, 25, 1);
            showOLED("WLACZONO!", 1, 35, 40, 500, false);
            workmode.choosenModes[LIGHT] = true;
            OLEDscreen.clearDisplay();
            light.turnOn = true;
          }
          else
          {
            showOLED("TRYB SWIATLA", 1, 25, 25, 1);
            showOLED("WYLACZONO!", 1, 32, 40, 500, false);
            workmode.choosenModes[LIGHT] = false;
            light.turnOn = false;
            OLEDscreen.clearDisplay();
          }
          break;
        }
        case 5:
        {
          if(mp3.turnOn == true)
            {
              mp3.turnOn = false;
              myDFPlayer.pause();
              myDFPlayer.volume(0);
              showOLED("TRYB MUZYKI", 1, 28, 25, 1);
              showOLED("WYLACZONO!", 1, 35, 40, 500, false);
              workmode.choosenModes[MUSIC] = false;
              OLEDscreen.clearDisplay();
            }
            else
            {
              mp3.turnOn = true;
              myDFPlayer.volume(30);
              myDFPlayer.playLargeFolder(mp3.genre, random(1, mp3.sizeFolders[mp3.genre - 1]));
              showOLED("TRYB MUZYKI", 1, 28, 25, 1);
              showOLED("WLACZONO!", 1, 32, 40, 500, false);
              workmode.choosenModes[MUSIC] = true;
              OLEDscreen.clearDisplay();
            }
            break;
        }
      }
    }
    else
    {
      if(!workmode.turnOffMenu) workmode.turnOffMenu = true;
      else
      {
        workmode.turnOffMenu = false;
        workmode.turnOn = false;
      }
    } 
  }
}

void checkBUTTON_EXIT()
{
  if(digitalRead(BUTTON_EXIT) == HIGH)
  {
    switch(menu.side1)
    {
      case 3:
      {
        if(temp.modeC) temp.modeC = false;
        else temp.modeC = true;
        break;
      }
      case 5:
      {
        if(mp3.turnOn == false)
          {
            if(mp3.genre == 4) mp3.genre = 1;
            else mp3.genre += 1;
          }
          break;
      }
    }
  }
}
