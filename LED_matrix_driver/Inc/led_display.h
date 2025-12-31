/*
 * led_display.h
 *
 *  Created on: Jul 27, 2025
 *      Author: abyss
 */

#ifndef INC_LED_DISPLAY_H_
#define INC_LED_DISPLAY_H_

#include "main.h"
#include <stdint.h>
#include "cmsis_os.h"

#include "font.h"
#include "colors.h"
#include <stdbool.h>

#define ADDRESSABLE_ROWS                    32/2
#define COLUMNS                 64
#define PIXELS                  ADDRESSABLE_ROWS * COLUMNS

#define ROWS         32
#define FONT_ROWS_SEPARATION    1

#define COLOR_PINS (R1_Pin | R2_Pin | G1_Pin | G2_Pin | B1_Pin | B2_Pin)
#define COLOR_PINS_CLEAR (COLOR_PINS << 16)

// bits per each color
#define BCM_BIT_DEPTH           4


typedef enum{
    RIGHT,
    LEFT
}SLIDE_DIRECTION;

typedef enum{
    TEXT_LANE_1 = 1,
    TEXT_LANE_2 = 7,
    TEXT_LANE_3 = 13,
    TEXT_LANE_4 = 19,
    TEXT_LANE_5 = 25
}TEXT_LANE;

typedef struct LED_frame{
    SemaphoreHandle_t Mutex;
    // 444 12bit RGB values
    uint16_t ** RGB_data;
    // BSSR register bitmask setting and reseting adequate
    // pins to display pixels, doesnt need to reset all pins
    // after each pixel if used to display frame
    uint32_t * register_data;
}LED_frame;
// standalone display function that displays raw register data
void LED_slide(const uint32_t * register_data);


// @brief initializes LED_objects necessities
int LED_Init(LED_frame * self);

void LED_ResetCanvas(LED_frame * self);
void LED_DestroyCanvas(LED_frame * self);
void LED_ClearLane(LED_frame * self, TEXT_LANE text_lane);
// @brief takes data from RGB_data and transforms it into BSRR register bitmasks for each pixel
// making a picture, each row is saved BCM_BIT_DEPTH times to implement image colors, BSRR
// data is then compared to the previous state of BSSR register and creates 32bit(BSSR size)
// bitmask that makes pin changes to achieve new BSSR state, data is saved in self->register_data,
// prepared this way data is ready for frame display utilizing dma
void LED_PrepareFrame(LED_frame * self);

// @biref displays data using data saved in self->register_change utilizing DMA
// for data transfer
void LED_DisplayFrame(LED_frame * self);

// 0 - displayed, 1 - displayed & end reached, 2 - displayed & beginning reched, 3+ error
int  LED_TextCreateLane(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, int16_t pixel_shift, COLOR color);
void LED_TextBounce(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, COLOR color, uint8_t speed);
void LED_TextSlide(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, COLOR color, uint8_t speed, SLIDE_DIRECTION direction, bool loop);
void LED_TextStatic(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, COLOR color);

#endif /* INC_LED_DISPLAY_H_ */
