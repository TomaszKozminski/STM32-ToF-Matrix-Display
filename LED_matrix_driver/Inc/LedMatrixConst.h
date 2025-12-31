#ifndef _LED_MATRIX_CONST_H
#define _LED_MATRIX_CONST_H

#include <inttypes.h>
#include "main.h"

#define ADDRESSABLE_ROWS        16
#define COLUMNS                 64
#define PIXELS                  ADDRESSABLE_ROWS * COLUMNS

#define ROWS                    32

#define COLOR_PINS (R1_Pin | R2_Pin | G1_Pin | G2_Pin | B1_Pin | B2_Pin)
#define COLOR_PINS_CLEAR (COLOR_PINS << 16)

// bits per each color
#define BCM_BIT_DEPTH           4




#endif /* _LED_MATRIX_CONST_H */