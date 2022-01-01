#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "DHT.h"
#include "SoftwareSerial.h"
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#define REPORTING_PERIOD_MS     1000
#define DHTPIN 4
#define DHTTYPE DHT11

const int SZEROKOSC = 128;
const int WYSOKOSC  = 64;

SoftwareSerial mySoftwareSerial(1, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 ekranik(SZEROKOSC, WYSOKOSC, &Wire, -1);

PulseOximeter pox;

uint32_t tsLastReport = 0;

const int PIN14 = 14;
const int PIN16 = 16;
const int PIN25 = 25;
const int PIN34 = 34;
const int PIN5 = 5;
const int PIN12 = 12;
int status = 0;
int poziom1 = 0;
int poziom2 = 0;

String listaTrybow [4] = {"1) Temperatura", "2) Muzyka", "3) Tryb pracy", "4) Pulsometr"};

void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);
    //Inicjalizacja ekranu
    if(!ekranik.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
    {
        Serial.println("Błąd inicjalizacji wyświetlacza!");
        exit(0);
    }
    ekranik.setTextColor(WHITE);
    ekranik.cp437(true);

    //Inicjalizacja pulsometru
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else 
    {
        Serial.println("SUCCESS");
    }

    //Inicjalizacja temperatury
    dht.begin();
    
    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);

    mySoftwareSerial.begin(9600);
    if (!myDFPlayer.begin(mySoftwareSerial)) 
    {  //Use softwareSerial to communicate with mp3.
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        while(true);
    }
    myDFPlayer.setTimeOut(500);
    myDFPlayer.volume(10);
    myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
}

String pulsometr()
{
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        tsLastReport = millis();
        return "Heart rate:" + String(pox.getHeartRate()) + "bpm\nSpO2:" + String(pox.getSpO2()) + "%";
    }
}

String temperatura()
{
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return "";
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
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
  return "Temperature: " + String(t) + "°C \nHumidity: " + String(h) + "\nHeat index: " + String(hif);
}

void muzyka()
{
   Serial.println("X");
}

void pokazEkranu(String x)
{
    ekranik.setTextSize(1);
    ekranik.setCursor(10, 0);
    ekranik.print(x);
    ekranik.display();
    delay(250);
}

void loop()
{
    if(digitalRead(PIN25) == LOW)
    {
        poziom2 += 1;
        if(poziom2 == 4)
        {
            poziom2 = 0;
        }
    }
    if(digitalRead(PIN14) == LOW)
    {
        if(poziom1 == 0)
        {
            poziom1 += 1;
        }
    }
    if(poziom1 == 0)
    {
        pokazEkranu(listaTrybow[poziom2]);
    }
    else
    {
        switch (poziom2)
        {
            case 1:
            {
                pokazEkranu(temperatura());
                break;
            }
            case 2:
            {
                muzyka();
                break;
            }
            case 3:
            {
                pokazEkranu(pulsometr());
                break;
            }
        }
    }
}
