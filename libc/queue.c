#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <kernel/vmm.h>
#include <queue.h>

queue_t *queue_create() {
    queue_t *q = (queue_t*)kmalloc(sizeof(queue_t));    
    memset(q, 0, sizeof(queue_t));

    q->head = NULL;
    q->tail = NULL;
    q->element_count = 0;
}

bool queue_is_empty(queue_t *q) {
    if (q->element_count == 0) return true;

    return false;
}

queue_t *queue_enqueue(queue_t *q, uint32_t value) {
    queue_element_t *temp = (queue_element_t*)kmalloc(sizeof(queue_element_t));
    temp->value = value;
    temp->next = NULL;

    if (queue_is_empty(q)) {
        q->head = temp;
        q->tail = temp;
    } else {
        q->tail->next = temp;
        q->tail = temp;
    }

    q->element_count += 1;
}

uint32_t queue_dequeue(queue_t *q) {
    queue_element_t *temp = q->head;
    uint32_t val = temp->value;

    if (queue_is_empty(q)) {
        panic("Cannot dequeue on an empty list.");
    }

    q->head = q->head->next;
    q->element_count -= 1;

    return val;
}
