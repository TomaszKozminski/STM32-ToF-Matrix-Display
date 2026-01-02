#ifndef _IMAGE_LIBRARY_H
#define _IMAGE_LIBRARY_H

#include <inttypes.h>
#include "LedMatrixConst.h"


#define LIB_IMAGES_CNT             3
typedef enum{
    SIMPLE,
    CUTE,
    CYCLOP
}LIB_IMAGE;

extern const uint8_t (*MovingItemLibrary[])[COLUMNS];
extern const uint8_t (*BackgroundLibrary[])[COLUMNS];
extern const uint8_t ItemOffsets[][2];

#endif /* _IMAGE_LIBRARY_H */