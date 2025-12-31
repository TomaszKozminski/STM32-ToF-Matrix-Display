#include "led_display.h"
#include "task.h"

#include "font.h"

#include <stdint.h>
#include <string.h>
extern DMA_HandleTypeDef hdma_tim8_ch1;
// extern SemaphoreHandle_t dmaSemaphore;

extern TaskHandle_t xTaskToNotify ;

static const uint16_t ROW_SELECT_PINS[4] = {A_Pin, B_Pin, C_Pin, D_Pin};
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
    delay_ticks(bit_delay);
}
// @brief returns pins bitmask in BSRR to set passed row
static uint16_t LED_row_set_reg(uint8_t row){
    uint16_t row_set_bits = 0;

    for(int i = 0; i < 4; ++i) {
        if (row & (1 << i)) 
            row_set_bits |= ROW_SELECT_PINS[i];
    }
    return row_set_bits;
}



void LED_slide(const uint32_t * register_data)
{
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
            
            HAL_DMA_Start_IT( &hdma_tim8_ch1,  
                                (uint32_t)&(register_data[ used_pixels_1 + used_pixels_2 ]),
                                (uint32_t)&(GPIOC->BSRR),
                                64);

            //task notify wait
            xTaskToNotify = xTaskGetCurrentTaskHandle();

            //timer update event
            TIM8->EGR = TIM_EGR_UG;
            //timer start in OPM mode + start counter,
            //starts DMA transfer
            TIM8->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;

            //task notify wait
            ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

            //semaphore wait
            // xSemaphoreTake(dmaSemaphore, portMAX_DELAY);

            LED_start_display();
            BCM_weight(BCM_delay[BCM_bit]);
            LED_stop_display();
            
        }   
    }
}

int LED_Init(LED_frame * self){
    
    self->Mutex = xSemaphoreCreateMutex();
    if(self->Mutex == NULL){
        return 1;
    }

    LED_ResetCanvas(self);
    LED_PrepareFrame(self);

    return 0;
}

void LED_PrepareFrame(LED_frame * self){

    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    uint16_t pixel_tracker_1 = 0;
    uint16_t pixel_tracker_2 = 0;
    uint16_t upper_pixel = 0;
    uint16_t lower_pixel = 0;
    
    uint16_t bitmask_b = 0;
    uint16_t bitmask_g = 0;
    uint16_t bitmask_r = 0;

    uint16_t BSRR_set_bits = 0;
    uint16_t row_bitmask = 0;

    uint16_t OLD_REG_STATE = 0;
    uint16_t set = 0;
    uint16_t reset = 0;
    uint16_t REG_changes = 0;
    uint32_t * p_new_reg_data = malloc(sizeof(uint32_t) * PIXELS * BCM_BIT_DEPTH);

    for(uint8_t row = 0; row < ADDRESSABLE_ROWS; row++)
    {
        pixel_tracker_1 = (row * COLUMNS * BCM_BIT_DEPTH);
        
        row_bitmask = LED_row_set_reg(row);
        for(uint8_t BCM_bit = 0; BCM_bit < BCM_BIT_DEPTH; BCM_bit++)
        {
            pixel_tracker_2 = (BCM_bit * COLUMNS);
            bitmask_b = 1 << BCM_bit;
            bitmask_g = bitmask_b << 4;
            bitmask_r = bitmask_b << 8;

            for (uint8_t column = 0; column < COLUMNS; column++) 
            {
                upper_pixel = self->RGB_data[row][column];
                lower_pixel = self->RGB_data[row + ADDRESSABLE_ROWS][column];

                BSRR_set_bits = 0;
                //set color vals
                if(upper_pixel & bitmask_r) BSRR_set_bits |= R1_Pin;
                if(upper_pixel & bitmask_g) BSRR_set_bits |= G1_Pin;
                if(upper_pixel & bitmask_b) BSRR_set_bits |= B1_Pin;
                if(lower_pixel & bitmask_r) BSRR_set_bits |= R2_Pin;
                if(lower_pixel & bitmask_g) BSRR_set_bits |= G2_Pin;
                if(lower_pixel & bitmask_b) BSRR_set_bits |= B2_Pin;
                //set row vals
                BSRR_set_bits |= row_bitmask;

                // BSRR_set_bits is new register state we want to achieve
                // get all pins that change value (XOR)
                REG_changes = OLD_REG_STATE^BSRR_set_bits;
                // save all pins that change 0 -> 1
                set = REG_changes & BSRR_set_bits;
                // save all pins that change 1 -> 0
                reset = REG_changes & OLD_REG_STATE;
                // value written to register to achieve BSRR_set_bits
                p_new_reg_data[pixel_tracker_1 + pixel_tracker_2 + column] = (reset << 16) | set;
                // save current register state 
                OLD_REG_STATE = BSRR_set_bits;
            }
        }   
    }
    if(self->register_data != NULL)
        free(self->register_data);
    self->register_data = p_new_reg_data;

    xSemaphoreGive(self->Mutex);
}

