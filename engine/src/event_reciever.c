#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "config.h"
#include "event_queue.h"
#include "event_types.h"
#include "event_parser.h"
#include "event_reciever.h"
#include "core_binding.h"



void *zmq_receiver_thread(void *arg)
{
    pin_thread_to_core(1, 95); // Assign this thread to core 1 with high priority (95) for real-time processing


    communication_args_t *args = (communication_args_t*)arg;
    event_queue_t *queue = args->queue;
    void *context = args->context;

    void *pull = zmq_socket(context, ZMQ_PULL);

    if (pull == NULL) {
        printf("[RECIEVER] Fallo al crear socket ZMQ: %s\n", zmq_strerror(zmq_errno()));
        return NULL; // Salir del hilo porque está roto
    }
    // Apagar Nagle también en la recepción del motor
    int hwm = 10;
    // zmq_setsockopt(pull, ZMQ_RCVHWM, &hwm, sizeof(hwm));


    char endpoint[64];
    snprintf(endpoint, sizeof(endpoint), "tcp://*:%s", ZMQ_PORT);
    int rc = zmq_bind(pull, endpoint);
    if (rc != 0) {
        fprintf(stderr, "[RECIEVER] Failed to bind to endpoint: %s\n", zmq_strerror(errno));
        zmq_close(pull);
        zmq_ctx_destroy(context);
        return NULL;
    }else{
        printf("[RECIEVER] Successfully bound to endpoint %s\n", endpoint);
    }

    int timeout_ms = 500;
    zmq_setsockopt(pull, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));

    uint8_t buffer[ZMQ_BUFFER_SIZE];

    while(keep_running){ // Esto se controla desde la señal de Ctrl+C para permitir una terminación limpia
        int size = zmq_recv(pull, buffer, sizeof(buffer), 0);
        if(size == -1) continue;

        // printf("[RECIEVER] Received event of size %d bytes\n", size);
        event_t event;

        if(deserialize_event(queue, buffer, size) != 0){
            printf("[RECIEVER] Failed to deserialize event\n");
        }
    }

    printf("[RECIEVER] Detected shutdown signal. Exiting ZeroMQ receiver thread.\n");

    zmq_close(pull);
    zmq_ctx_term (context);

    printf("[RECIEVER] ZeroMQ receiver thread terminated cleanly.\n");
    
    return NULL;
}



int start_zmq_receiver(event_queue_t *queue, void *context)
{
    pthread_t thread;

    communication_args_t *args = (communication_args_t*) malloc(sizeof(communication_args_t));

    args->queue = queue;
    args->context = context;

    printf("[RECIEVER] Starting ZeroMQ receiver thread...\n");

    if(pthread_create(&thread, NULL, zmq_receiver_thread, args) != 0)
    {
        return -1;
    }

    pthread_detach(thread);
    return 0;
}