#define EVENT_HANDLER_H
#include "event_types.h"
#include "event_queue.h"

void event_dispatcher_init(event_queue_t *queue_out_notify, event_queue_t *queue_out_user_actions);
void dispatch_event(event_t *event);