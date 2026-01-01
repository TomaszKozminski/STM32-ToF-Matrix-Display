#include "LedMatrix.h"
#include "Frame.h"
#include "LedMatrixConst.h"
#include "main.h"
#include "portmacro.h"
#include <string.h>

static LedMatrix _singleton;

extern DMA_HandleTypeDef hdma_tim8_ch1;
extern TaskHandle_t xTaskToNotify ;

static const uint8_t BCM_delay[4] = {1,2,4,8};

// @brief disables LED output, allows setting new data
static inline void LED_stop_display(){
    GPIOC->BSRR = OE_Pin;
}
// @brief enables display of set data
static inline void LED_start_display(){
    GPIOC->BSRR = (OE_Pin << 16);
}
// @brief delays adequate amount to implement Binary Code Modulation coloring
static inline void BCM_weight(uint8_t bit_delay){
    // vTaskDelay(bit_delay);
    // portENTER_CRITICAL();
    // taskENTER_CRITICAL();
    delay_ticks(bit_delay);
    // portEXIT_CRITICAL();
}



int LedMatrix_Init()
{
    _singleton.Mutex = xSemaphoreCreateMutex();
    if(_singleton.Mutex == NULL){
        return 1;
    }

    _singleton.FrameData = Frame_Create();
    _singleton.Buffer = NULL;

    return 0;
}

void LedMatrix_DisplayFrame()
{
    xSemaphoreTake(_singleton.Mutex, portMAX_DELAY);

    uint16_t used_pixels_1 = 0;
    uint16_t used_pixels_2 = 0;
    if(_singleton.Buffer != NULL){
        memcpy(_singleton.FrameData, _singleton.Buffer, sizeof(uint32_t) * PIXELS * BCM_BIT_DEPTH);
        Frame_Delete(_singleton.Buffer);
        _singleton.Buffer = NULL;
    }

    xSemaphoreGive(_singleton.Mutex);


    // reset register row and colors value
    GPIOC->BSRR = ((A_Pin|B_Pin|C_Pin|D_Pin|COLOR_PINS) << 16)|OE_Pin;

    for(uint8_t row = 0; row < ADDRESSABLE_ROWS; row++)
    {
        used_pixels_1 = row * COLUMNS * BCM_BIT_DEPTH;
        for(uint8_t BCM_bit = 0; BCM_bit < BCM_BIT_DEPTH; BCM_bit++)
        {
            LED_stop_display();

            used_pixels_2 = BCM_bit * COLUMNS;
            
            HAL_DMA_Start_IT(   &hdma_tim8_ch1,  
                                (uint32_t)&(_singleton.FrameData[ used_pixels_1 + used_pixels_2 ]),
                                (uint32_t)&(GPIOC->BSRR),
                                COLUMNS
                            );

            //wait for task notify
            xTaskToNotify = xTaskGetCurrentTaskHandle();
            
            //timer update event
            TIM8->EGR = TIM_EGR_UG;
            //timer start in OPM mode + start counter,
            //starts DMA transfer
            TIM8->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;

            //task notify wait
            ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
            
            LED_start_display();
            BCM_weight(BCM_delay[BCM_bit]);
            LED_stop_display();
        }   
    }

    
}


void LedMatrix_ChangeFrame(Frame newFrame)
{
    if(newFrame == NULL)
        return;

    xSemaphoreTake(_singleton.Mutex, portMAX_DELAY);
    
    if(_singleton.Buffer != NULL)
        Frame_Delete(_singleton.Buffer);

    _singleton.Buffer = newFrame;

    xSemaphoreGive(_singleton.Mutex);
}