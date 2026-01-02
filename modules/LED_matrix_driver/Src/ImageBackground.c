#include "ImageBackground.h"
#include "LedMatrixConst.h"


// użyte wzorce:
// - object pattern

void ImageBackground_Init(ImageBackground * self, const uint8_t (*newBackgroundMask)[COLUMNS] , COLOR newColor)
{
    if(self != NULL && newBackgroundMask != NULL){
        self->BackgroundMask = newBackgroundMask;
        self->Color = newColor;
    }
}

void ImageBackground_SetColor(ImageBackground * self, COLOR newColor)
{
    if(self != NULL){
        self->Color = newColor;
    }
}

void ImageBackground_SetNewBackgroundData(ImageBackground * self, const uint8_t (*newBackgroundMask)[COLUMNS])
{
    if(self != NULL && newBackgroundMask != NULL){
        self->BackgroundMask = newBackgroundMask;
    }
}