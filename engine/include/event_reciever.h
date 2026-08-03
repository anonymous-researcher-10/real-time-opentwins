#define EVENT_RECEIVER_H

#include <stdint.h>
#include "event_queue.h"


// Start ZeroMQ loop to receive events and push them to the queue
int start_zmq_receiver(event_queue_t *queue, void *context);