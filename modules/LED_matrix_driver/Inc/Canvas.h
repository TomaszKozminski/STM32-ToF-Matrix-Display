#ifndef _LED_MATRIX_CANVAS_H
#define _LED_MATRIX_CANVAS_H

#include "Frame.h"
#include "Font.h"
#include "cmsis_os.h"
#include "Colors.h"
#include "Image.h"

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

/**
 * @brief inicjalizuje obiekt
 * @param self wskaźnik na obiekt
 * @note po udanym wywołaniu niezbędne jest wywołanie Canvas_Destroy w celu uniknięcia wycieku pamięci
 */
int  Canvas_Create(Canvas * self);

/**
 * @brief resetuje zawartość "płótna"
 * @param self wskaźnik na obiekt
 */
void Canvas_Reset(Canvas * self);

/**
 * @brief usuwa obiekt
 * @param self wskaźnik na obiekt
 */
void Canvas_Destroy(Canvas * self);

/**
 * @brief kładzie obraz na płótno
 * @param self wskaźnik na obiekt
 * @param newImage wskaźnik na obraz
 */
int Canvas_PutImage(Canvas * self, Image * newImage);

/**
 * @brief kładzie tekst na płótno
 * @param self wskaźnik na obiekt
 * @param font czcionka tekstu
 * @param line linia w której ma powstać tekst
 * @param text zawartość do wyświetlenia
 * @param pixel_shift wartość przesunięcia względem lewej krawędzi matrycy
 * @param color kolor tekstu
 */
int Canvas_PutTextLine(Canvas * self, FONT_TYPE font, TEXT_LINE line, const char * text, int16_t pixel_shift, COLOR color);

/**
 * @brief generuje klatkę gotową do wyświetlnia przez matryce z zawartości płótna
 * @param self wskażnik na obiekt
 * @returns wskaźnik na wygenerowaną klatkę
 * @note wygenerowana jest alokowana dynamicznie
 */
Frame Canvas_GenerateFrame(Canvas * self);

#endif /* _LED_MATRIX_CANVAS_H */