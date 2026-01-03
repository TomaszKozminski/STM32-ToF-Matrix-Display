/**
 ******************************************************************************
 * @file    animation_reader.h
 * @brief   Animation reader interface (OOP pattern in C)
 ******************************************************************************
 */

#ifndef ANIMATION_READER_H
#define ANIMATION_READER_H

#include <stdint.h>
#include <stdbool.h>

/* Animation frame size: 4096 uint32_t = 16384 bytes */
#define ANIMATION_FRAME_SIZE_WORDS 4096
#define ANIMATION_FRAME_SIZE_BYTES (ANIMATION_FRAME_SIZE_WORDS * sizeof(uint32_t))

/* Animation metadata */
typedef struct {
    uint32_t frame_count;      // Total number of frames
    uint32_t fps;              // Frames per second
    uint32_t width;            // Frame width (for future use)
    uint32_t height;           // Frame height (for future use)
    char name[32];             // Animation name
} AnimationMetadata_t;

/* Animation status */
typedef enum {
    ANIM_OK = 0,
    ANIM_ERROR,
    ANIM_FILE_NOT_FOUND,
    ANIM_INVALID_FRAME,
    ANIM_READ_ERROR,
    ANIM_NOT_INITIALIZED
} AnimationStatus_t;

/* Forward declaration */
struct AnimationReader;

/* Animation reader interface (virtual methods) */
typedef struct {
    AnimationStatus_t (*init)(struct AnimationReader* self, const char* path);
    AnimationStatus_t (*get_metadata)(struct AnimationReader* self, AnimationMetadata_t* metadata);
    AnimationStatus_t (*read_frame)(struct AnimationReader* self, uint32_t frame_index, uint32_t* buffer);
    AnimationStatus_t (*read_frame_range)(struct AnimationReader* self, uint32_t start_frame, uint32_t count, uint32_t* buffer);
    void (*deinit)(struct AnimationReader* self);
} AnimationReaderVTable_t;

/* Animation reader base class */
typedef struct AnimationReader {
    const AnimationReaderVTable_t* vtable;
    void* private_data;
    bool initialized;
} AnimationReader_t;

/* Public API (calls virtual methods) */
static inline AnimationStatus_t AnimationReader_Init(AnimationReader_t* reader, const char* path) {
    return reader->vtable->init(reader, path);
}

static inline AnimationStatus_t AnimationReader_GetMetadata(AnimationReader_t* reader, AnimationMetadata_t* metadata) {
    return reader->vtable->get_metadata(reader, metadata);
}

static inline AnimationStatus_t AnimationReader_ReadFrame(AnimationReader_t* reader, uint32_t frame_index, uint32_t* buffer) {
    return reader->vtable->read_frame(reader, frame_index, buffer);
}

static inline AnimationStatus_t AnimationReader_ReadFrameRange(AnimationReader_t* reader, uint32_t start_frame, uint32_t count, uint32_t* buffer) {
    return reader->vtable->read_frame_range(reader, start_frame, count, buffer);
}

static inline void AnimationReader_Deinit(AnimationReader_t* reader) {
    reader->vtable->deinit(reader);
}

#endif /* ANIMATION_READER_H */
