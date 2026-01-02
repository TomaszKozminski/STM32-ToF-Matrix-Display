#ifndef _LED_MATRIX_FONT_H
#define _LED_MATRIX_FONT_H

#include <inttypes.h>

// struktura na dane kształtu czcionki
typedef struct{
    uint8_t  shape[5];
}charShape;

// dostępne generatory czcionki
typedef enum{
    BASIC
}FONT_TYPE;


/**
 * @brief zwraca kształt znaku z odpowiedniego generatora
 * @param fontType generator czcionki
 * @param character znak do wygenerowania
 * @param retShape wskaźnik na zwrócenie wygenerowanego kształtu
 */
void Font_GetCharShape(FONT_TYPE fontType, char character, charShape * retShape);

/**
 * @brief zwraca wysokość znaku danego generatora
 * @param fontType generator czcionki
 * @param retHeight wskaźnik na zwrot wysokości danego generatora
 */
void Font_GetShapeHeight(FONT_TYPE fontType, uint8_t * retHeight);

/**
 * @brief zwraca szerokość znaku danego generatora
 * @param fontType generator czcionki
 * @param retWidth wskaźnik na zwrot szerokości danego generatora
 */
void Font_GetShapeWidth(FONT_TYPE fontType, uint8_t * retWidth);

#endif /* _LED_MATRIX_FONT_H */