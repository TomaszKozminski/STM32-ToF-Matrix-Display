#ifndef _INC_FONT_H_
#define _INC_FONT_H_

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

#endif /* _INC_FONT_H_ */