/**
 ******************************************************************************
 * @file    sd_animation_reader.c
 * @brief   SD Card animation reader implementation
 ******************************************************************************
 */

#include "sd_animation_reader.h"
#include <string.h>
#include <stdio.h>

/* Animation file format:
 * Header (64 bytes):
 *   - Magic: "ANIM" (4 bytes)
 *   - Version: uint32_t (4 bytes)
 *   - Frame count: uint32_t (4 bytes)
 *   - FPS: uint32_t (4 bytes)
 *   - Width: uint32_t (4 bytes)
 *   - Height: uint32_t (4 bytes)
 *   - Name: char[32] (32 bytes)
 *   - Reserved: 8 bytes
 * Frames: Each frame is 16384 bytes (4096 uint32_t)
 */

#define ANIM_MAGIC 0x4D494E41  // "ANIM" in little-endian
#define ANIM_VERSION 1
#define ANIM_HEADER_SIZE 64

/* Forward declarations of virtual methods */
static AnimationStatus_t SDAnimationReader_Init(AnimationReader_t* self, const char* path);
static AnimationStatus_t SDAnimationReader_GetMetadata(AnimationReader_t* self, AnimationMetadata_t* metadata);
static AnimationStatus_t SDAnimationReader_ReadFrame(AnimationReader_t* self, uint32_t frame_index, uint32_t* buffer);
static AnimationStatus_t SDAnimationReader_ReadFrameRange(AnimationReader_t* self, uint32_t start_frame, uint32_t count, uint32_t* buffer);
static void SDAnimationReader_Deinit(AnimationReader_t* self);

/* VTable for SD Animation Reader */
static const AnimationReaderVTable_t sd_animation_reader_vtable = {
    .init = SDAnimationReader_Init,
    .get_metadata = SDAnimationReader_GetMetadata,
    .read_frame = SDAnimationReader_ReadFrame,
    .read_frame_range = SDAnimationReader_ReadFrameRange,
    .deinit = SDAnimationReader_Deinit
};

/**
 * @brief Create SD Animation Reader instance
 */
void SDAnimationReader_Create(SDAnimationReader_t* reader) {
    if (reader == NULL) return;
    
    memset(reader, 0, sizeof(SDAnimationReader_t));
    reader->base.vtable = &sd_animation_reader_vtable;
    reader->base.private_data = &reader->data;
    reader->base.initialized = false;
}

/**
 * @brief Destroy SD Animation Reader instance
 */
void SDAnimationReader_Destroy(SDAnimationReader_t* reader) {
    if (reader == NULL) return;
    
    if (reader->base.initialized) {
        SDAnimationReader_Deinit(&reader->base);
    }
}

/**
 * @brief Initialize animation reader with file path
 */
static AnimationStatus_t SDAnimationReader_Init(AnimationReader_t* self, const char* path) {
    if (self == NULL || path == NULL) {
        return ANIM_ERROR;
    }
    
    SDAnimationReaderData_t* data = (SDAnimationReaderData_t*)self->private_data;
    
    // Store file path
    strncpy(data->file_path, path, sizeof(data->file_path) - 1);
    data->file_path[sizeof(data->file_path) - 1] = '\0';
    
    // Open file
    FRESULT fres = f_open(&data->file, path, FA_READ);
    if (fres != FR_OK) {
        return ANIM_FILE_NOT_FOUND;
    }
    
    // Read and validate header
    uint8_t header[ANIM_HEADER_SIZE];
    UINT bytes_read;
    fres = f_read(&data->file, header, ANIM_HEADER_SIZE, &bytes_read);
    if (fres != FR_OK || bytes_read != ANIM_HEADER_SIZE) {
        f_close(&data->file);
        return ANIM_READ_ERROR;
    }
    
    // Parse header
    uint32_t* header_words = (uint32_t*)header;
    uint32_t magic = header_words[0];
    uint32_t version = header_words[1];
    
    if (magic != ANIM_MAGIC || version != ANIM_VERSION) {
        f_close(&data->file);
        return ANIM_ERROR;
    }
    
    // Extract metadata
    data->metadata.frame_count = header_words[2];
    data->metadata.fps = header_words[3];
    data->metadata.width = header_words[4];
    data->metadata.height = header_words[5];
    memcpy(data->metadata.name, &header[24], 32);
    data->metadata.name[31] = '\0';
    
    // Close file (will reopen on read)
    f_close(&data->file);
    
    self->initialized = true;
    return ANIM_OK;
}

