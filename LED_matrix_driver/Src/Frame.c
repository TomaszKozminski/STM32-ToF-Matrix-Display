#include "Frame.h"


Frame Frame_Create(void)
{
    Frame newFrame = calloc(sizeof(uint32_t), PIXELS * BCM_BIT_DEPTH);
    return newFrame;
}

void Frame_Delete(Frame frame)
{
    if(frame != NULL)
        free(frame);
}