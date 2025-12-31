#include "ObjectTracker.h"
#include <math.h>
#include <string.h>

/* Stałe stały się częścią konfiguracji, ale te pomocnicze zostawiamy */
#define STATUS_RANGE_VALID          5
#define STATUS_RANGE_VALID_LARGE    9

/* Stałe geometryczne */
#define SENSOR_CENTER_X             3.5f  /* Środek między pikselem 3 a 4 (0..7) */
#define SENSOR_RES_X                8.0f  /* Rozdzielczość pozioma */

/* --- Funkcje Pomocnicze (Prywatne) --- */

static float _calculate_pixel_weight(ObjectTracker *self, uint16_t dist_mm, uint8_t status) {
    // 1. Odrzuć błędy sprzętowe
    if (status != STATUS_RANGE_VALID && status != STATUS_RANGE_VALID_LARGE) {
        return 0.0f;
    }
    if (dist_mm == 0) return 0.0f;

    // Pobieramy parametry z CONFIGU (przez wskaźnik)
    float d = (float)dist_mm;
    float min = self->config->min_dist_mm;
    float max = self->config->max_dist_mm;
    float fade = self->config->fade_dist_mm;

    // 3. Poza zakresem
    if (d < min || d > max) return 0.0f;

    // 4. Fade In
    if (d < (min + fade)) return (d - min) / fade;

    // 5. Fade Out
    if (d > (max - fade)) return (max - d) / fade;

    // 6. Środek
    return 1.0f;
}

static float _low_pass_filter(float current, float target, float alpha) {
    if (isnan(current)) return target;
    return current * (1.0f - alpha) + target * alpha;
}

/* --- Implementacja API --- */

void ObjectTracker_Init(ObjectTracker *self, const ObjectTrackerConfig *config) {
    // 1. Zerowanie pamięci RAM (stanu)
    memset(self, 0, sizeof(ObjectTracker));

    // 2. Przypisanie wskaźnika do konfiguracji (Flash)
    self->config = config;

    // 3. Inicjalizacja filtrów (Startujemy od środka)
    self->_prev_x    = SENSOR_CENTER_X;
    self->_prev_y    = SENSOR_CENTER_X;
    self->_prev_dist = 0.0f;
    self->_prev_prob = 0.0f;
}

void ObjectTracker_Process(ObjectTracker *self, const RangeSensorFrame *input_frame) {
    if (!input_frame->is_valid) return;

    float sum_weight = 0.0f;
    float sum_weight_x = 0.0f;
    float sum_weight_y = 0.0f;
    float sum_weight_dist = 0.0f;

    /* --- KROK 1: Analiza Obrazu --- */
    for (int i = 0; i < RS_TOTAL_ZONES; i++) {
        // Obliczamy X i Y z indeksu liniowego (zakładając układ wierszami)
        int x = i % (int)SENSOR_RES_X;
        int y = i / (int)SENSOR_RES_X;

        uint16_t dist = input_frame->distances_mm[i];
        uint8_t stat  = input_frame->statuses[i];

        float w = _calculate_pixel_weight(self, dist, stat);

        if (w > 0.001f) {
            sum_weight      += w;
            sum_weight_x    += w * (float)x;
            sum_weight_y    += w * (float)y;
            sum_weight_dist += w * (float)dist;
        }
    }

    /* --- KROK 2: Logika Decyzyjna --- */
    // Używamy progu z configu
    float raw_prob = sum_weight / (self->config->min_mass_detect * 2.0f); 
    if (raw_prob > 1.0f) raw_prob = 1.0f;

    bool is_detected_now = (sum_weight >= self->config->min_mass_detect);

    /* --- KROK 3: Aktualizacja Stanu --- */
    if (is_detected_now) {
        float raw_x    = sum_weight_x / sum_weight;
        float raw_y    = sum_weight_y / sum_weight;
        float raw_dist = sum_weight_dist / sum_weight;

        // Filtrowanie (parametr alpha z configu)
        float alpha = self->config->smooth_factor;
        
        self->target.position_x  = _low_pass_filter(raw_x,    self->_prev_x,    alpha);
        self->target.position_y  = _low_pass_filter(raw_y,    self->_prev_y,    alpha);
        self->target.distance_mm = _low_pass_filter(raw_dist, self->_prev_dist, alpha);
        
        // Kąt = (Odchylenie od środka w pikselach) * (Stopnie na piksel)
        // Dla FOV 45 stopni i 8 pikseli: 1 piksel = 5.625 stopnia.
        float deg_per_pixel = self->config->sensor_fov_deg / SENSOR_RES_X;
        self->target.angle_deg = (self->target.position_x - SENSOR_CENTER_X) * deg_per_pixel;
        
        // Historia
        self->_prev_x    = self->target.position_x;
        self->_prev_y    = self->target.position_y;
        self->_prev_dist = self->target.distance_mm;

        self->target.is_detected = true;
    } else {
        self->target.is_detected = false;
        // TODO: reset/ghosting
    }

    self->target.probability = _low_pass_filter(raw_prob, self->_prev_prob, 0.1f);
    self->_prev_prob = self->target.probability;
}