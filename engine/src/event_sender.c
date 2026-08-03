#define _POSIX_C_SOURCE 200809L
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
#include "event_sender.h"
#include "core_binding.h"

static inline uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void serialize_event(event_t *event, uint8_t *buffer, size_t *size)
{
    cw_pack_context pc;
    cw_pack_context_init(&pc, buffer, ZMQ_BUFFER_SIZE, 0);

    cw_pack_array_size(&pc, 5 + event->payload.update.count * 2);

    cw_pack_unsigned(&pc, 0);
    cw_pack_unsigned(&pc, event->header.timestamp);
    cw_pack_unsigned(&pc, (uint64_t)event->header.deviation);
    cw_pack_unsigned(&pc, event->header.twin_id);
    cw_pack_unsigned(&pc, event->payload.update.count);

    for (int i=0; i<event->payload.update.count; i++){
        cw_pack_unsigned(&pc, event->payload.update.updates[i].var_id);
        cw_pack_float(&pc, event->payload.update.updates[i].value.f32);
    }

    *size = pc.current - pc.start;

}

void send_event(void *publisher, event_t *event)
{
    // printf("[EVENT SENDER] Enviando evento: timestamp=%lu, deviation=%.2f, twin_id=%u, var_count=%u\n",
        //    event->header.timestamp, event->header.deviation, event->header.twin_id, event->payload.update.count);
    uint8_t buffer[ZMQ_BUFFER_SIZE];
    size_t size;
    serialize_event(event, buffer, &size);
    

    unsigned int idx = atomic_fetch_add_explicit(&log_index2, 1, memory_order_relaxed);
    if (idx < MAX_HISTORY_RECORDS) {
        history_log2[idx].timestamp_ns = get_time_ns();
        history_log2[idx].value = event->payload.update.updates[0].value.f32; // Solo para pruebas, lo eliminaremos después
        history_log2[idx].twin_id = event->header.twin_id; // Solo para pruebas, lo eliminaremos después
    }
    

    // printf("[EVENT SENDER] Enviando evento serializado de tamaño %zu bytes\n", size);
    int result = zmq_send(publisher, buffer, size, 0);
    // if (result == -1) {
    //     fprintf(stderr, "[EVENT SENDER] Failed to send event: %s\n", zmq_strerror(errno));
    // }
}

