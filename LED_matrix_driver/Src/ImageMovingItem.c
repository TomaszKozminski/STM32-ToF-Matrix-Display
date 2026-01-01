#include "ImageMovingItem.h"
#include "LedMatrixConst.h"

void ImageMovingItem_Init(ImageMovingItem * self, const uint8_t (*newItemMask)[COLUMNS], MoveRange newMoveLimits, PositionDataRange newInputRange, COLOR newColor)
{
    if(self != NULL && newItemMask != NULL){
        self->ItemMask = newItemMask;
        self->Color = newColor;
        self->MoveLimts = newMoveLimits;
        self->InputRange = newInputRange;
        // defaulty position in the middle
        self->OffsetX = (self->MoveLimts.maxOffsetX)/2;
        self->OffsetY = (self->MoveLimts.maxOffsetY)/2;

        self->ShiftUnitX = (self->InputRange.maxX - self->InputRange.minX) / (float)(self->MoveLimts.maxOffsetX);
        self->ShiftUnitY = (self->InputRange.maxY - self->InputRange.minY) / (float)(self->MoveLimts.maxOffsetY);
    }
}

void ImageMovingItem_SetMoveLimits(ImageMovingItem * self, MoveRange newMoveLimits){
    if(self != NULL){
        self->MoveLimts = newMoveLimits;
    }
}

void ImageMovingItem_SetPositionDataRange(ImageMovingItem * self, PositionDataRange newInputRange)
{
    if(self != NULL){
        self->InputRange = newInputRange;
    }
}

void ImageMovingItem_SetPosition(ImageMovingItem * self, float inputX, float inputY)
{
    if(self != NULL){

        self->OffsetX = (uint16_t)((7.0 - inputX)/self->ShiftUnitX);
        self->OffsetY = (uint16_t)(inputY/self->ShiftUnitY);
        if(self->OffsetX > self->MoveLimts.maxOffsetX){
            self->OffsetX = self->MoveLimts.maxOffsetX;
        }
        if(self->OffsetY > self->MoveLimts.maxOffsetY){
            self->OffsetY = self->MoveLimts.maxOffsetY;
        }
    }
}

void ImageMovingItem_SetNewItemData(ImageMovingItem * self, const uint8_t (*newItemMask)[COLUMNS])
{
    if(self != NULL && newItemMask != NULL){
        self->ItemMask = newItemMask;
    }
}