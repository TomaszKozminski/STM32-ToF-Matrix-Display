#include "LedMatrix.h"
#include "Frame.h"
#include "LedMatrixConst.h"
#include "main.h"
#include "portmacro.h"
#include <string.h>

// użyte wzorce:
// - object pattern
// - singleton pattern
// - opaque pattern
// - mutex multithread safety


// obiekt matrycy
typedef struct{
    SemaphoreHandle_t Mutex;
    // buffer na wyświetlane dane
    Frame DisplayData;
    // drugi buffer przyjmujący nowe dane
    Frame Buffer;
}LedMatrix;

// instancja obiektu
static LedMatrix _singleton = {NULL};


/***********************************************************************************
 *                              EXTERN VARIABLES
 * *********************************************************************************/
extern DMA_HandleTypeDef hdma_tim8_ch1;
extern TaskHandle_t xTaskToNotify ;

/***********************************************************************************
 *                              LOCAL CONSTANTS
 * *********************************************************************************/
static const uint8_t BCM_delay[4] = {1,2,4,8};


/***********************************************************************************
 *                              PRIVATE METHODS
 * *********************************************************************************/

/**
 * @brief wyłącza wyświetlane diody
 */
static inline void LED_stop_display(){
    GPIOC->BSRR = OE_Pin;
}

/**
 * @brief włącza wyświetlanie diód
 */
static inline void LED_start_display(){
    GPIOC->BSRR = (OE_Pin << 16);
}

/**
 * @brief wprowadza opóźnienie przed wyłączeniem diody w celu zasymulowania koloru
 * @param bit_delay wartość opóźnienia
 */
static inline void BCM_weight(uint8_t bit_delay){
    delay_ticks(bit_delay);
}

/**
 * @brief podmienia zawartość wyświtlanych danych na zawartość bufforu
 */
static void LedMatrix_LoadBuffer()
{
    xSemaphoreTake(_singleton.Mutex, portMAX_DELAY);

    if(_singleton.Buffer != NULL){
        memcpy(_singleton.DisplayData, _singleton.Buffer, sizeof(uint32_t) * ADDRESABLE_PIXELS * BCM_BIT_DEPTH);
        Frame_Delete(_singleton.Buffer);
        _singleton.Buffer = NULL;
    }

    xSemaphoreGive(_singleton.Mutex);
}

/***********************************************************************************
 *                              PUBLIC METHODS
 * *********************************************************************************/

int LedMatrix_Init()
{
    if(_singleton.Mutex == NULL && _singleton.DisplayData == NULL){

        _singleton.Mutex = xSemaphoreCreateMutex();
        if(_singleton.Mutex == NULL){
            return 1;
        }

        _singleton.DisplayData = Frame_Create();
        if(_singleton.DisplayData == NULL){
            return 1;
        }
            
        _singleton.Buffer = NULL;
    }
    

    return 0;
}

int LedMatrix_DeInit()
{
    if(_singleton.DisplayData != NULL){
        Frame_Delete(_singleton.DisplayData);
    }
    
    if(_singleton.Buffer != NULL){
        Frame_Delete(_singleton.Buffer);
    }

    if(_singleton.Mutex != NULL){
        vSemaphoreDelete(_singleton.Mutex);
    }
    
    return 0;
}

void LedMatrix_DisplayFrame()
{
    LedMatrix_LoadBuffer();

    uint16_t used_pixels_1 = 0;
    uint16_t used_pixels_2 = 0;

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
                                (uint32_t)&(_singleton.DisplayData[ used_pixels_1 + used_pixels_2 ]),
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