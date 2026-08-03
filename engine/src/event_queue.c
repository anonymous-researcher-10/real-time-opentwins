#include "event_queue.h"



void init_queue(event_queue_t* q)
{
    q->head = 0;
    q->tail = 0;
}

int push_event(event_queue_t* q, event_t event)
{
    size_t next_head = (q->head + 1) % QUEUE_SIZE;

    if (next_head == q->tail) {
        // Queue is full
        return -1;
    }

    q->buffer[q->head] = event;
    q->head = next_head;
    return 0;
}

int pop_event(event_queue_t* q, event_t* event)
{
    if (q->head == q->tail) {
        // Queue is empty
        return -1;
    }

    *event = q->buffer[q->tail];
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    return 0;
}