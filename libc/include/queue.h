#ifndef _QUEUE_H
#define _QUEUE_H 1

#include <stdint.h>
#include <stdbool.h>

typedef struct queue_element {
    uint32_t value;
    struct queue_element *next;
} queue_element_t;

typedef struct queue {
    queue_element_t *head;
    queue_element_t *tail;
    uint32_t element_count;
} queue_t;

queue_t *queue_create();
bool queue_is_empty();
queue_t *queue_enqueue(queue_t *q, uint32_t value);
uint32_t queue_dequeue(queue_t *q);

#endif
