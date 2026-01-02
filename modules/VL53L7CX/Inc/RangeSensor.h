#ifndef RANGE_SENSOR_H
#define RANGE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

/* --- Konfiguracja Stała Interfejsu --- */
#define RS_RES_X  8
#define RS_RES_Y  8
#define RS_TOTAL_ZONES (RS_RES_X * RS_RES_Y) // 64

/* --- Kody Błędów (Decentralized Error Definitions) --- */
typedef enum {
    RS_OK = 0,
    RS_ERROR_SENSOR_INIT    = -1,
    RS_ERROR_HW_FAILURE     = -2,
    RS_ERROR_TIMEOUT        = -3,
    RS_ERROR_DATA_NOT_READY = -4,
    RS_ERROR_PARAM          = -5,
    RS_ERROR_DATA_CORRUPTED = -6
} RangeSensor_Status;

/* --- Struktura Danych (Czyste Dane dla Aplikacji) --- */
typedef struct {
    // Odległość w milimetrach dla każdej strefy
    // Układ: wierszami (Row-major: index = y * 8 + x)
    uint16_t distances_mm[RS_TOTAL_ZONES];

    // Status pomiaru dla każdej strefy (wg standardu ST: 5 i 9 to OK)
    // Przekazujemy to wyżej, bo ObjectTracker może chcieć filtrować np. błędy "Low Confidence"
    uint8_t statuses[RS_TOTAL_ZONES];
    
    // Flaga pomocnicza - czy ramka jest w ogóle poprawna
    bool is_valid;
    
} RangeSensorFrame;

/* --- Definicja Typu Abstrakcyjnego --- */
// To jest tzw. "Opaque Type" (Nieprzezroczysty Typ).
// Klient (ObjectTracker) widzi tylko wskaźnik 'RangeSensor*'.
// Szczegóły struktury (pola dev, i2c) będą w pliku Adaptera (.h).
typedef struct RangeSensor_t RangeSensor;


/* --- Metody Interfejsu --- */

/**
 * @brief Inicjalizuje sensor (HW reset, ładowanie firmware'u, ustawienia domyślne).
 * @param self Wskaźnik do instancji adaptera.
 * @return RS_OK lub kod błędu.
 */
RangeSensor_Status RangeSensor_Init(RangeSensor *self);

/**
 * @brief Rozpoczyna ciągły pomiar (Ranging).
 * @param self Wskaźnik do instancji adaptera.
 * @return RS_OK lub kod błędu.
 */
RangeSensor_Status RangeSensor_Start(RangeSensor *self);

/**
 * @brief Zatrzymuje pomiar (Low Power).
 * @param self Wskaźnik do instancji adaptera.
 * @return RS_OK lub kod błędu.
 */
RangeSensor_Status RangeSensor_Stop(RangeSensor *self);

/**
 * @brief Pobiera najnowszą ramkę danych (Non-blocking).
 * Funkcja dokonuje też translacji (Impedance Matching) z formatu drivera na format RangeSensorFrame.
 * @param self Wskaźnik do instancji adaptera.
 * @param out_frame Wskaźnik do struktury, gdzie zostaną zapisane dane.
 * @return RS_OK - pobrano nowe dane.
 * RS_ERROR_DATA_NOT_READY - brak nowych danych (to nie jest błąd krytyczny).
 * Inny kod błędu - problem sprzętowy.
 */
RangeSensor_Status RangeSensor_GetFrame(RangeSensor *self, RangeSensorFrame *out_frame);

#endif // RANGE_SENSOR_H