#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE // Obligatorio antes de incluir sched.h o pthread.h
#include <sched.h>
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "event_types.h"
#include <stdint.h>
#include <time.h>
#include "cwpack.h"

#define EVENT_UPDATE 0
#define EVENT_ERROR  1

#define ERROR_SENSOR_FAILURE 1

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
    pin_thread_to_core(1, 95);

    int pruebas[] = {10, 50, 100, 200, 500, 1000, 5000};
    int num_pruebas = sizeof(pruebas) / sizeof(pruebas[0]);

    int twin_id = 1;

    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    if (strcmp(hostname, "rbpi5-2") == 0) {
        // Soy la RBPI5
        printf("[SISTEMA] Soy la rb 2");
        twin_id = 10;

    } else if (strcmp(hostname, "rbpi5-3") == 0) {
        // Soy la RBPI6
        printf("[SISTEMA] Soy la rb 3");
        twin_id = 0;

    } else if (strcmp(hostname, "rbpi5-4") == 0) {
        // Soy la RBPI7
        printf("[SISTEMA] Soy la rb 4");
        twin_id = 1;

    } else if (strcmp(hostname, "rbpi5-5") == 0) {
        // Soy la RBPI6
        printf("[SISTEMA] Soy la rb 5");
        twin_id = 2;

    } else if (strcmp(hostname, "rbpi5-6") == 0) {
        // Soy la RBPI6
        printf("[SISTEMA] Soy la rb 6");
        twin_id = 3;
        
    } else if (strcmp(hostname, "rbpi5-7") == 0) {
        // Soy la RBPI6
        printf("[SISTEMA] Soy la rb 7");
        twin_id = 4;
    }

    
    int total_vars = 1;
    float deviation = 0.5f;

    // ==========================================
    // ZeroMQ
    // ==========================================

    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PUSH);

    int hwm = 10;
    // zmq_setsockopt(socket, ZMQ_RCVHWM, &hwm, sizeof(hwm));

    if (zmq_connect(socket, "tcp://192.168.5.192:5555") != 0) {
        perror("zmq_connect");
        return 1;
    }

    printf("[C] Conectado a ZeroMQ en el puerto 5555\n");

    sleep(1);

    // ==========================================
    // Envío de mensajes
    // ==========================================

    float base_value = 0.0f;

    for (int p = 0; p < num_pruebas; p++) {

        int total_msg = pruebas[p];

        printf("\n[C] Enviando %d mensajes...\n", total_msg);

        for (int m = 0; m < total_msg; m++) {
            size_t size;
            uint8_t buffer[ZMQ_BUFFER_SIZE];
            uint64_t timestamp = current_time_ms();

            // Inicializar packer MessagePack
            cw_pack_context pc;
            cw_pack_context_init(&pc, buffer, ZMQ_BUFFER_SIZE, 0);

            cw_pack_array_size(&pc, 5 + 1 * 2);
            cw_pack_unsigned(&pc, 0);
            cw_pack_unsigned(&pc, timestamp);
            cw_pack_unsigned(&pc, (uint64_t)deviation);
            cw_pack_unsigned(&pc, twin_id);
            cw_pack_unsigned(&pc, total_vars);
            cw_pack_unsigned(&pc, 0);
            cw_pack_float(&pc, base_value);

            base_value += 1.0f; // Incrementar el valor para simular cambios
            // =====================================================
            // Estructura:
            // [EVENT_UPDATE, timestamp, deviation,
            //  twin_id, total_vars, var_id, value...]
            // =====================================================
            
            // printf("Sending message: [EVENT_UPDATE, %u, %u, %.2f]\n", twin_id, total_vars, base_value);


            size = pc.current - pc.start;

            // ==========================================
            // Enviar por ZeroMQ
            // ==========================================


            int rc = zmq_send(socket, buffer, size, 0);

            // if (rc == -1) {
            //     perror("zmq_send");
            // } else {
            //     printf("[C] UPDATE enviado (%ld bytes)\n", size);
            // }

            // Liberar buffer MessagePack

            usleep(100000); // 0.5 s
        }

        sleep(30);
    }

    // ==========================================
    // ERROR EVENT (equivalente al comentado)
    // ==========================================

    /*
    {
        uint64_t timestamp = current_time_ms();

        msgpack_sbuffer sbuf;
        msgpack_sbuffer_init(&sbuf);

        msgpack_packer pk;
        msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

        // [TYPE, timestamp, deviation,
        //  twin_id, error_code, message]

        msgpack_pack_array(&pk, 6);

        msgpack_pack_int(&pk, EVENT_ERROR);
        msgpack_pack_uint64(&pk, timestamp);
        msgpack_pack_float(&pk, deviation);
        msgpack_pack_int(&pk, twin_id);
        msgpack_pack_int(&pk, ERROR_SENSOR_FAILURE);

        const char *msg = "Fallo critico de presion";

        msgpack_pack_str(&pk, strlen(msg));
        msgpack_pack_str_body(&pk, msg, strlen(msg));

        zmq_send(socket, sbuf.data, sbuf.size, 0);

        printf("[C] ERROR enviado (%ld bytes)\n", sbuf.size);

        msgpack_sbuffer_destroy(&sbuf);
    }
    */

    // ==========================================
    // Limpieza
    // ==========================================

    zmq_close(socket);
    zmq_ctx_destroy(context);

    printf("\n[C] Transmisión finalizada.\n");

    return 0;
}