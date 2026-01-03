#include "Canvas.h"
#include "Frame.h"
#include "cmsis_os.h"
#include "main.h"
#include <string.h>
#include "LedMatrixConst.h"

// użyte wzorce:
// - object pattern
// - mutex multithread safety

/***********************************************************************************
 *                              LOCAL CONSTANTS
 * *********************************************************************************/

static const uint16_t ROW_SELECT_PINS[4] = {A_Pin, B_Pin, C_Pin, D_Pin};

/***********************************************************************************
 *                              PRIVATE METHODS
 * *********************************************************************************/

 /**
 * @brief zwraca bitmaske rejestru BSRR dla podanego rzędu
 * @param row rząd na matrycy
 */
static uint16_t Canvas_SelectRow(uint8_t row)
{
    uint16_t row_set_bits = 0;

    for(int i = 0; i < 4; ++i) {
        if (row & (1 << i)) 
            row_set_bits |= ROW_SELECT_PINS[i];
    }
    return row_set_bits;
}

/**
 * @brief wprowadza tekst z przesnięciem w prawo
 * @param self wskaźnik na obiekt
 * @param font czcionka tekstu
 * @param line linia w której ma powstać tekst
 * @param text zawartość do wyświetlenia
 * @param pixel_shift wartość przesunięcia względem lewej krawędzi matrycy
 * @param color kolor tekstu
 */
static int Canvas_TextPositiveShift(Canvas * self, FONT_TYPE font, TEXT_LINE line, const char * text, int16_t pixel_shift, COLOR color)
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
        currentRow = line + x;
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

/**
 * @brief wprowadza tekst z przesnięciem w lewo
 * @param self wskaźnik na obiekt
 * @param font czcionka tekstu
 * @param line linia w której ma powstać tekst
 * @param text zawartość do wyświetlenia
 * @param pixel_shift wartość przesunięcia względem lewej krawędzi matrycy
 * @param color kolor tekstu
 */
static int Canvas_TextNegativeShift(Canvas * self, FONT_TYPE font, TEXT_LINE line, const char * text, int16_t pixel_shift, COLOR color)
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
        currentRow = line + x;
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

/**
 * @brief kładzie tło obrazu
 * @param self wskaźnik na obiekt
 * @param newImage wzkaźnik na obraz
 */
static void Canvas_PutBackground(Canvas * self, Image * newImage)
{

    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    for(int row = 0; row < ROWS; row++)
    {
        for(int column = 0; column < COLUMNS; column++)
        {
            if(newImage->Background->BackgroundMask[row][column]){
                self->RGB_data[row][column] = newImage->Background->Color;
            }else{
                self->RGB_data[row][column] = BLACK;
            }
            
        }
    }

    xSemaphoreGive(self->Mutex);
}

/**
 * @brief kładzie dynamiczny element obrazu
 * @param self wskaźnik na obiekt
 * @param newImage wzkaźnik na obraz
 */
static void Canvas_PutMovingItem(Canvas * self, Image * newImage)
{

    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    for(int row = newImage->MovingItem->OffsetY; row < ROWS; row++)
    {
        for(int column = newImage->MovingItem->OffsetX; column < COLUMNS; column++)
        {
            if(newImage->MovingItem->ItemMask[row - newImage->MovingItem->OffsetY][column - newImage->MovingItem->OffsetX])
                self->RGB_data[row][column] = newImage->MovingItem->Color;
        }
    }

    xSemaphoreGive(self->Mutex);
}
/***********************************************************************************
 *                              PUBLIC METHODS
 * *********************************************************************************/

int Canvas_Create(Canvas * self)
{
    if(self == NULL)
        return 1;

    /* ensure predictable initial state in case caller passed uninitialized memory */
    self->RGB_data = NULL;

    self->Mutex = xSemaphoreCreateMutex();
    if(self->Mutex == NULL){
        return 1;
    }

    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    if(self->RGB_data == NULL){
        self->RGB_data = (uint16_t**)malloc(ROWS * sizeof(uint16_t*));
        if(self->RGB_data == NULL){
            xSemaphoreGive(self->Mutex);
            return 1;
        }

        for (int x = 0; x < ROWS; x++){
            self->RGB_data[x] = (uint16_t*)malloc(COLUMNS * sizeof(uint16_t));
            if(self->RGB_data[x] == NULL){
                /* free previously allocated rows */
                for(int y = 0; y < x; y++) free(self->RGB_data[y]);
                free(self->RGB_data);
                self->RGB_data = NULL;
                xSemaphoreGive(self->Mutex);
                return 1;
            }
        }
    }


    for(int x = 0; x < ROWS; x++){
        memset(self->RGB_data[x], 0, sizeof(uint16_t) * COLUMNS);
    }


    xSemaphoreGive(self->Mutex);

    return 0;

}

void Canvas_Reset(Canvas * self)
{
    if(self == NULL || self->RGB_data == NULL)
        return;

    xSemaphoreTake(self->Mutex, portMAX_DELAY);

    for(int x = 0; x < ROWS; x++)
    {
        memset(self->RGB_data[x], 0, sizeof(uint16_t) * COLUMNS);
    }

    xSemaphoreGive(self->Mutex);
}

void Canvas_Destroy(Canvas * self)
{
    if(self == NULL)
        return;

    if(self->Mutex != NULL)
        xSemaphoreTake(self->Mutex, portMAX_DELAY);

    if(self->RGB_data != NULL)
    {
        for (int x = 0; x < ROWS; x++)
            free(self->RGB_data[x]);
        free(self->RGB_data);
        self->RGB_data = NULL;
    }

    if(self->Mutex != NULL){
        xSemaphoreGive(self->Mutex);
        vSemaphoreDelete(self->Mutex);
        self->Mutex = NULL;
    }
}

int Canvas_PutTextLine(Canvas * self, FONT_TYPE font, TEXT_LINE line, const char * text, int16_t pixel_shift, COLOR color)
{
    int ret;
    if(pixel_shift >= 0){
        ret = Canvas_TextPositiveShift(self, font, line, text, pixel_shift, color);
    }else{
        ret = Canvas_TextNegativeShift(self, font, line, text, pixel_shift, color);
    }

    return ret;
}

int Canvas_PutImage(Canvas * self, Image * newImage)
{
    int ret = 0;
    if(self != NULL && newImage != NULL){
        
        if(newImage->Background != NULL)
            Canvas_PutBackground(self, newImage);

        if(newImage->MovingItem != NULL)
            Canvas_PutMovingItem(self, newImage);
    }

    return ret;
}

Frame Canvas_GenerateFrame(Canvas * self)
{
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

    Frame newFrame = Frame_Create();
    if(newFrame == NULL)
        return NULL;

    for(uint8_t row = 0; row < ADDRESSABLE_ROWS; row++)
    {
        pixel_tracker_1 = (row * COLUMNS * BCM_BIT_DEPTH);
        
        row_bitmask = Canvas_SelectRow(row);
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
                newFrame[pixel_tracker_1 + pixel_tracker_2 + column] = (reset << 16) | set;
                // save current register state 
                OLD_REG_STATE = BSRR_set_bits;
            }
        }   
    }

    xSemaphoreGive(self->Mutex); 

    return newFrame;
}
