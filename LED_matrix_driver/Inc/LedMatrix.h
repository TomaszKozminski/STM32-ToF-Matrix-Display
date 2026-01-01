#ifndef _LED_MATRIX_H
#define _LED_MATRIX_H

// #include "main.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "Frame.h"

typedef struct{
    SemaphoreHandle_t Mutex;
    Frame FrameData;
    Frame Buffer;
}LedMatrix;

int LedMatrix_Init();

// @biref displays data saved in Frame utilizing DMA
// for data transfer
void LedMatrix_DisplayFrame();


void LedMatrix_ChangeFrame(Frame newFrame);



#endif /* _LED_MATRIX_H */