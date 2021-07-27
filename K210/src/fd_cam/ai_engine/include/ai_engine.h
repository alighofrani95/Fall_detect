#ifndef _AI_ENGINE_H
#define _AI_ENGINE_H

#include <kpu.h>
// #include "image_process.h"

enum ai_status {
    AI_READY,
    AI_RUNNING,
    AI_DONE,
};

int sigmoid(const float *src);
void ai_done(void *ctx);
void ai_infer(kpu_model_context_t *task, uint8_t *in, float **output, size_t *output_size);

int is_ai_running();

#endif