void *zmq_sender_thread(void *arg)
{

    pin_thread_to_core(1, 95); // Assign this thread to core 3 with high priority (95) for real-time processing

    communication_args_t *args = (communication_args_t*)arg;
    event_queue_t *queue = args->queue;
    void *context = args->context;



    // Init all connectors to send data to the cloud or other device:

    void *connectors[MAX_TWINS];

    //###########################################################################################################
    //############ ESTO ES PROVISIONAL PARA LAS PRUEBAS, LUEGO HAY QUE PONERLO PARA CUALQUIER COSA ##############
    //###########################################################################################################


    // Create socket for RBPI3

    connectors[0] = zmq_socket(context, ZMQ_PUSH);

    if (connectors[0] == NULL) {
        printf("[EVENT SENDER] Fallo al crear socket ZMQ: %s\n", zmq_strerror(zmq_errno()));
        return NULL; // Salir del hilo porque está roto
    }

    int rc = zmq_connect(connectors[0], RBPI3_url);

    if (rc != 0) {
        fprintf(stderr, "[EVENT SENDER] Failed to connect to server: %s\n", zmq_strerror(errno));
        zmq_close(connectors[0]);
        zmq_ctx_destroy(context);
        return NULL;
    }else{
        printf("[EVENT SENDER] Successfully connected to server at %s\n", RBPI3_url);
    }


    // Create socket for RBPI4

    connectors[1] = zmq_socket(context, ZMQ_PUSH);

    if (connectors[1] == NULL) {
        printf("[EVENT SENDER] Fallo al crear socket ZMQ: %s\n", zmq_strerror(zmq_errno()));
        return NULL; // Salir del hilo porque está roto
    }


    rc = zmq_connect(connectors[1], RBPI4_url);

    if (rc != 0) {
        fprintf(stderr, "[EVENT SENDER] Failed to connect to server: %s\n", zmq_strerror(errno));
        zmq_close(connectors[1]);
        zmq_ctx_destroy(context);
        return NULL;
    }else{
        printf("[EVENT SENDER] Successfully connected to server at %s\n", RBPI4_url);
    }


    // Create socket for RBPI5

    connectors[2] = zmq_socket(context, ZMQ_PUSH);

    if (connectors[2] == NULL) {
        printf("[EVENT SENDER] Fallo al crear socket ZMQ: %s\n", zmq_strerror(zmq_errno()));
        return NULL; // Salir del hilo porque está roto
    }


    rc = zmq_connect(connectors[2], RBPI5_url);

    if (rc != 0) {
        fprintf(stderr, "[EVENT SENDER] Failed to connect to server: %s\n", zmq_strerror(errno));
        zmq_close(connectors[2]);
        zmq_ctx_destroy(context);
        return NULL;
    }else{
        printf("[EVENT SENDER] Successfully connected to server at %s\n", RBPI5_url);
    }    


    // Create socket for RBPI6

    connectors[3] = zmq_socket(context, ZMQ_PUSH);

    if (connectors[3] == NULL) {
        printf("[EVENT SENDER] Fallo al crear socket ZMQ: %s\n", zmq_strerror(zmq_errno()));
        return NULL; // Salir del hilo porque está roto
    }


    rc = zmq_connect(connectors[3], RBPI6_url);

    if (rc != 0) {
        fprintf(stderr, "[EVENT SENDER] Failed to connect to server: %s\n", zmq_strerror(errno));
        zmq_close(connectors[3]);
        zmq_ctx_destroy(context);
        return NULL;
    }else{
        printf("[EVENT SENDER] Successfully connected to server at %s\n", RBPI6_url);
    }

    // Create socket for RBPI7

    connectors[4] = zmq_socket(context, ZMQ_PUSH);

    if (connectors[4] == NULL) {
        printf("[EVENT SENDER] Fallo al crear socket ZMQ: %s\n", zmq_strerror(zmq_errno()));
        return NULL; // Salir del hilo porque está roto
    }

    rc = zmq_connect(connectors[4], RBPI7_url);
    if (rc != 0) {
        fprintf(stderr, "[EVENT SENDER] Failed to connect to server: %s\n", zmq_strerror(errno));
        zmq_close(connectors[4]);
        zmq_ctx_destroy(context);
        return NULL;
    }else{
        printf("[EVENT SENDER] Successfully connected to server at %s\n", RBPI7_url);
    }

    //###########################################################################################################


    while(keep_running) {
        event_t event;
        if(pop_event(queue, &event) != -1) {
            send_event(connectors[event.header.twin_id], &event);
        }
        usleep(10); // Sleep 10 microseconds to prevent busy waiting
    }


    printf("[EVENT SENDER] Detected shutdown signal. Exiting ZeroMQ sender thread.\n");


    for (int i=0; i<5; i++){  // TODO: Cambiar esto por el número real de conectores que tengamos
        zmq_close(connectors[i]);
    }

    zmq_ctx_destroy(context);
    printf("[EVENT SENDER] ZeroMQ sender thread terminated cleanly.\n");
    return NULL;
}



int start_zmq_sender(event_queue_t *queue, void *context)
{
    pthread_t thread;

    communication_args_t *args = (communication_args_t*) malloc(sizeof(communication_args_t));
    args->queue = queue;
    args->context = context;

    if(pthread_create(&thread, NULL, zmq_sender_thread, args) != 0)
    {
        return -1;
    }

    pthread_detach(thread);
    return 0;
}