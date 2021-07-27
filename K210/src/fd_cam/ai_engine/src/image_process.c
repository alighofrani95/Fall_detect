#include "image_process.h"

#include <stdlib.h>
#include <stdio.h>

#include "iomem.h"

static inline int fast_roundf(float x) { return (int)(x + 0.5); }

int image_init(image_t *image) {
    image->addr = iomem_malloc(image->width * image->height * image->depth);
    if (image->addr == NULL) return -1;

    return 0;
}

void image_deinit(image_t *image) { iomem_free(image->addr); }

void image_resize(uint8_t *in, int w0, int h0, uint8_t *out, int w, int h) {
    float sx = (float)(w0) / w;
    float sy = (float)(h0) / h;
    int x, y, x0, y0, x1, y1, val_x1, val_y1;
    float xf, yf;
    // mp_obj_t image = py_image(w, h, img->bpp, out);
    if (w >= w0 || h >= h0) {
        for (y = 0; y < h; y++) {
            yf = (y + 0.5) * sy - 0.5;
            y0 = (int)yf;
            y1 = y0 + 1;
            val_y1 = y0 < h0 - 1 ? y1 : y0;
            for (x = 0; x < w; x++) {
                xf = (x + 0.5) * sx - 0.5;
                x0 = (int)xf;
                x1 = x0 + 1;
                val_x1 = x0 < w0 - 1 ? x1 : x0;
                out[y * w + x] =
                    (uint8_t)(in[y0 * w0 + x0] * (x1 - xf) * (y1 - yf) +
                              in[y0 * w0 + val_x1] * (xf - x0) * (y1 - yf) +
                              in[val_y1 * w0 + x0] * (x1 - xf) * (yf - y0) +
                              in[val_y1 * w0 + val_x1] * (xf - x0) * (yf - y0));
            }
        }
    } else {
        for (y = 0; y < h; y++) {
            y0 = y * sy;
            y1 = (y + 1) * sy;
            for (x = 0; x < w; x++) {
                x0 = x * sy;
                x1 = (x + 1) * sy;
                int sum, xx, yy;
                sum = 0;
                for (yy = y0; yy <= y1; yy++) {
                    for (xx = x0; xx <= x1; xx++) {
                        sum += in[yy * w0 + xx];
                    }
                }
                out[y * w + x] =
                    sum / ((y1 - y0 + 1) *
                           (x1 - x0 + 1));  // avg to get better picture
            }
        }
    }
    // return image;
}
