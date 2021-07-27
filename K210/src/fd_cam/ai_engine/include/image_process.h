#ifndef _IMAGE_PROCESS_H
#define _IMAGE_PROCESS_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *addr;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
} image_t;


int image_init(image_t *image);
void image_deinit(image_t *image);
void image_resize(uint8_t *in, int w0, int h0, uint8_t *out, int w, int h);

#endif /* _IMAGE_PROCESS_H */