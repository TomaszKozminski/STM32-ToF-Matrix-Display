#ifndef _IMAGE_MOVING_ITEM_H_
#define _IMAGE_MOVING_ITEM_H_

#include <inttypes.h>
#include "Colors.h"
#include "LedMatrixConst.h"

typedef struct{
    uint16_t maxOffsetX;
    uint16_t maxOffsetY;
}MoveRange;

typedef struct{
    float minX;
    float maxX;
    float minY;
    float maxY;
}PositionDataRange;

typedef struct{
    const uint8_t (*ItemMask)[COLUMNS];
    COLOR Color;
    MoveRange MoveLimts;
    PositionDataRange InputRange;
    uint16_t OffsetX;
    uint16_t OffsetY;
    float ShiftUnitX;
    float ShiftUnitY;
}ImageMovingItem;



void ImageMovingItem_Init(ImageMovingItem * self, const uint8_t (*newItemMask)[COLUMNS], MoveRange newMoveLimits, PositionDataRange newInputRange, COLOR newColor);
void ImageMovingItem_SetMoveLimits(ImageMovingItem * self, MoveRange newMoveLimits);
void ImageMovingItem_SetPositionDataRange(ImageMovingItem * self, PositionDataRange newInputRange);

void ImageMovingItem_SetPosition(ImageMovingItem * self, float inputX, float inputY);
void ImageMovingItem_SetNewItemData(ImageMovingItem * self, const uint8_t (*newItemMask)[COLUMNS]);

#endif /* _IMAGE_MOVING_ITEM_H_ */