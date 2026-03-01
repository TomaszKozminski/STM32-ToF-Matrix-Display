# STM32-ToF-Matrix-Display (Projekt "Ciche Oczy")

Zaawansowany system wbudowany łączący wielostrefowy czujnik odległości (Time-of-Flight) z szybkim sterowaniem matrycą LED. System śledzi obiekty w przestrzeni 3D i wizualizuje interakcje w czasie rzeczywistym.

## 🚀 Przegląd

Projekt zrealizowany w ramach inicjatywy "Ciche Oczy" na Politechnice Warszawskiej, oparty na autorskiej płytce PCB z mikrokontrolerem **STM32L476**. Wykorzystuje 64-strefowy (8x8) czujnik **VL53L7CX** do wykrywania obecności i ruchu osób, płynnie przekładając te dane na animacje wyświetlane na **matrycy LED RGB 64x32**.

### Kluczowe cechy:
- **Śledzenie obiektów w czasie rzeczywistym:** Algorytm oparty na obliczaniu środka ciężkości (centroid) do określania pozycji i odległości obiektu.
- **Wydajny sterownik wyświetlacza:** Autorski sterownik HUB75 wykorzystujący **DMA i Timery** oraz **Modulację Kodową (BCM - Binary Code Modulation)**, zapewniający 12-bitową głębię kolorów (RGB444).
- **Modułowa architektura:** Wyraźny podział na warstwy: sterowniki, oprogramowanie pośredniczące (middleware) oraz logikę aplikacji.
- **Projekt gotowy do wdrożenia:** W pełni zaprojektowana dwuwarstwowa płytka PCB oraz obszerna dokumentacja techniczna.

## 🛠 Technologie i narzędzia

- **Mikrokontroler:** STM32L476RGT6 (Cortex-M4, 80MHz)
- **Czujniki:** VL53L7CX (Wielostrefowy czujnik Time-of-Flight 8x8)
- **Wyświetlacz:** Matryca LED RGB 64x32 (interfejs HUB75)
- **System operacyjny:** FreeRTOS
- **Komunikacja:** ESP32 (integracja przez UART dla funkcji IoT/Web API)
- **Pamięć masowa:** Karta MicroSD (SPI + FatFs) do przechowywania animacji
- **Języki:** C (Embedded), Python (Generowanie zasobów graficznych)
- **Narzędzia:** CMake, STM32CubeHAL, Ceedling (TDD), Altium (PCB)

## 🏗 Architektura oprogramowania

Projekt wykorzystuje architekturę warstwową, co zapewnia jego testowalność i łatwość w utrzymaniu:

1.  **Warstwa aplikacji (Application Layer):** Wątki FreeRTOS obsługujące logikę wyższego poziomu i synchronizację.
2.  **Warstwa logiki (Logic Layer):** Algorytmy śledzenia obiektów oraz mapowania animacji.
3.  **Oprogramowanie pośredniczące (Middleware):** FatFs do obsługi systemu plików i FreeRTOS do szeregowania zadań.
4.  **Warstwa abstrakcji sprzętu (HAL):** Dedykowane sterowniki dla matrycy LED (oparte na DMA), czujnika TOF oraz karty SD.

### Główne moduły
- `LED_matrix_driver`: Niskopoziomowa implementacja interfejsu HUB75 wykorzystująca TIM+DMA dla odświeżania matrycy bez obciążania procesora (zero-CPU-overhead).
- `VL53L7CX`: Implementacja sterownika ST Ultra Lite Driver (ULD) zaadaptowana na potrzeby projektu.
- `ObjectTracker`: Moduł analityczny przetwarzający macierze odległości 8x8 na współrzędne ekranowe.
- `SD_driver`: Niezawodna komunikacja SPI z kartą MicroSD.

## 🧪 Zapewnienie jakości i TDD (Test-Driven Development)

W projekcie zastosowano podejście **Test-Driven Development (TDD)** z wykorzystaniem frameworka Ceedling (Unity/CMock). Kluczowa logika, taka jak `ObjectTracker`, jest w pełni pokryta testami jednostkowymi, co gwarantuje niezawodność przed wdrożeniem na sprzęt.

```bash
# Struktura katalogów TDD
TDD/
├── src/   # Testowany kod źródłowy
└── test/  # Testy jednostkowe
```

## 🎨 Generowanie zasobów graficznych

W folderze `imageMakerPython/` znajduje się skrypt narzędziowy konwertujący standardowe obrazy na autorski format `.ANI` używany przez wbudowany czytnik animacji, co ułatwia proces tworzenia nowych grafik.

## 📸 Sprzęt

System działa na dedykowanej płytce PCB, zaprojektowanej do montażu bezpośrednio z tyłu matrycy LED. Charakteryzuje się ona:
- Zintegrowanymi konwerterami poziomów logicznych (74HCT) dla pełnej kompatybilności z 5V standardem HUB75.
- Dedykowanymi obwodami zasilania dla diod LED o dużym poborze prądu.
- Kompaktowym rozmiarem dopasowanym do urządzeń z branży IoT.

---

**Autorzy:** Tomasz Koźmiński, Adam Kuryś, Filip Zawadzki
**Uczelnia:** Politechnika Warszawska, Wydział Elektroniki i Technik Informacyjnych.
