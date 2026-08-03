#define EVENT_SENDER_H

#include <stdint.h>
#include "event_queue.h"


// Start ZeroMQ loop to receive events and push them to the queue
int start_zmq_sender(event_queue_t *queue, void *context);