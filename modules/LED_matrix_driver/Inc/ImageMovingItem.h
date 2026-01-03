#ifndef _IMAGE_MOVING_ITEM_H_
#define _IMAGE_MOVING_ITEM_H_

#include <inttypes.h>
#include "Colors.h"
#include "LedMatrixConst.h"

// stuktura opisująca maksymalne przsunięcie obiektu w dwóch osiach
typedef struct{
    // maksymalne przesunięcie w pikselach na osi X
    uint16_t maxOffsetX;
    // maksymalne przesunięcie w pikselach na osi Y
    uint16_t maxOffsetY;
}MoveRange;

// struktura opisująca zakres danych wejściowych opisujących położenie
typedef struct{
    // wartość dla minimalnego wychylenia X
    float minX;
    // wartość dla maksymalnego wychylenia X
    float maxX;
    // wartość dla minimalnego wychylenia Y
    float minY;
    // wartość dla maksymalnego wychylenia Y
    float maxY;
}PositionDataRange;

// obiekt dynamicznego elementu orazu
typedef struct{
    // wskaźnik na dane maski obiektu w zerowym wychyleniu
    const uint8_t (*ItemMask)[COLUMNS];
    // kolor elementu
    COLOR Color;
    // maksymalne wychylenia
    MoveRange MoveLimts;
    // zakres danych wejściowych
    PositionDataRange InputRange;
    // obecne wychylenie obiektu w osi X
    uint16_t OffsetX;
    // obecne wychylenie obiektu w osi Y
    uint16_t OffsetY;
    // wartość danych wejściowych potrzebna do przesunięcia elementu o jeden piksel w osi X
    float ShiftUnitX;
    // wartość danych wejściowych potrzebna do przesunięcia elementu o jeden piksel w osi Y
    float ShiftUnitY;
}ImageMovingItem;


/**
 * @brief inicjalizuje obiekt
 * @param self wskaźnik na obiekt
 * @param newItemMask wskaźnik na dane maski obiektu w zerowym wychyleniu
 * @param newMoveLimits maksymalne wychylenia
 * @param newInputRange zakresy danych wejściowych
 * @param newColor color elementu
 */
void ImageMovingItem_Init(ImageMovingItem * self, const uint8_t (*newItemMask)[COLUMNS], MoveRange newMoveLimits, PositionDataRange newInputRange, COLOR newColor);

/**
 * @brief zmienia wychylenie obiektu na podstawie danych wejściowych
 * @param self wskaźnik na obiekt
 * @param inputX dane wejściowe dla osi X
 * @param inputY dane wejściowe dla osi Y
 */
void ImageMovingItem_SetPosition(ImageMovingItem * self, float inputX, float inputY);

/**
 * @brief zmienia wskaźnik na dane maski obiektu w zerowym wychyleniu
 * @param wskaźnik na obiekt
 * @param newItemMask nowy wskaźnik na dane maski obiektu w zerowym wychyleniu
 */
void ImageMovingItem_SetNewItemData(ImageMovingItem * self, const uint8_t (*newItemMask)[COLUMNS]);

#endif /* _IMAGE_MOVING_ITEM_H_ */