/**
 * @brief Get animation metadata
 */
static AnimationStatus_t SDAnimationReader_GetMetadata(AnimationReader_t* self, AnimationMetadata_t* metadata) {
    if (self == NULL || metadata == NULL || !self->initialized) {
        return ANIM_NOT_INITIALIZED;
    }
    
    SDAnimationReaderData_t* data = (SDAnimationReaderData_t*)self->private_data;
    memcpy(metadata, &data->metadata, sizeof(AnimationMetadata_t));
    
    return ANIM_OK;
}

/**
 * @brief Read single animation frame
 */
static AnimationStatus_t SDAnimationReader_ReadFrame(AnimationReader_t* self, uint32_t frame_index, uint32_t* buffer) {
    if (self == NULL || buffer == NULL || !self->initialized) {
        return ANIM_NOT_INITIALIZED;
    }
    
    SDAnimationReaderData_t* data = (SDAnimationReaderData_t*)self->private_data;
    
    // Validate frame index
    if (frame_index >= data->metadata.frame_count) {
        return ANIM_INVALID_FRAME;
    }
    
    // Open file
    FRESULT fres = f_open(&data->file, data->file_path, FA_READ);
    if (fres != FR_OK) {
        return ANIM_FILE_NOT_FOUND;
    }
    
    // Seek to frame position (skip header + previous frames)
    FSIZE_t frame_offset = ANIM_HEADER_SIZE + (frame_index * ANIMATION_FRAME_SIZE_BYTES);
    fres = f_lseek(&data->file, frame_offset);
    if (fres != FR_OK) {
        f_close(&data->file);
        return ANIM_READ_ERROR;
    }
    
    // Read frame data
    UINT bytes_read;
    fres = f_read(&data->file, buffer, ANIMATION_FRAME_SIZE_BYTES, &bytes_read);
    f_close(&data->file);
    
    if (fres != FR_OK || bytes_read != ANIMATION_FRAME_SIZE_BYTES) {
        return ANIM_READ_ERROR;
    }
    
    return ANIM_OK;
}

/**
 * @brief Read range of animation frames
 */
static AnimationStatus_t SDAnimationReader_ReadFrameRange(AnimationReader_t* self, uint32_t start_frame, uint32_t count, uint32_t* buffer) {
    if (self == NULL || buffer == NULL || !self->initialized || count == 0) {
        return ANIM_NOT_INITIALIZED;
    }
    
    SDAnimationReaderData_t* data = (SDAnimationReaderData_t*)self->private_data;
    
    // Validate range
    if (start_frame >= data->metadata.frame_count || 
        (start_frame + count) > data->metadata.frame_count) {
        return ANIM_INVALID_FRAME;
    }
    
    // Open file
    FRESULT fres = f_open(&data->file, data->file_path, FA_READ);
    if (fres != FR_OK) {
        return ANIM_FILE_NOT_FOUND;
    }
    
    // Seek to start frame
    FSIZE_t frame_offset = ANIM_HEADER_SIZE + (start_frame * ANIMATION_FRAME_SIZE_BYTES);
    fres = f_lseek(&data->file, frame_offset);
    if (fres != FR_OK) {
        f_close(&data->file);
        return ANIM_READ_ERROR;
    }
    
    // Read all frames in one go
    UINT bytes_to_read = count * ANIMATION_FRAME_SIZE_BYTES;
    UINT bytes_read;
    fres = f_read(&data->file, buffer, bytes_to_read, &bytes_read);
    f_close(&data->file);
    
    if (fres != FR_OK || bytes_read != bytes_to_read) {
        return ANIM_READ_ERROR;
    }
    
    return ANIM_OK;
}

/**
 * @brief Deinitialize animation reader
 */
static void SDAnimationReader_Deinit(AnimationReader_t* self) {
    if (self == NULL || !self->initialized) {
        return;
    }
    
    SDAnimationReaderData_t* data = (SDAnimationReaderData_t*)self->private_data;
    
    // Close file if open (should be closed already)
    f_close(&data->file);
    
    self->initialized = false;
}
