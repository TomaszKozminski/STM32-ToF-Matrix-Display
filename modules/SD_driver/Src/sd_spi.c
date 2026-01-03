/**
 ******************************************************************************
 * @file    sd_spi.c
 * @brief   SD Card SPI driver implementation
 ******************************************************************************
 */

#include "sd_spi.h"
#include "main.h"
#include <string.h>

/* External SPI handle */
extern SPI_HandleTypeDef hspi2;

/* SD Card CS Pin */
#define SD_CS_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_12

/* Private variables */
static SD_CardInfo_t card_info = {0};

/* Private function prototypes */
static void SD_CS_Low(void);
static void SD_CS_High(void);
static uint8_t SD_SendByte(uint8_t byte);
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg);
static SD_Status_t SD_WaitReady(uint32_t timeout_ms);
static SD_Status_t SD_ReadData(uint8_t *buffer, uint16_t len);

/**
 * @brief Select SD Card (CS Low)
 */
static void SD_CS_Low(void) {
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Small delay for SD card
}

/**
 * @brief Deselect SD Card (CS High)
 */
static void SD_CS_High(void) {
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
}

/**
 * @brief Send/Receive single byte via SPI
 */
static uint8_t SD_SendByte(uint8_t byte) {
    uint8_t rx_byte = 0xFF;
    HAL_SPI_TransmitReceive(&hspi2, &byte, &rx_byte, 1, 100);
    return rx_byte;
}

/**
 * @brief Send command to SD card
 * @param cmd Command index
 * @param arg Command argument
 * @return R1 response
 */
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg) {
    uint8_t response;
    uint8_t retry = 0;
    
    // Send dummy bytes before command
    SD_SendByte(0xFF);
    
    // Send command packet
    SD_SendByte(0x40 | cmd);           // Start bit + command
    SD_SendByte((uint8_t)(arg >> 24)); // Argument[31:24]
    SD_SendByte((uint8_t)(arg >> 16)); // Argument[23:16]
    SD_SendByte((uint8_t)(arg >> 8));  // Argument[15:8]
    SD_SendByte((uint8_t)arg);         // Argument[7:0]
    
    // CRC (only needed for CMD0 and CMD8)
    if (cmd == SD_CMD0) {
        SD_SendByte(0x95); // CRC for CMD0
    } else if (cmd == SD_CMD8) {
        SD_SendByte(0x87); // CRC for CMD8
    } else {
        SD_SendByte(0xFF); // Dummy CRC
    }
    
    // Wait for response (R1)
    do {
        response = SD_SendByte(0xFF);
        retry++;
    } while ((response & 0x80) && (retry < 10));
    
    return response;
}

/**
 * @brief Wait for SD card to be ready
 */
static SD_Status_t SD_WaitReady(uint32_t timeout_ms) {
    uint32_t start = HAL_GetTick();
    
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (SD_SendByte(0xFF) == 0xFF) {
            return SD_OK;
        }
        HAL_Delay(1);
    }
    
    return SD_TIMEOUT;
}

/**
 * @brief Read data block from SD card
 */
static SD_Status_t SD_ReadData(uint8_t *buffer, uint16_t len) {
    uint8_t token;
    uint32_t retry = 0;
    
    // Wait for data token (0xFE)
    do {
        token = SD_SendByte(0xFF);
        retry++;
        if (retry > 1000) {
            return SD_TIMEOUT;
        }
    } while (token != 0xFE);
    
    // Read data
    for (uint16_t i = 0; i < len; i++) {
        buffer[i] = SD_SendByte(0xFF);
    }
    
    // Read CRC (2 bytes, ignored)
    SD_SendByte(0xFF);
    SD_SendByte(0xFF);
    
    return SD_OK;
}

/**
 * @brief Initialize SD Card
 */
