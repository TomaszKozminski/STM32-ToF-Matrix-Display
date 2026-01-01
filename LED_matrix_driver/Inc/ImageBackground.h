#ifndef _IMAGE_BACKGROUND_H_
#define _IMAGE_BACKGROUND_H_

#include <inttypes.h>
#include "Colors.h"
#include "LedMatrixConst.h"


typedef struct{
    const uint8_t (*BackgroundMask)[COLUMNS];
    COLOR Color;
}ImageBackground;

void ImageBackground_Init(ImageBackground * self, const uint8_t (*newBackgroundMask)[COLUMNS], COLOR newColor);
void ImageBackground_SetColor(ImageBackground * self, COLOR newColor);
void ImageBackground_SetNewBackgroundData(ImageBackground * self, const uint8_t (*newBackgroundMask)[COLUMNS]);

#endif /* _IMAGE_BACKGROUND_H_ */