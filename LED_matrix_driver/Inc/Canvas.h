#ifndef _LED_MATRIX_CANVAS_H
#define _LED_MATRIX_CANVAS_H

#include "LedMatrixConst.h"
#include "Frame.h"
#include "font.h"
#include "cmsis_os.h"
#include "colors.h"

typedef enum{
    RIGHT,
    LEFT
}SLIDE_DIRECTION;

typedef enum{
    TEXT_LINE_1 = 1,
    TEXT_LINE_2 = 7,
    TEXT_LINE_3 = 13,
    TEXT_LINE_4 = 19,
    TEXT_LINE_5 = 25
}TEXT_LINE;


typedef struct{
    
    SemaphoreHandle_t Mutex;
    uint16_t ** RGB_data;
}Canvas;

int Canvas_Create(Canvas * self);
void Canvas_Reset(Canvas * self);
void Canvas_Destroy(Canvas * self);

int Canvas_TextLine(Canvas * self, FONT_TYPE font, TEXT_LINE line, const char * text, int16_t pixel_shift, COLOR color);

Frame Canvas_GenerateFrame(Canvas * self);

#endif /* _LED_MATRIX_CANVAS_H */