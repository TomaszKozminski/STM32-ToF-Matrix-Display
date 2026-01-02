#ifndef _IMAGE_BACKGROUND_H_
#define _IMAGE_BACKGROUND_H_

#include <inttypes.h>
#include "Colors.h"
#include "LedMatrixConst.h"

// obiekt tła obrazu
typedef struct{
    // wskaźnik na tablice wskaźników szerokości matrycy
    const uint8_t (*BackgroundMask)[COLUMNS];
    // kolor tła
    COLOR Color;
}ImageBackground;

/**
 * @brief inicjalizacja obiektu
 * @param self wskaźnik na obiekt
 * @param newBackgroundMask wskaźnik na dane maski tła
 * @param newColor kolor tła
 */
void ImageBackground_Init(ImageBackground * self, const uint8_t (*newBackgroundMask)[COLUMNS], COLOR newColor);

/**
 * @param zmienia kolor tła
 * @param self wskaźnik na obiekt
 * @param newColor nowy kolor tła
 */
void ImageBackground_SetColor(ImageBackground * self, COLOR newColor);

/**
 * @param zmienia zmienia wskaźnik danych maski tła
 * @param self wskaźnik na obiekt
 * @param newBackgroundMask wskaźnik na nowe dane maski tła
 */
void ImageBackground_SetNewBackgroundData(ImageBackground * self, const uint8_t (*newBackgroundMask)[COLUMNS]);

#endif /* _IMAGE_BACKGROUND_H_ */