/**
 ******************************************************************************
 * @file    sd_animation_reader.h
 * @brief   SD Card animation reader implementation
 ******************************************************************************
 */

#ifndef SD_ANIMATION_READER_H
#define SD_ANIMATION_READER_H

#include "animation_reader.h"
#include "ff.h"

/* SD Animation Reader private data */
typedef struct {
    FIL file;
    AnimationMetadata_t metadata;
    char file_path[64];
} SDAnimationReaderData_t;

/* SD Animation Reader (extends AnimationReader) */
typedef struct {
    AnimationReader_t base;
    SDAnimationReaderData_t data;
} SDAnimationReader_t;

/* Constructor/Destructor */
void SDAnimationReader_Create(SDAnimationReader_t* reader);
void SDAnimationReader_Destroy(SDAnimationReader_t* reader);

#endif /* SD_ANIMATION_READER_H */
