#ifndef _LED_MATRIX_H
#define _LED_MATRIX_H

// #include "main.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "Frame.h"

/**
 * @brief inicjalizuje instancje obiektu, gdy taki jeszcze nie istnieje
 */
int LedMatrix_Init();

/**
 * @brief niszczy obiekt
 */
int LedMatrix_DeInit();

/**
 * @brief wyświetla dane załadowanej klatki
 * @note przed wyświetleniem sprawdza zawartość bufforu, jeżeli taka istnieje to jest ładowana do wyświtlenia
 */
void LedMatrix_DisplayFrame();

/**
 * @brief podmienia zawartość bufforu na nową klatkę
 */
void LedMatrix_ChangeFrame(Frame newFrame);



#endif /* _LED_MATRIX_H */