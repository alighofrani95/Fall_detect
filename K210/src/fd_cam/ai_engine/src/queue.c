#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void initQueue(struct QUEUE *q) {
    q->count = 0;
    q->front = 0;
}

void enQueue(struct QUEUE *q, image_t element) {
    int index;
    index = (q->count + q->front) % (sizeof(q->array) / sizeof(image_t));
    if (index == q->front && q->count != 0) {
        deQueue(q);
    }
    q->count++;
    q->array[index] = element;
}

void deQueue(struct QUEUE *q) {
    image_t elem;
    if (q->count == 0) {
        return (-1000);
    }
    elem = q->array[q->front];
    q->count--;
    q->front = (q->front + 1) % (sizeof(q->array) / sizeof(int));
    // return elem;
    return ;
}

void display(struct QUEUE *q) {
    int i, index;
    if (q->count == 0) {
        printf(" {}");
    } else {
        for (index = q->front, i = 1; i <= q->count; i++, index++) {
            printf("%d ", q->array[index % (sizeof(q->array) / sizeof(image_t))]);
        }
        printf("\n");
    }
}
