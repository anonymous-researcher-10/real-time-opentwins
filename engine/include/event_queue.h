#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stddef.h>
#include "event_types.h"      
#include "config.h"



// Initializes the event queue
void init_queue(event_queue_t* q);

// Enqueues an event into the queue
int push_event(event_queue_t* q, event_t event);

// Dequeues an event from the queue
int pop_event(event_queue_t* q, event_t* event);

#endif // EVENT_QUEUE_H