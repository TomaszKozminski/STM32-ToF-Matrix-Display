#include "TimeMeasure.h"

int CyclesMeasueTest(){
    MEASURE_CYCLES_START()

    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop"); 

    MEASURE_CYCLES_STOP()
    return cycles;
}