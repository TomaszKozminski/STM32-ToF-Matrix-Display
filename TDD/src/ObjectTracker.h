#ifndef OBJECT_TRACKER_H
#define OBJECT_TRACKER_H

#include <stdint.h>
#include <stdbool.h>
#include "RangeSensor.h" 

/* --- Struktura Wynikowa (Output) --- */
typedef struct {
    bool is_detected;       // Czy wykryto obiekt?
    float probability;      // Pewność (0.0 - 1.0)
    float position_x;       // Pozycja X (0.0 - 7.0)
    float position_y;       // Pozycja Y (0.0 - 7.0)
    float angle_deg;        // Kąt w stopniach
    float distance_mm;      // Odległość
} TrackedObject;

/* --- Struktura Konfiguracyjna (Tylko Odczyt / Flash) --- */
typedef struct {
    /* Odległości graniczne filtru */
    float min_dist_mm;       // Np. 200 mm
    float max_dist_mm;       // Np. 2000 mm
    
    /* Strefa miękkiego przejścia (Trapezoid) */
    float fade_dist_mm;      // Np. 300 mm
    
    /* Progi detekcji ("Masa dowodowa") */
    float min_mass_detect;   // Np. 3.5
    
    /* Parametr wygładzania (Low Pass Filter) */
    float smooth_factor;     // Np. 0.15
    
    /* Maksymalne dopuszczalne rozproszenie (wariancja) obiektu */
    float max_dispersion;    // Np. 2.0 (jeśli więcej, to "duch" lub szum)
    
    /* Czas podtrzymania detekcji po utracie sygnału (Hysteresis) */
    uint16_t max_hold_frames; // Np. 5 klatek

    /* Parametry geometryczne czujnika (do obliczania kąta) */
    float sensor_fov_deg;    // Np. 45.0
} ObjectTrackerConfig;

/* --- Główny Obiekt Trackera (Context) --- */
typedef struct {
    /* --- Stan Publiczny (Wyniki) --- */
    TrackedObject target;

    /* --- Stan Wewnętrzny (Prywatny) --- */
    float _prev_x;
    float _prev_y;
    float _prev_dist;
    float _prev_prob;
    
    uint16_t _frames_lost; // Licznik klatek od ostatniej detekcji

    /* Wskaźnik do konfiguracji (przechowywanej we Flashu) */
    const ObjectTrackerConfig *config;

} ObjectTracker;

/* --- Metody --- */

/**
 * @brief Inicjalizuje tracker.
 * @param self Wskaźnik do instancji trackera (RAM).
 * @param config Wskaźnik do stałej konfiguracji (Flash).
 */
void ObjectTracker_Init(ObjectTracker *self, const ObjectTrackerConfig *config);

/**
 * @brief Główna funkcja algorytmu. Przetwarza nową klatkę danych z czujnika.
 * Wykonuje analizę wagową (Center of Mass), decyduje o obecności obiektu
 * i aktualizuje filtry wygładzające pozycję oraz odległość.
 * * @param self Wskaźnik do instancji trackera.
 * @param input_frame Wskaźnik do surowych danych pobranych z czujnika (RangeSensorFrame).
 */
void ObjectTracker_Process(ObjectTracker *self, const RangeSensorFrame *input_frame);

/**
 * @brief Sprawdza status detekcji obiektu.
 * * @param self Wskaźnik do instancji trackera.
 * @return true - jeśli obiekt został wykryty i spełnia kryteria (masa > próg).
 * @return false - jeśli brak obiektu lub sygnał jest zbyt słaby (traktowany jako szum).
 */
bool  ObjectTracker_IsDetected(const ObjectTracker *self);

/**
 * @brief Pobiera aktualną, przefiltrowaną pozycję X obiektu (współrzędna pozioma).
 * Wartość odnosi się do siatki czujnika (np. 0.0 - 7.0 dla sensora 8x8).
 * * @param self Wskaźnik do instancji trackera.
 * @return float Pozycja X (gdzie 3.5 to środek).
 */
float ObjectTracker_GetX(const ObjectTracker *self);

/**
 * @brief Pobiera aktualną, przefiltrowaną pozycję Y obiektu (współrzędna pionowa).
 * Wartość odnosi się do siatki czujnika (np. 0.0 - 7.0 dla sensora 8x8).
 * * @param self Wskaźnik do instancji trackera.
 * @return float Pozycja Y (gdzie 3.5 to środek).
 */
float ObjectTracker_GetY(const ObjectTracker *self);

/**
 * @brief Pobiera aktualną odległość do środka ciężkości obiektu.
 * Wartość jest wygładzona filtrem dolnoprzepustowym.
 * * @param self Wskaźnik do instancji trackera.
 * @return float Odległość w milimetrach [mm].
 */
float ObjectTracker_GetDistance(const ObjectTracker *self);

/**
 * @brief Oblicza kąt odchylenia obiektu od osi optycznej czujnika.
 * Kąt jest wyliczany na podstawie pozycji X i pola widzenia (FOV) czujnika.
 * * @param self Wskaźnik do instancji trackera.
 * @return float Kąt w stopniach (ujemny w lewo, dodatni w prawo).
 */
float ObjectTracker_GetAngle(const ObjectTracker *self);

/**
 * @brief Zwraca poziom pewności (wiarygodności) detekcji.
 * Jest to znormalizowana wartość określająca siłę sygnału odbitego od obiektu.
 * * @param self Wskaźnik do instancji trackera.
 * @return float Wartość od 0.0 (brak pewności) do 1.0 (pełna pewność).
 */
float ObjectTracker_GetProbability(const ObjectTracker *self);

/**
 * @brief Aktualizuje stan trackera, pobierając dane z czujnika.
 * Funkcja ta łączy się z zewnętrznym modułem RangeSensor.
 * @param self Wskaźnik do instancji trackera.
 */
void ObjectTracker_Update(ObjectTracker *self);

#endif // OBJECT_TRACKER_H