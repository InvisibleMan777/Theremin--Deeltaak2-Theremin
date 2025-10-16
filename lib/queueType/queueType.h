#ifndef QUEUETYPE_H
#define QUEUETYPE_H

#include <stdint.h>

struct Queue_uint32 {
    uint32_t *data;
    uint8_t size;
    uint8_t capacity;
};

typedef struct Queue_uint32 Queue_uint32;

//function to create a queue
Queue_uint32 *createQueue_uint32(uint8_t initialCapacity);

//function to free the queue
void freeQueue_uint32(Queue_uint32 *queue);

//function to add an element to the queue
uint8_t enqueue_uint32(Queue_uint32 *queue, uint32_t value);

//function to remove and return the head from the queue
uint32_t dequeue_uint32(Queue_uint32 *queue);

#endif 