SD_Status_t SD_Init(void) {
    uint8_t response;
    uint16_t retry = 0;
    
    // Reset card info
    memset(&card_info, 0, sizeof(card_info));
    
    // CS high during init
    SD_CS_High();
    
    // Send 80 dummy clocks to wake up SD card
    for (uint8_t i = 0; i < 10; i++) {
        SD_SendByte(0xFF);
    }
    
    // Select card
    SD_CS_Low();
    
    // CMD0: Reset card to idle state
    response = SD_SendCommand(SD_CMD0, 0);
    if (response != SD_R1_IDLE) {
        SD_CS_High();
        return SD_ERROR;
    }
    
    // CMD8: Check voltage range (SDC V2)
    response = SD_SendCommand(SD_CMD8, 0x1AA);
    if (response == SD_R1_IDLE) {
        // V2.0 card
        uint8_t ocr[4];
        for (uint8_t i = 0; i < 4; i++) {
            ocr[i] = SD_SendByte(0xFF);
        }
        
        // Check if voltage accepted
        if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
            // Initialize with ACMD41
            retry = 0;
            do {
                SD_SendCommand(SD_CMD55, 0);
                response = SD_SendCommand(SD_ACMD41, 0x40000000);
                retry++;
                HAL_Delay(10);
            } while ((response != SD_R1_READY) && (retry < 100));
            
            if (response == SD_R1_READY) {
                // Read OCR to check if SDHC
                response = SD_SendCommand(SD_CMD58, 0);
                if (response == SD_R1_READY) {
                    uint8_t ocr_read[4];
                    for (uint8_t i = 0; i < 4; i++) {
                        ocr_read[i] = SD_SendByte(0xFF);
                    }
                    
                    // Check CCS bit (bit 30)
                    if (ocr_read[0] & 0x40) {
                        card_info.type = SD_TYPE_SDHC;
                    } else {
                        card_info.type = SD_TYPE_V2;
                    }
                }
            }
        }
    } else {
        // V1.x card or not SD card
        card_info.type = SD_TYPE_V1;
    }
    
    SD_CS_High();
    
    if (card_info.type == SD_TYPE_UNKNOWN) {
        return SD_ERROR;
    }
    
    card_info.initialized = true;
    return SD_OK;
}

/**
 * @brief Read single block from SD card
 */
SD_Status_t SD_ReadBlock(uint32_t block_addr, uint8_t *buffer) {
    SD_Status_t status;
    
    if (!card_info.initialized) {
        return SD_NOT_INITIALIZED;
    }
    
    // For SDSC cards, address is byte address
    if (card_info.type != SD_TYPE_SDHC) {
        block_addr *= SD_BLOCK_SIZE;
    }
    
    SD_CS_Low();
    
    // Send READ_SINGLE_BLOCK command
    if (SD_SendCommand(SD_CMD17, block_addr) != SD_R1_READY) {
        SD_CS_High();
        return SD_ERROR;
    }
    
    // Read data
    status = SD_ReadData(buffer, SD_BLOCK_SIZE);
    
    SD_CS_High();
    SD_SendByte(0xFF); // Dummy clock
    
    return status;
}

/**
 * @brief Write single block to SD card
 */
SD_Status_t SD_WriteBlock(uint32_t block_addr, const uint8_t *buffer) {
    uint8_t response;
    
    if (!card_info.initialized) {
        return SD_NOT_INITIALIZED;
    }
    
    // For SDSC cards, address is byte address
    if (card_info.type != SD_TYPE_SDHC) {
        block_addr *= SD_BLOCK_SIZE;
    }
    
    SD_CS_Low();
    
    // Send WRITE_BLOCK command
    if (SD_SendCommand(SD_CMD24, block_addr) != SD_R1_READY) {
        SD_CS_High();
        return SD_ERROR;
    }
    
    // Wait for card ready
    if (SD_WaitReady(SD_CMD_TIMEOUT) != SD_OK) {
        SD_CS_High();
        return SD_TIMEOUT;
    }
    
    // Send data token
    SD_SendByte(0xFE);
    
    // Send data
    for (uint16_t i = 0; i < SD_BLOCK_SIZE; i++) {
        SD_SendByte(buffer[i]);
    }
    
    // Send dummy CRC
    SD_SendByte(0xFF);
    SD_SendByte(0xFF);
    
    // Check response
    response = SD_SendByte(0xFF);
    if ((response & 0x1F) != 0x05) {
        SD_CS_High();
        return SD_ERROR;
    }
    
    // Wait for write completion
    if (SD_WaitReady(SD_READ_TIMEOUT) != SD_OK) {
        SD_CS_High();
        return SD_TIMEOUT;
    }
    
    SD_CS_High();
    SD_SendByte(0xFF); // Dummy clock
    
    return SD_OK;
}

/**
 * @brief Read multiple blocks from SD card
 */
SD_Status_t SD_ReadMultipleBlocks(uint32_t block_addr, uint8_t *buffer, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        SD_Status_t status = SD_ReadBlock(block_addr + i, buffer + (i * SD_BLOCK_SIZE));
        if (status != SD_OK) {
            return status;
        }
    }
    return SD_OK;
}

/**
 * @brief Get SD card info
 */
SD_CardInfo_t SD_GetCardInfo(void) {
    return card_info;
}

/**
 * @brief Check if SD card is initialized
 */
bool SD_IsInitialized(void) {
    return card_info.initialized;
}
