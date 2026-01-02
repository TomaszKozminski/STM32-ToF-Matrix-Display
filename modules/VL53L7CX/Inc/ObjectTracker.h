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

void ObjectTracker_Process(ObjectTracker *self, const RangeSensorFrame *input_frame);

#endif // OBJECT_TRACKER_H