#ifndef VL53L7CX_ADAPTER_H
#define VL53L7CX_ADAPTER_H

/* --- Includes --- */
#include "RangeSensor.h"       // Implementujemy ten interfejs
#include "vl53l7cx_api.h"      // Używamy tego sterownika (Hardware Proxy)
#include "stm32l4xx_hal.h"     // Potrzebne do definicji I2C_HandleTypeDef

/* --- Definicja Struktury Implementacyjnej --- */
/* * To jest "wnętrze" typu opaque zdefiniowanego w RangeSensor.h.
 * Tylko kod adaptera (.c) widzi te pola. Reszta systemu widzi tylko wskaźnik.
 */
struct RangeSensor_t {
    /* * 1. Hardware Proxy Instance
     * To jest główna struktura sterownika ST. Zawiera konfigurację,
     * kalibrację oraz strukturę 'platform' (adres I2C).
     */
    VL53L7CX_Configuration dev;

    /* * 2. Bufor Pośredni (Proxy Output)
     * Biblioteka ST zwraca dane w bardzo rozbudowanej strukturze (kilkaset bajtów).
     * Musimy ją tutaj trzymać, aby potem przepisać z niej dane do lżejszej 'RangeSensorFrame'.
     */
    VL53L7CX_ResultsData st_results;

    /* * 3. Stan Adaptera
     * Pola pomocnicze do zarządzania logiką adaptera.
     */
    uint8_t is_ranging;         // Flaga: 1 = czujnik aktywnie mierzy, 0 = stop
    uint8_t initialization_done; // Flaga: zabezpieczenie przed użyciem bez Init
};

/* --- Konstruktor Specyficzny dla Adaptera --- */

/**
 * @brief Konfiguruje wstępnie strukturę adaptera, wiążąc ją z konkretnym I2C.
 * Ta funkcja NIE komunikuje się jeszcze z czujnikiem (to robi RangeSensor_Init).
 * Służy tylko do ustawienia wskaźników (Dependency Injection).
 * @param self Wskaźnik do instancji sensora (pamięć musi być już zaalokowana).
 * @param hi2c Wskaźnik do uchwytu I2C (HAL) - np. &hi2c2.
 * @param address Adres I2C czujnika (zazwyczaj 0x52).
 */
void VL53L7CX_Adapter_Construct(RangeSensor *self, I2C_HandleTypeDef *hi2c, uint16_t address);

#endif // VL53L7CX_ADAPTER_H