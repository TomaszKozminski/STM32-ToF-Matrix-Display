/**
 * @file animation_mapper.h
 * @brief Maps motion sensor zones or 2D coordinates to animation frame indices
 * 
 * This module provides coordinate-to-animation-frame mapping for a 64-frame
 * animation arranged in an 8x8 grid pattern. Supports two mapping modes:
 * 1. Direct zone mapping (0-63) for motion sensor integration
 * 2. 2D coordinate mapping (x,y) for flexible position-based animation control
 * 
 * Frame Layout:
 *   Frame index = x_index * 8 + y_index (where x,y ∈ [0,7])
 *   - Zone 0: x=0, y=0
 *   - Zone 8: x=1, y=0
 *   - Zone 17: x=2, y=1
 *   - Zone 63: x=7, y=7
 * 
 * Thread Safety: NOT thread-safe. Use from single task/thread only.
 * If multi-threaded access needed, add mutex to AnimationReader_t.
 */

#ifndef ANIMATION_MAPPER_H
#define ANIMATION_MAPPER_H

#include <stdint.h>
#include "animation_reader.h"

/**
 * @brief Convert 2D floating-point coordinates to frame index (0-63)
 * 
 * @param x X coordinate [0.0, 7.0] - automatically clamped to range
 * @param y Y coordinate [0.0, 7.0] - automatically clamped to range
 * 
 * @return Frame index (0-63)
 *         Calculated as: floor(x) * 8 + floor(y)
 * 
 * @note No I/O operations - pure calculation
 * @note Out-of-range coordinates are automatically clamped
 * 
 * Examples:
 *   x=0.5, y=0.3 → floor(0)=0, floor(0)=0 → frame 0
 *   x=1.9, y=0.0 → floor(1)=1, floor(0)=0 → frame 8
 *   x=2.0, y=1.0 → floor(2)=2, floor(1)=1 → frame 17
 *   x=7.5, y=7.5 → clamped to x=7.0, y=7.0 → frame 63
 */
uint32_t AnimationMapper_GetFrameIndex(float x, float y);

/**
 * @brief Load animation frame directly from motion sensor zone (0-63)
 * 
 * Direct mapping for motion sensor integration where sensor returns zone
 * number that maps directly to frame index. Simplest and fastest method.
 * 
 * @param[in] reader Initialized animation reader (opened file required)
 * @param[in] zone Motion sensor zone number [0, 63]
 *                 Automatically clamped to valid range
 * @param[out] frame_buffer Pre-allocated buffer for frame data
 *                          Size must be: ANIMATION_FRAME_SIZE_BYTES (16384 bytes)
 * 
 * @return ANIM_OK (0) on success
 *         ANIM_ERROR (3) if reader or buffer is NULL
 *         Other AnimationStatus_t on file I/O errors
 * 
 * @note Reads frame from SD card - blocks until complete
 * @note Frame data completely replaces buffer contents
 * 
 * Usage example:
 *   uint32_t frame_buf[ANIMATION_FRAME_SIZE_BYTES/4];
 *   AnimationStatus_t status = AnimationMapper_LoadFrameByZone(reader, 25, frame_buf);
 *   if (status == ANIM_OK) { // Frame 25 now in frame_buf }
 */
AnimationStatus_t AnimationMapper_LoadFrameByZone(AnimationReader_t* reader, 
                                                   uint32_t zone, 
                                                   uint32_t* frame_buffer);

/**
 * @brief Load animation frame using 2D floating-point coordinates
 * 
 * Flexible mapping for analog input sources (joystick, accelerometer, etc).
 * Coordinates are mapped to grid positions via flooring.
 * 
 * @param[in] reader Initialized animation reader (opened file required)
 * @param[in] x X coordinate [0.0, 7.0] - automatically clamped
 * @param[in] y Y coordinate [0.0, 7.0] - automatically clamped
 * @param[out] frame_buffer Pre-allocated buffer for frame data
 *                          Size must be: ANIMATION_FRAME_SIZE_BYTES (16384 bytes)
 * 
 * @return ANIM_OK (0) on success
 *         ANIM_ERROR (3) if reader or buffer is NULL
 *         Other AnimationStatus_t on file I/O errors
 * 
 * @note Internally calls AnimationMapper_GetFrameIndex() then AnimationReader_ReadFrame()
 * @note Reads frame from SD card - blocks until complete
 * 
 * Usage example:
 *   uint32_t frame_buf[ANIMATION_FRAME_SIZE_BYTES/4];
 *   AnimationStatus_t status = AnimationMapper_LoadFrame(reader, 2.5, 1.7, frame_buf);
 *   if (status == ANIM_OK) { // Frame 18 (2*8+1) now in frame_buf }
 */
AnimationStatus_t AnimationMapper_LoadFrameByZone(AnimationReader_t* reader, 
                                                   uint32_t zone, 
                                                   uint32_t* frame_buffer);

#endif // ANIMATION_MAPPER_H
