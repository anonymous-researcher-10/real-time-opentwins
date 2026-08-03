#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "event_queue.h"
#include "event_processor.h"
#include "event_reciever.h"
#include "event_dispatcher.h"



int main()
{
    event_queue_t queue;

    init_queue(&queue);
    start_zmq_receiver(&queue);

     while(1){
        event_t event;
        if(pop_event(&queue, &event) == 0){
            dispatch_event(&event);
        } else {
            usleep(10);
        }
    }

    return 0;
}