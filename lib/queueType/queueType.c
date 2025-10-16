#include <stdint.h>
#include <stdlib.h>

#include "queueType.h"

//function to create a queue
Queue_uint32 *createQueue_uint32(uint8_t initialCapacity) {
    //minimum capacity of 2
    if (initialCapacity == 0) {
        initialCapacity = 2;
    }

    //make sure initial capacity is even to make halving work correctly
    initialCapacity = initialCapacity + initialCapacity % 2;

    // Allocate memory for the queue structure
    Queue_uint32 *queue = (Queue_uint32*) malloc(sizeof(Queue_uint32));

    //Check for malloc failure
    if (!queue) return 0;

    // Allocate memory for the data array
    queue->data = (uint32_t*) malloc(initialCapacity * sizeof(uint32_t));

    //Check for malloc failure
    if (!queue->data) {
        free(queue);
        return 0;
    }

    // Initialize properties
    queue->capacity = initialCapacity;
    queue->size = 0;

    // Return pointer to the created queue
    return queue;
}

//function to free the queue
void freeQueue_uint32(Queue_uint32 *queue) {
    if (queue) {
        free(queue->data);
        free(queue);
    }
}

//function to add an element to the queue
uint8_t enqueue_uint32(Queue_uint32 *queue, uint32_t value) {
    // Check if the queue is full
    if (queue->size >= queue->capacity) {
        // reallocate memory to double the capacity
        uint32_t *newData = (uint32_t*) realloc(queue->data, queue->capacity * 2 * sizeof(uint32_t));

        //Check for realloc failure
        if (!newData) {
            return 1;
        }

        //update queue properties
        queue->data = newData;
        queue->capacity *= 2;
    }
    queue->data[queue->size++] = value;
    return 0;
}

//function to remove and return the head from the queue
uint32_t dequeue_uint32(Queue_uint32 *queue) {
    //queue is empty
    if (queue->size == 0) {
        return 1;
    }

    //store the head value to return later
    uint32_t value = queue->data[0];

    //shift elements to the left
    for (uint8_t i = 1; i < queue->size; i++) {
        queue->data[i - 1] = queue->data[i];
    }

    queue->size--;

    //check if queue capacity can be halved to save memory
    if (queue->size > 0 && queue->size <= queue->capacity / 2) {
        uint32_t *newData = (uint32_t*) realloc(queue->data, queue->capacity / 2 * sizeof(uint32_t));

        //Check for realloc failure
        if (newData) {
            queue->data = newData;
            queue->capacity /= 2;
        }
    }
    return value;
}

