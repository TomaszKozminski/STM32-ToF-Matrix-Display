#ifndef _LED_MATRIX_FONT_H
#define _LED_MATRIX_FONT_H

#include <inttypes.h>

typedef struct{
    uint8_t  shape[5];
}charShape;


typedef enum{
    BASIC
}FONT_TYPE;


/*

*/
uint8_t Font_GetCharShape(FONT_TYPE fontType, char character, charShape * retShape);

void Font_GetShapeHeight(FONT_TYPE fontType, uint8_t * retHeight);
void Font_GetShapeWidth(FONT_TYPE fontType, uint8_t * retWidth);

#endif /* _LED_MATRIX_FONT_H */