/**
 ******************************************************************************
 * @file    sd_spi.h
 * @brief   SD Card SPI driver header
 ******************************************************************************
 */

#ifndef SD_SPI_H
#define SD_SPI_H

#include <stdint.h>
#include <stdbool.h>

/* SD Card Commands */
#define SD_CMD0   0   // GO_IDLE_STATE
#define SD_CMD8   8   // SEND_IF_COND
#define SD_CMD17  17  // READ_SINGLE_BLOCK
#define SD_CMD24  24  // WRITE_BLOCK
#define SD_CMD55  55  // APP_CMD
#define SD_CMD58  58  // READ_OCR
#define SD_ACMD41 41  // SD_SEND_OP_COND

/* SD Card Response Types */
#define SD_R1_READY     0x00
#define SD_R1_IDLE      0x01

/* SD Card Timeouts */
#define SD_INIT_TIMEOUT 1000
#define SD_CMD_TIMEOUT  100
#define SD_READ_TIMEOUT 500

/* SD Card Block Size */
#define SD_BLOCK_SIZE 512

/* SD Card Status */
typedef enum {
    SD_OK = 0,
    SD_ERROR,
    SD_TIMEOUT,
    SD_NOT_INITIALIZED
} SD_Status_t;

/* SD Card Type */
typedef enum {
    SD_TYPE_UNKNOWN = 0,
    SD_TYPE_V1,
    SD_TYPE_V2,
    SD_TYPE_SDHC
} SD_CardType_t;

/* SD Card Info */
typedef struct {
    SD_CardType_t type;
    uint32_t capacity;      // in blocks
    bool initialized;
} SD_CardInfo_t;

/* Public Functions */
SD_Status_t SD_Init(void);
SD_Status_t SD_ReadBlock(uint32_t block_addr, uint8_t *buffer);
SD_Status_t SD_WriteBlock(uint32_t block_addr, const uint8_t *buffer);
SD_Status_t SD_ReadMultipleBlocks(uint32_t block_addr, uint8_t *buffer, uint32_t count);
SD_CardInfo_t SD_GetCardInfo(void);
bool SD_IsInitialized(void);

#endif /* SD_SPI_H */
