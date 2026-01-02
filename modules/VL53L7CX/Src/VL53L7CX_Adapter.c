#include "VL53L7CX_Adapter.h"
#include <string.h>

/* --- Implementacja Konstruktora --- */
void VL53L7CX_Adapter_Construct(RangeSensor *self, I2C_HandleTypeDef *hi2c, uint16_t address) {
    memset(self, 0, sizeof(RangeSensor));
    self->dev.platform.address = address;
    self->dev.platform.hi2c_handle = hi2c; 
    self->is_ranging = 0;
    self->initialization_done = 0;
}

/* --- Implementacja Interfejsu RangeSensor --- */

RangeSensor_Status RangeSensor_Init(RangeSensor *self) {
    uint8_t status;
    uint8_t is_alive = 0;

    // Krok 1: Ping
    status = vl53l7cx_is_alive(&self->dev, &is_alive);
    if (status != VL53L7CX_STATUS_OK || !is_alive) {
        return RS_ERROR_HW_FAILURE;
    }

    // Krok 2: Init Firmware
    status = vl53l7cx_init(&self->dev);
    if (status != VL53L7CX_STATUS_OK) {
        return RS_ERROR_SENSOR_INIT;
    }

    // Krok 3: Konfiguracja parametrów
    status = vl53l7cx_set_resolution(&self->dev, VL53L7CX_RESOLUTION_8X8);
    if (status != VL53L7CX_STATUS_OK) return RS_ERROR_PARAM;

    status = vl53l7cx_set_ranging_mode(&self->dev, VL53L7CX_RANGING_MODE_CONTINUOUS);
    if (status != VL53L7CX_STATUS_OK) return RS_ERROR_PARAM;

    status = vl53l7cx_set_ranging_frequency_hz(&self->dev, 15);
    if (status != VL53L7CX_STATUS_OK) return RS_ERROR_PARAM;

    self->initialization_done = 1;
    return RS_OK;
}

RangeSensor_Status RangeSensor_Start(RangeSensor *self) {
    if (!self->initialization_done) return RS_ERROR_SENSOR_INIT;

    uint8_t status = vl53l7cx_start_ranging(&self->dev);
    if (status != VL53L7CX_STATUS_OK) {
        return RS_ERROR_HW_FAILURE;
    }
    
    self->is_ranging = 1;
    return RS_OK;
}

RangeSensor_Status RangeSensor_Stop(RangeSensor *self) {
    uint8_t status = vl53l7cx_stop_ranging(&self->dev);
    if (status != VL53L7CX_STATUS_OK) {
        return RS_ERROR_HW_FAILURE;
    }

    self->is_ranging = 0;
    return RS_OK;
}

RangeSensor_Status RangeSensor_GetFrame(RangeSensor *self, RangeSensorFrame *out_frame) {
    if (!self->is_ranging) {
        out_frame->is_valid = false;
        return RS_ERROR_DATA_NOT_READY;
    }

    uint8_t status;
    uint8_t ready = 0;

    // 1. Sprawdź gotowość danych
    status = vl53l7cx_check_data_ready(&self->dev, &ready);
    if (status != VL53L7CX_STATUS_OK) {
        out_frame->is_valid = false;
        return RS_ERROR_HW_FAILURE; 
    }

    if (ready) {
        // 2. Pobierz dane
        status = vl53l7cx_get_ranging_data(&self->dev, &self->st_results);
        if (status != VL53L7CX_STATUS_OK) {
            out_frame->is_valid = false;
            return RS_ERROR_DATA_CORRUPTED;
        }

        // 3. Mapowanie danych (Impedance Matching)
        for (int i = 0; i < RS_TOTAL_ZONES; i++) {
            out_frame->distances_mm[i] = self->st_results.distance_mm[i];
            out_frame->statuses[i]     = self->st_results.target_status[i];
        }
        
        out_frame->is_valid = true;
        return RS_OK; 
    }

    // Brak nowych danych (to nie jest błąd krytyczny)
    out_frame->is_valid = false;
    return RS_ERROR_DATA_NOT_READY;
}