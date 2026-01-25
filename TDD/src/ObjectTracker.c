
#include "ObjectTracker.h"
#include <math.h>
#include <string.h>

/* Stałe stały się częścią konfiguracji, ale te pomocnicze zostawiamy */
#define STATUS_RANGE_VALID          5
#define STATUS_RANGE_VALID_LARGE    9

/* Stałe geometryczne */
#define SENSOR_CENTER_X             3.5f  /* Środek między pikselem 3 a 4 (0..7) */
#define SENSOR_RES_X                8.0f  /* Rozdzielczość pozioma */

typedef struct {
    float sum_weight;       // Całkowita "masa" obiektu
    float sum_weight_x;     // Ważona suma X
    float sum_weight_y;     // Ważona suma Y
    float sum_weight_xx;    // Ważona suma kwadratów X (do wariancji)
    float sum_weight_dist;  // Ważona suma odległości
} AnalysisData;

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

static AnalysisData _step1_analyze_frame(ObjectTracker *self, const RangeSensorFrame *frame) {
    AnalysisData result = {0}; // Zerowanie

    for (int i = 0; i < RS_TOTAL_ZONES; i++) {
        int x = i % (int)SENSOR_RES_X;
        int y = i / (int)SENSOR_RES_X;

        uint16_t dist = frame->distances_mm[i];
        uint8_t stat  = frame->statuses[i];

        float w = _calculate_pixel_weight(self, dist, stat);

        if (w > 0.001f) {
            float fx = (float)x;
            result.sum_weight      += w;
            result.sum_weight_x    += w * fx;
            result.sum_weight_y    += w * (float)y;
            result.sum_weight_xx   += w * (fx * fx); // Suma wag * x^2
            result.sum_weight_dist += w * (float)dist;
        }
    }
    return result;
}

static bool _step2_check_detection(const ObjectTracker *self, const AnalysisData *data) {
    // 1. Sprawdź masę
    if (data->sum_weight < self->config->min_mass_detect) {
        return false;
    }

    // 2. Sprawdź wariancję (rozproszenie)
    // Var(X) = E[X^2] - (E[X])^2
    float mean_x = data->sum_weight_x / data->sum_weight;
    float mean_sq_x = data->sum_weight_xx / data->sum_weight;
    float variance = mean_sq_x - (mean_x * mean_x);

    // Zabezpieczenie przed ujemną wariancją (błędy numeryczne float)
    if (variance < 0.0f) variance = 0.0f;

    if (variance > self->config->max_dispersion) {
        return false; // Obiekt zbyt rozproszony (duch)
    }

    return true;
}

static void _step3_update_state(ObjectTracker *self, const AnalysisData *data, bool is_detected) {
    
    // 1. Oblicz surową pewność
    float raw_prob = data->sum_weight / (self->config->min_mass_detect * 2.0f);
    if (raw_prob > 1.0f) raw_prob = 1.0f;

    // 2. Logika Detekcji i Podtrzymania (Hysteresis)
    if (is_detected) {
        // Obiekt widoczny - reset licznika
        self->_frames_lost = 0;

        float raw_x    = data->sum_weight_x / data->sum_weight;
        float raw_y    = data->sum_weight_y / data->sum_weight;
        float raw_dist = data->sum_weight_dist / data->sum_weight;

        float alpha = self->config->smooth_factor;
        
        self->target.position_x  = _low_pass_filter(raw_x,    self->_prev_x,    alpha);
        self->target.position_y  = _low_pass_filter(raw_y,    self->_prev_y,    alpha);
        self->target.distance_mm = _low_pass_filter(raw_dist, self->_prev_dist, alpha);
        
        float deg_per_pixel = self->config->sensor_fov_deg / SENSOR_RES_X;
        self->target.angle_deg = (self->target.position_x - SENSOR_CENTER_X) * deg_per_pixel;
        
        // Zapisz historię
        self->_prev_x    = self->target.position_x;
        self->_prev_y    = self->target.position_y;
        self->_prev_dist = self->target.distance_mm;

        self->target.is_detected = true;
    } else {
        // Obiekt niewidoczny (lub odrzucony)
        self->_frames_lost++;

        if (self->_frames_lost <= self->config->max_hold_frames) {
            // Tryb Podtrzymania (Ghosting)
            // Zwracamy true, ale NIE aktualizujemy pozycji (trzymamy ostatnią znaną)
            self->target.is_detected = true;
        } else {
            // Całkowita utrata
            self->target.is_detected = false;
        }
    }

    // C. Filtrowanie prawdopodobieństwa (zawsze, nawet jak brak obiektu)
    self->target.probability = _low_pass_filter(raw_prob, self->_prev_prob, 0.1f);
    self->_prev_prob = self->target.probability;
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
    self->_frames_lost = 0;
}

void ObjectTracker_Process(ObjectTracker *self, const RangeSensorFrame *input_frame) {
    if (!input_frame->is_valid) return;

    AnalysisData analysis = _step1_analyze_frame(self, input_frame);

    bool detected = _step2_check_detection(self, &analysis);

    _step3_update_state(self, &analysis, detected);
}

/* --- GETTY --- */

bool ObjectTracker_IsDetected(const ObjectTracker *self) {
    return self->target.is_detected;
}

float ObjectTracker_GetX(const ObjectTracker *self) {
    return self->target.position_x;
}

float ObjectTracker_GetY(const ObjectTracker *self) {
    return self->target.position_y;
}

float ObjectTracker_GetDistance(const ObjectTracker *self) {
    return self->target.distance_mm;
}

float ObjectTracker_GetAngle(const ObjectTracker *self) {
    return self->target.angle_deg;
}

float ObjectTracker_GetProbability(const ObjectTracker *self) {
    return self->target.probability;
}

void ObjectTracker_Update(ObjectTracker *self) {
    RangeSensorFrame frame;
    if (RangeSensor_GetNewFrame(&frame)) {
        ObjectTracker_Process(self, &frame);
    }
}