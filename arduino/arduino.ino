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
    int genre;
    int sizeFolders[4] = {0, 0, 0, 0};
    MP3()
    {
       turnOn = false;
       genre = 1;
       
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
};

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
int delayms = 100;
Temperature temp;
Menu menu;
MP3 mp3;
Light light;
Pulse pulse;

// Inicjalizacja czujników
DHT dht(DHTPIN, DHTTYPE);
PulseOximeter pox;
Adafruit_SSD1306 ekran(SZEROKOSC, WYSOKOSC, &Wire, -1);
HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;

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

  //MP3
  mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17);
  if (!myDFPlayer.begin(mySoftwareSerial)) 
  {
    Serial.println(myDFPlayer.readType(), HEX);
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(0);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  for(int i=0; i<4; i++) mp3.sizeFolders[i] = myDFPlayer.readFileCountsInFolder(i + 1);
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
    ekran.setTextSize(1);
    ekran.setCursor(22, 45);
    ekran.print("by POGGERS TEAM");
    ekran.display();
    menu.intro = false;
    delay(1000);
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

  //MP3
if(mp3.turnOn)
{
  if (myDFPlayer.available()) 
  {
    if (myDFPlayer.readType()==DFPlayerPlayFinished) 
    {
      Serial.println(myDFPlayer.read());
      Serial.println(F("next--------------------"));
      myDFPlayer.playLargeFolder(mp3.genre, random(1, mp3.sizeFolders[mp3.genre - 1]));  //Play next mp3 every 3 second.
      Serial.println(F("readCurrentFileNumber--------------------"));
      Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
      delay(500);
    }
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
        String x = "Puls: ";
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
        if(digitalRead(BUTTON_ACCEPT) == HIGH)
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
        }
        if(digitalRead(BUTTON_NEXT) == HIGH)
        {
          if(mp3.turnOn == false)
          {
            if(mp3.genre == 4)
            {
              mp3.genre = 1;
            }
            else
            {
              mp3.genre += 1;
            }
          }
          Serial.println(mp3.turnOn);
          Serial.println(mp3.genre);
        }
        String x = "Rodzaj muzyki:\n";
        switch(mp3.genre)
        {
          case 1: x += "PRACA KLASYCZNA"; break;
          case 2: x += "PRACA LOFI"; break;
          case 3: x += "RELAKS NATURA"; break;
          case 4: x += "RELAKS SOUNDTRACK"; break;
        }
        x += "\n";
        x += "Status odtwarzania:\n";
        if(mp3.turnOn) x += "ODTWARZANIE";
        else x += "STOP";
        showLCD(x);
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
