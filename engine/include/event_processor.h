#define EVENT_PROCESSOR_H

#include "event_types.h"
#include "event_queue.h"

typedef struct {
    event_queue_t *queue;
} event_processor_t;

// Initializes the event processor with a reference to the event queue
void event_processor_init(event_processor_t *processor, event_queue_t *queue);

// Processes events from the queue
int event_processor_process_next(event_processor_t *processor);

// Loop
void *event_processor_loop(void *arg);

int start_processor_loop(event_processor_t *processor);