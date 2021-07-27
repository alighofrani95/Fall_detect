#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "global_config.h"
#include "image_process.h"

struct QUEUE {
    image_t array[FRAMES_NUM];
    int front;
    int count;
};

void initQueue(struct QUEUE *p);
void enQueue(struct QUEUE *p, image_t element);
void deQueue(struct QUEUE *p);
void display(struct QUEUE *p);

#endif