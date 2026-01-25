#ifndef RANGE_SENSOR_H
#define RANGE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

#define RS_TOTAL_ZONES 64

typedef struct {
    uint16_t distances_mm[RS_TOTAL_ZONES];
    uint8_t statuses[RS_TOTAL_ZONES];
    bool is_valid;
} RangeSensorFrame;

/**
 * @brief Pobiera najnowszą klatkę z czujnika.
 * @param frame Wskaźnik do struktury, która ma zostać wypełniona.
 * @return true jeśli pobrano nową klatkę, false w przeciwnym razie.
 */
bool RangeSensor_GetNewFrame(RangeSensorFrame *frame);

#endif // RANGE_SENSOR_H
