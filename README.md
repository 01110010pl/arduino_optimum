# Arduino Optimum Project

<p align="center"><img src="readme_files/file1.png" alt="drawing" width="250"/></p>

<p align="center"><img src="readme_files/file2.gif" alt="drawing" width="250"/></p>

## Elementy

- 1x MAX30100/RCWL-0530 (Pulsometr)
- 1x DHT11 (Czujnik temperatury i wilgoci)
- 1x DFPlayer Mini (Odtwarzacz kart/muzyki)
- 1x ESP32 DEVkit
- 1x Karta microSD
- 1x SSD1306 (Ekran OLED)
- 1x PAM8403/GF1002 (Wzmacniacz)
- 3x Diody LED (czerwona, niebieska, zielona)
- 3x Przyciski
- 1x Głośnik YD30
- 2x Rezystor 470R
- 4x Rezystor 10k
- 2x Rezystor 2.7k

## Co działa, a co nie działa?

- [x] Temperatura
- [x] Tryb światła
- [x] Ekran
- [x] Przyciski
- [x] Menu
- [ ] Tryb pracy
- [ ] Pulsometr
- [x] Muzyka

## Instrukcja obsługi

Przyciski:

- Czerwony(BUTTON_NEXT): 
    * przełączanie trybów w menu
    * przełączanie opcji w trybach
    * przełączanie rodzajów muzyki w trybie "Muzyka"
- Zółty(BUTTON_ACCEPT)
    * Wchodzenie w dany, wybrany tryb
    * Zatrzymywanie/Odtwarzanie muzyki
    * Włączanie i wyłączanie światła
- Niebieski(BUTTON_EXIT)
    * Wychodzenie z trybów

## Schemat

[Schemat jest pod tym linkiem](/schematic_diagram/optimum-project.pdf) (NIE AKTUALNY!)