#ifndef _LED_MATRIX_FRAME_H
#define _LED_MATRIX_FRAME_H


#include <inttypes.h>
#include "LedMatrixConst.h"

typedef uint32_t * Frame;

Frame Frame_Create(void);
void Frame_Delete(Frame frame);


#endif /* _LED_MATRIX_FRAME_H */