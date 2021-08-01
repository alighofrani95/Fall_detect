#include "ai_engine.h"

volatile int ai_status;

int sigmoid(const float *src) { return (src[0] < 0.1f ? 0 : 1); }

void ai_done(void *ctx) { ai_status = AI_DONE; }

void ai_infer(kpu_model_context_t *task, uint8_t *in, float **output, size_t *output_size) {
    ai_status = AI_RUNNING;
    kpu_run_kmodel(task, in, DMAC_CHANNEL5, ai_done, NULL);
    while (ai_status != AI_DONE)
        ;
    kpu_get_output(task, 0, (uint8_t **)output, output_size);
    *output_size /= sizeof(float);

    ai_status = AI_READY;
}

int is_ai_running() {
    return (ai_status == AI_RUNNING ? 1 : 0);
}