void LED_DisplayFrame(LED_frame * self){
    xSemaphoreTake(self->Mutex, portMAX_DELAY);

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
            
            HAL_DMA_Start_IT( &hdma_tim8_ch1,  
                                (uint32_t)&(self->register_data[ used_pixels_1 + used_pixels_2 ]),
                                (uint32_t)&(GPIOC->BSRR),
                                64);

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

    xSemaphoreGive(self->Mutex);
}


static int LED_TextPositiveShift(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, int16_t pixel_shift, COLOR color)
{
    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    uint8_t shapeCnt;
    uint8_t shapeMaxCnt;    
    uint8_t shapeW, shapeH;
    uint16_t shapesAfterShift;

    Font_GetShapeHeight(font, &shapeH);
    // get single shape's pixel width and calculate max possibly needed shapes' data and allocate
    Font_GetShapeWidth(font, &shapeW);

    shapeMaxCnt = COLUMNS/shapeW + 1;
    shapesAfterShift = (strlen(text) - (pixel_shift/shapeW));
    shapeCnt = shapesAfterShift > shapeMaxCnt ? shapeMaxCnt : shapesAfterShift;

    if(pixel_shift/shapeW >= strlen(text)){
        xSemaphoreGive(self->Mutex);
        return 1;
    }

    charShape * shapeData = malloc(sizeof(charShape) * shapeCnt);

    // load shape data of signs after pixel shift
    for(int x = 0; x < shapeCnt; x++)
    {
        Font_GetCharShape(font, text[x + (pixel_shift/shapeW)], shapeData + x);
    }

    uint8_t rowPixelsUsed = 0;
    uint8_t currentRow;
    charShape * currentShape;
    uint8_t skipPixels = pixel_shift % shapeW;
    // x - rząd danych shape'u
    for(int x = 0; x < shapeH; x++)
    {   
        skipPixels = pixel_shift % shapeW;
        currentRow = text_lane + x;
        rowPixelsUsed = 0;
        //y - shape
        for(int y = 0; y < shapeCnt; y++)
        {
            currentShape = (shapeData + y);

            for(int z = 0; z < shapeW; z++)
            {   
                if((y == 0) && (skipPixels)){
                    skipPixels -= 1;
                    continue;
                }
                    
                if(rowPixelsUsed < 64)
                {// + (pixel_shift%shapeW)

                    if(currentShape->shape[x] & (1 << ((shapeW-1) - (z)))){
                        self->RGB_data[currentRow][rowPixelsUsed] = color;
                    }else{
                        self->RGB_data[currentRow][rowPixelsUsed] = BLACK;
                    }
                    rowPixelsUsed++;
                }
            }
        }
    }


    free(shapeData);
    xSemaphoreGive(self->Mutex);
    return 0;
}

static int LED_TextNegativeShift(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, int16_t pixel_shift, COLOR color)
{
    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    uint8_t shapeCnt;
    uint8_t shapeMaxCnt;    
    uint8_t shapeW, shapeH;
    uint16_t shapesAfterShift;

    Font_GetShapeHeight(font, &shapeH);
    // get single shape's pixel width and calculate max possibly needed shapes' data and allocate
    Font_GetShapeWidth(font, &shapeW);

    shapeMaxCnt = COLUMNS/shapeW;
    if(COLUMNS % shapeW)
        shapeMaxCnt+=1;

    shapesAfterShift = (shapeMaxCnt + (pixel_shift/shapeW));
    shapeCnt = shapesAfterShift > strlen(text) ? strlen(text) : shapesAfterShift;

    if(shapeCnt <= 0){
        xSemaphoreGive(self->Mutex);
        return 2;
    }

    charShape * shapeData = malloc(sizeof(charShape) * shapeCnt);

    // load shape data of signs after pixel shift
    for(int x = 0; x < shapeCnt; x++)
    {
        Font_GetCharShape(font, text[x], shapeData + x);
    }

    uint8_t rowPixelsUsed = 0;
    uint8_t currentRow;
    charShape * currentShape;

    // x - rząd danych shape'u
    for(int x = 0; x < shapeH; x++)
    {   
        currentRow = text_lane + x;
        rowPixelsUsed = 0;
        //y - shape
        for(int y = 0; y < shapeCnt; y++)
        {
            currentShape = (shapeData + y);

            for(int z = 0; z < shapeW; z++)
            {                           
                if(rowPixelsUsed < 64)
                {
                    if(currentShape->shape[x] & (1 << ((shapeW-1) - (z)))){
                        self->RGB_data[currentRow][rowPixelsUsed - pixel_shift] = color;
                    }else{
                        self->RGB_data[currentRow][rowPixelsUsed - pixel_shift] = BLACK;
                    }
                    rowPixelsUsed++;
                }
            }
        }
    }


    free(shapeData);
    xSemaphoreGive(self->Mutex);
    return 0;
}
// 0 - displayed, 1 - displayed & end reached, 2 - displayed & beginning reched, 3+ error
int LED_TextCreateLane(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, int16_t pixel_shift, COLOR color)
{
    int ret;
    if(pixel_shift >= 0){
        ret = LED_TextPositiveShift(self, font, text_lane, text, pixel_shift, color);
    }else{
        ret = LED_TextNegativeShift(self, font, text_lane, text, pixel_shift, color);
    }

    return ret;
}

