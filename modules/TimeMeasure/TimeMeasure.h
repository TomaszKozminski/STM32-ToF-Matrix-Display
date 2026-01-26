#ifndef TIME_MEASUE_H_
#define TIME_MEASUE_H_

#include "stm32l4xx_hal.h"

#define MEASURE_CYCLES_START() \
int cycles; \
int v1, v2 ; \
SysTick->LOAD = 0xFFFFFF; \
SysTick->VAL = 0; \
SysTick->CTRL = 0x05; \
v1 = SysTick->VAL; 

#define MEASURE_CYCLES_STOP() \
v2 = SysTick->VAL; \
cycles = v2 - v1 - 2; 


#define OSC_SIGNAL_HIGH 
#define OSC_SIGNAL_LOW
// void inline f(){
//     int cycles; 
//     int v1, v2 ; 
//     SysTick->LOAD = 0xFFFFFF;
//     SysTick->VAL = 0;
//     SysTick->CTRL = 0x05;
//     v1 = SysTick->VAL;

//     v2 = SysTick->VAL;
//     cycles = v2 - v1 - 2;
// }

#endif /* TIME_MEASUE_H_ */