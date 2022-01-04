// Biblioteki
#include "DHT.h"
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Derektywy preprocesora
#define DHTPIN 4
#define DHTTYPE DHT11
#define REPORTING_PERIOD_MS     1000

DHT dht(DHTPIN, DHTTYPE);
PulseOximeter pox;
SoftwareSerial mySoftwareSerial(1, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);

uint32_t tsLastReport = 0;

void setup()
{
    Serial.begin(115200);
    
    // MP3
    mySoftwareSerial.begin(9600);
    if (!myDFPlayer.begin(mySoftwareSerial)) 
    {
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        while(true);
    }
    myDFPlayer.setTimeOut(500);
    myDFPlayer.volume(15);
    myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
    myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

    // Temperatura
    dht.begin();

    // Pulsometr
    if (!pox.begin())
    {
        Serial.println("FAILED");
        for(;;);
    } 
    else Serial.println("SUCCESS");
}

String temperatura()
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
    String message = "Temperatura: " + String(t) + "°C";
    x += "\n" + "Wilgotność: " String(h) + "%";
    x += "\n" + "Indeks ciepła: " + String(hic) + "°C/" + String(hif) + "°F";
    return message;
}

String pulsometr()
{
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
    {
        Serial.print("Heart rate:");
        Serial.print();
        Serial.print("bpm / SpO2:");
        Serial.print();
        Serial.println("%");
        tsLastReport = millis();
        Strign message = "";
        message += "Puls: " + String(pox.getHeartRate()) + "bpm";
        message += "\n" + "Sp02: " + String(pox.getSpO2());
        return message;
    }
}

void muzyka()
{
    myDFPlayer.randomAll();
}

void loop()
{

}