void LED_ResetCanvas(LED_frame * self)
{
    if(self == NULL)
        return;

    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    if(self->RGB_data == NULL){
        self->RGB_data = (uint16_t**)malloc(ROWS * sizeof(uint16_t*));
        for (int x = 0; x < ROWS; x++)
            self->RGB_data[x] = (uint16_t*)malloc(COLUMNS * sizeof(uint16_t));
    }
    
    for(int x = 0; x < ROWS; x++)
    {
        memset(self->RGB_data[x], 0, sizeof(uint16_t) * COLUMNS);
    }

    xSemaphoreGive(self->Mutex);
}

void LED_DestroyCanvas(LED_frame * self){
    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    if((self != NULL) && (self->RGB_data != NULL))
    {
        for (int x = 0; x < ROWS; x++)
            free(self->RGB_data[x]);
        free(self->RGB_data);
    }

    xSemaphoreGive(self->Mutex);
}

void LED_TextBounce(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, COLOR color, uint8_t speed)
{
    if(self == NULL || text == NULL){
        return;
    }
    int ret = 2;
    int shift = 0;
    for(;;)
    {
        if(ret == 1){
            while(ret != 2){
                LED_ClearLane(self, text_lane);
                ret = LED_TextCreateLane(self, font, text_lane, text, shift--, color);
                LED_PrepareFrame(self);
                for(int x = 0; x < speed; x++)
                    LED_DisplayFrame(self);    
            }
        }else if(ret == 2){
            while(ret != 1){
                LED_ClearLane(self, text_lane);
                ret = LED_TextCreateLane(self, font, text_lane, text, shift++, color);
                LED_PrepareFrame(self);
                for(int x = 0; x < speed; x++)
                    LED_DisplayFrame(self);    
            }
        }
    }
}

void LED_TextSlide(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, COLOR color, uint8_t speed, SLIDE_DIRECTION direction, bool loop)
{
    int ret = 0;
    int16_t shift;
    uint8_t shapeW, shapeH;
    Font_GetShapeHeight(font, &shapeH);
    Font_GetShapeWidth(font, &shapeW);
    int16_t maxShiftPos = strlen(text) * shapeW;
    int16_t maxShiftNeg = 1 - COLUMNS ;

    switch(direction){
        case RIGHT:
            do{
                ret = 0;
                shift = maxShiftPos;
                while(!ret){
                    LED_ClearLane(self, text_lane);
                    ret = LED_TextCreateLane(self, font, text_lane, text, shift--, color);
                    LED_PrepareFrame(self);
                    for(int x = 0; x < speed; x++)
                        LED_DisplayFrame(self);  
                }
            }while(loop);
            break;
        case LEFT:
            do{
                ret = 0;
                shift = maxShiftNeg;
                while(!ret){
                    LED_ClearLane(self, text_lane);
                    ret = LED_TextCreateLane(self, font, text_lane, text, shift++, color);
                    LED_PrepareFrame(self);
                    for(int x = 0; x < speed; x++)
                        LED_DisplayFrame(self);  
                }
            }while(loop);
            break;
        default:
            Error_Handler();
    }
}

void LED_TextStatic(LED_frame * self, FONT_TYPE font, TEXT_LANE text_lane, const char * text, COLOR color)
{
    LED_ResetCanvas(self);
    LED_TextCreateLane(self, font, text_lane, text, 0, color);
    LED_PrepareFrame(self);
    for(;;)
        LED_DisplayFrame(self);  
}

void LED_ClearLane(LED_frame * self, TEXT_LANE text_lane){
    if(self == NULL || self->RGB_data == NULL)
        return;

    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    for(int x = text_lane; x < text_lane+6; x++)
    {
        memset(self->RGB_data[x], 0, sizeof(uint16_t) * COLUMNS);
    }

    xSemaphoreGive(self->Mutex);
}

