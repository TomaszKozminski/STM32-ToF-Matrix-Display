#ifndef _LED_MATRIX_FRAME_H
#define _LED_MATRIX_FRAME_H

#include <inttypes.h>

// format klatki wyświtlanej przez matrycę
typedef uint32_t * Frame;

/**
 * @brief alokuje pamięć na klatkę wyświtlaną przez matrycę
 * @returns wskaźnik do zaalokowanych danych
 */
Frame Frame_Create(void);

/**
 * @brief zwalnia zaalokowane dane
 * @param frame wskażnik na dane do zwolnienia
 */
void Frame_Delete(Frame frame);


#endif /* _LED_MATRIX_FRAME_H */