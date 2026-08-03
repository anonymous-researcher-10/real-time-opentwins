#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE // Obligatorio antes de incluir sched.h o pthread.h
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <zmq.h>
#include "cwpack.h"
#include "config.h"

#define EVENT_UPDATE 0
#define EVENT_ERROR  1

#define MAX_MSGS 1000000

typedef struct {
    uint64_t sending_ts;
    uint64_t receiving_ts;
    uint32_t twin_id;
    float message_id;
} mensaje_t;

mensaje_t history_log[8000000];

static volatile bool running = true;

static void handle_sigint(int sig) {
    (void)sig;
    running = false;
}

static uint64_t current_time_ms() {
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return ((uint64_t)ts.tv_sec * 1000ULL) +
           ((uint64_t)ts.tv_nsec / 1000000ULL);
}

void pin_thread_to_core(int core_id, int rt_priority) {
    pthread_t thread = pthread_self();

    // 1. Asignar la Afinidad de CPU (Obligar al hilo a vivir en un núcleo concreto)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    int rc = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        printf("[ERROR] No se pudo anclar el hilo al Core %d\n", core_id);
    }

    // 2. Aplicar la política de Tiempo Real (SCHED_FIFO)
    struct sched_param param;
    param.sched_priority = rt_priority;

    rc = pthread_setschedparam(thread, SCHED_FIFO, &param);
    if (rc != 0) {
        printf("[ERROR] Fallo al asignar prioridad RT al Core %d. ¿Usaste sudo?\n", core_id);
    } else {
        printf("[SISTEMA] Hilo configurado exitosamente en Core %d con prioridad %d\n", core_id, rt_priority);
    }
}

int main() {
    pin_thread_to_core(2, 95);

    signal(SIGINT, handle_sigint);

    // ==========================================
    // ZeroMQ
    // ==========================================

    void *context = zmq_ctx_new();
    void *pull = zmq_socket(context, ZMQ_PULL);

    int hwm = 10;
    // zmq_setsockopt(pull, ZMQ_RCVHWM, &hwm, sizeof(hwm));

    char endpoint[64];
    snprintf(endpoint, sizeof(endpoint), "tcp://*:%s", "5556");
    zmq_bind(pull, endpoint);
    int timeout_ms = 500;
    zmq_setsockopt(pull, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));

    uint8_t buffer[ZMQ_BUFFER_SIZE];

    printf("[C] Conectado a tcp://192.168.5.192:5556\n");


    // ==========================================
    // Recepción
    // ==========================================
    int total_received = 0;
    while (running) {

        int size = zmq_recv(pull, buffer, sizeof(buffer), 0);
        if(size == -1) continue;

        
        // printf("[C] Mensaje recibido (%d bytes)\n", size);
        uint64_t receiving_ts = current_time_ms();
        

        total_received++;
        // ==========================================
        // Parsear MessagePack
        // ==========================================

        

        cw_unpack_context ctx;
        cw_unpack_context_init(&ctx, buffer, size, 0);

        cw_unpack_next(&ctx);
        if (ctx.return_code != 0) {
            return -1; // Unpacking error
        }

        uint32_t array_size = ctx.item.as.array.size;

        if (array_size < 4){
            return -1; // Not enough items for header
        }


        cw_unpack_next(&ctx); //Tipo de evento
        cw_unpack_next(&ctx); // Timestamp
        uint64_t sending_ts = (uint64_t)ctx.item.as.u64;
        cw_unpack_next(&ctx); // Deviation
        cw_unpack_next(&ctx); // ID twin
        uint32_t twin_id = (uint32_t)ctx.item.as.u64;
        cw_unpack_next(&ctx); // Var count
        cw_unpack_next(&ctx); // Var ID o error code
        uint32_t var_count = (uint32_t)ctx.item.as.u64;
        cw_unpack_next(&ctx); // Var ID o error code
        float value = (float)ctx.item.as.real;

        // printf("[C] Recibido: twin_id=%u, value=%f, sending_ts=%lu, receiving_ts=%lu\n",
        //        twin_id, value, sending_ts, receiving_ts);

        if (total_received <= MAX_MSGS) {
            history_log[total_received - 1].sending_ts = sending_ts;
            history_log[total_received - 1].receiving_ts = receiving_ts;
            history_log[total_received - 1].twin_id = twin_id;
            history_log[total_received - 1].message_id = value;
        }
    }

    // ==========================================
    // Guardar CSV
    // ==========================================

    FILE *fp = fopen("mensajes_recibidos2.csv", "w");

    if (!fp) {
        perror("fopen");
    } else {

        fprintf(fp,
                "Sending_Timestamp,Receiving_Timestamp,twin_id,message_id\n");

        for (int i = 0; i < total_received; i++) {

            fprintf(fp,
                    "%lu,%lu,%u,%f\n",
                    history_log[i].sending_ts,
                    history_log[i].receiving_ts,
                    history_log[i].twin_id,
                    history_log[i].message_id);
        }

        fclose(fp);

        printf("\n[C] CSV guardado con %u mensajes\n",
               total_received);
    }

    // ==========================================
    // Limpieza
    // ==========================================


    zmq_close(pull);
    zmq_ctx_destroy(context);

    printf("[C] Recepción finalizada\n");

    return 0;
}