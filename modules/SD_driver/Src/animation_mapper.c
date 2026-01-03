/**
 * @file animation_mapper.c
 * @brief Implementation of animation frame mapping functions
 * 
 * Maps motion sensor zones (0-63) or 2D coordinates to animation frames
 * arranged in an 8x8 grid on SD card.
 * 
 * Typical usage flow:
 *   1. Open animation file with AnimationReader_Init()
 *   2. Call AnimationMapper_LoadFrameByZone() or AnimationMapper_LoadFrame()
 *   3. Access frame data in provided buffer
 *   4. Repeat step 2-3 for different frames as needed
 *   5. Close animation with AnimationReader_Destroy()
 */

#include "animation_mapper.h"
#include "animation_reader.h"
#include <math.h>

#define GRID_SIZE 8          /**< Animation grid is 8x8 (64 frames total) */
#define MAX_FRAME_INDEX 63   /**< Maximum valid frame index */

uint32_t AnimationMapper_GetFrameIndex(float x, float y) {
    // Input validation and clamping
    // Clamp coordinates to valid range [0.0, 7.0]
    if (x < 0.0f) x = 0.0f;
    if (x > 7.0f) x = 7.0f;
    if (y < 0.0f) y = 0.0f;
    if (y > 7.0f) y = 7.0f;
    
    // Convert floating-point grid coordinates to integer indices
    // floor() rounds down to get grid position
    // Example: x=2.7 → floor(2.7)=2, y=1.3 → floor(1.3)=1
    uint32_t ix = (uint32_t)floorf(x);
    uint32_t iy = (uint32_t)floorf(y);
    
    // Calculate linear frame index from 2D grid position
    // Frame layout: rows are X-axis, columns are Y-axis
    // frame_index = row * columns_per_row + column
    // frame_index = ix * GRID_SIZE + iy
    uint32_t frame_index = ix * GRID_SIZE + iy;
    
    // Safety check (should not be needed after clamping)
    if (frame_index > MAX_FRAME_INDEX) {
        frame_index = MAX_FRAME_INDEX;
    }
    
    return frame_index;
}

AnimationStatus_t AnimationMapper_LoadFrameByZone(AnimationReader_t* reader, 
                                                   uint32_t zone, 
                                                   uint32_t* frame_buffer) {
    // Null pointer validation
    if (!reader || !frame_buffer) {
        return ANIM_ERROR;
    }
    
    // Clamp zone to valid range (0-63)
    // Motion sensor may occasionally send out-of-range values
    if (zone > MAX_FRAME_INDEX) {
        zone = MAX_FRAME_INDEX;
    }
    
    // Zone directly maps to frame index (0-63)
    // This is the most efficient method for motion sensor integration
    // because sensor hardware already provides zone numbers 0-63
    // 
    // Mapping example:
    //   zone 0 = frame 0 (x=0, y=0)
    //   zone 1 = frame 1 (x=0, y=1)
    //   zone 8 = frame 8 (x=1, y=0)
    //   zone 17 = frame 17 (x=2, y=1)
    //   zone 63 = frame 63 (x=7, y=7)
    
    // Delegate to animation reader to load the frame from SD card
    return AnimationReader_ReadFrame(reader, zone, frame_buffer);
}

AnimationStatus_t AnimationMapper_LoadFrame(AnimationReader_t* reader, 
                                             float x, float y, 
                                             uint32_t* frame_buffer) {
    // Null pointer validation
    if (!reader || !frame_buffer) {
        return ANIM_ERROR;
    }
    
    // Convert 2D floating coordinates to frame index
    // This handles:
    //   - Coordinate clamping to [0.0, 7.0]
    //   - Floating-point to integer conversion via flooring
    //   - Linear index calculation for 8x8 grid
    uint32_t frame_index = AnimationMapper_GetFrameIndex(x, y);
    
    // Load frame from SD card using calculated index
    return AnimationReader_ReadFrame(reader, frame_index, frame_buffer);
}
