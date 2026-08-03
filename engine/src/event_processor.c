#include "event_processor.h"
#include "event_dispatcher.h"
#include <stdio.h>
#include <unistd.h>
#include "core_binding.h"


void event_processor_init(event_processor_t *processor, event_queue_t *queue)
{
    processor->queue = queue;
}

int event_processor_process_next(event_processor_t *processor)
{
    event_t event;
    if(pop_event(processor->queue, &event) != -1) {
        dispatch_event(&event);
    }

    return 0;
}




void *event_processor_loop(void *arg)
{
    pin_thread_to_core(1, 95); // Assign this thread to core 2 with high priority (95) for real-time processing
    
    event_processor_t *processor = (event_processor_t*)arg;
    while(keep_running) {
        event_processor_process_next(processor);

        usleep(10); // Sleep 10 microseconds to prevent busy waiting
    }

    printf("[EVENT_PROCESSOR] Detected shutdown signal. Exiting event processor loop.\n");

    return NULL;
}

int start_processor_loop(event_processor_t *processor){
    pthread_t thread;

    if(pthread_create(&thread, NULL, event_processor_loop, (void*)processor) != 0)
    {
        return -1;
    }

    pthread_detach(thread);
    return 0;
}