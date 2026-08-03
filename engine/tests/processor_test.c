#include "event_queue.h"
#include "event_processor.h"
#include <stdio.h>
#include <string.h>

int main()
{
    event_queue_t queue;
    event_processor_t processor;

    init_queue(&queue);
    event_processor_init(&processor, &queue);

    event_t ev;

    ev.header.type = EVENT_UPDATE;
    ev.header.twin_id = 1;
    ev.header.timestamp = 123456789;
    ev.payload.update.updates[0].var_id = 1;
    ev.payload.update.updates[0].value.f32 = 22.5;
    ev.payload.update.updates[1].var_id = 2;
    ev.payload.update.updates[1].value.f32 = 1013.0;

    push_event(&queue, ev);

    event_processor_process_next(&processor);


    event_t evt_error;
    evt_error.header.type = EVENT_ERROR;
    evt_error.header.twin_id = 42;
    evt_error.header.timestamp = 123456790;
    evt_error.payload.error.error_code = ERROR_SENSOR_FAILURE;
    evt_error.payload.error.message[0] = '\0';  // Initialize the message as an empty string

    push_event(&queue, evt_error);

    event_processor_process_next(&processor);

    return 0;
}