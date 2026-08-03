#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <zmq.h>
#include "event_queue.h"
#include "event_dispatcher.h"
#include "event_processor.h"
#include "rule_engine.h"
#include "event_reciever.h" 
#include "event_sender.h"
#include <signal.h> // Para manejar Ctrl+C
#include <stdatomic.h>
#include <stdint.h>
#include <signal.h> // Para manejar Ctrl+C

event_queue_t queue_in;
event_queue_t queue_out_notify;
event_queue_t queue_out_user;
event_processor_t processor;

TimeSeriesRecord history_log[MAX_HISTORY_RECORDS];
atomic_uint log_index = ATOMIC_VAR_INIT(0);
TimeSeriesRecord history_log2[MAX_HISTORY_RECORDS];
atomic_uint log_index2 = ATOMIC_VAR_INIT(0);
volatile sig_atomic_t keep_running = 1;


void handle_sigint(int sig) {
    printf("\n[SISTEMA] Señal de apagado recibida. Deteniendo el motor...\n");
    keep_running = 0; // Rompe los bucles while(1)
}

int main() {
    signal(SIGINT, handle_sigint);

    printf("Arrancando Motor Gemelo Digital (Tiempo Real)...\n\n");

    // 1. Inicializar las colas en memoria
    init_queue(&queue_in);
    init_queue(&queue_out_notify);
    init_queue(&queue_out_user);

    // 2. Inicializar Reglas y simular que un usuario las configura
    rule_engine_init();
    
    // Configuración de prueba para el Gemelo 1:
    // (Recuerda que en Python mandamos 10 variables, de la 0 a la 9)
    printf("Configurando reglas de enrutamiento...\n");
    set_update_rule(0, 0, ACTION_NOTIFY_UPWARDS);     // Var 0 de twin 0 va a RBPI3
    set_update_rule(1, 0, ACTION_NOTIFY_UPWARDS);     // Var 0 de twin 1 va a RBPI4
    set_update_rule(2, 0, ACTION_NOTIFY_UPWARDS);     // Var 0 de twin 2 va a RBPI5
    set_update_rule(3, 0, ACTION_NOTIFY_UPWARDS);     // Var 0 de twin 3 va a RBPI6
    set_update_rule(4, 0, ACTION_NOTIFY_UPWARDS);     // Var 0 de twin 4 va a RBPI7

    // 3. Inicializar la memoria compartida de los gemelos digitales
    twins_memory_init();

    // 4. Enchufar las tuberías (Inyectar dependencias)
    event_dispatcher_init(&queue_out_notify, &queue_out_user);
    event_processor_init(&processor, &queue_in);

    // 5. Arrancar los hilos de salida (Mocks por ahora
    printf("Arrancando hilos de salida (nube)...\n");
    void *zmq_context = zmq_ctx_new();
    start_zmq_sender(&queue_out_notify, zmq_context);


    // 6. Arrancar el hilo de recepción de ZeroMQ
    printf("Arrancando el receptor ZeroMQ para recibir eventos...\n");
    start_zmq_receiver(&queue_in, zmq_context);
    // 7. Arrancar el procesador principal (Bloquea el main infinitamente)
    // A partir de aquí, el hilo principal de C se dedica EXCLUSIVAMENTE a procesar datos.
    printf("Arrancando el bucle principal del procesador de eventos...\n");
    start_processor_loop(&processor);

    while(keep_running) {
        sleep(1); // El hilo principal solo duerme, el trabajo real lo hacen los hilos de recepción y procesamiento
    }


    printf("[SISTEMA] Motor detenido. Realizando limpieza y volcando resultados...\n");


    unsigned int final_count = atomic_load_explicit(&log_index, memory_order_relaxed);
    if (final_count > MAX_HISTORY_RECORDS) {
        final_count = MAX_HISTORY_RECORDS;
    }

    printf("[SISTEMA] Pruebas finalizadas. Volcando %u registros a CSV...\n", final_count);
    
    FILE *f = fopen("resultados_experimento.csv", "w");
    if (f != NULL) {
        // Escribir cabecera del CSV
        fprintf(f, "timestamp_ns,value,twin_id\n");
        
        // Escribir los datos desde la RAM al disco duro
        for (unsigned int i = 0; i < final_count; i++) {
            fprintf(f, "%lu,%f,%u\n", 
                    history_log[i].timestamp_ns,
                    history_log[i].value,
                    history_log[i].twin_id);
        }
        fclose(f);
        printf("[SISTEMA] CSV guardado exitosamente como 'resultados_experimento.csv'.\n");
    } else {
        printf("[ERROR] No se pudo crear el archivo CSV.\n");
    }

    unsigned int final_count2 = atomic_load_explicit(&log_index2, memory_order_relaxed);
    if (final_count2 > MAX_HISTORY_RECORDS) {
        final_count2 = MAX_HISTORY_RECORDS;
    }

    printf("[SISTEMA] Pruebas finalizadas. Volcando %u registros a CSV...\n", final_count2);
    
    FILE *f2 = fopen("resultados_experimento2.csv", "w");
    if (f2 != NULL) {
        // Escribir cabecera del CSV
        fprintf(f2, "timestamp_ns,value,twin_id\n");
        
        // Escribir los datos desde la RAM al disco duro
        for (unsigned int i = 0; i < final_count2; i++) {
            fprintf(f2, "%lu,%f,%u\n", 
                    history_log2[i].timestamp_ns,
                    history_log2[i].value,
                    history_log2[i].twin_id);
        }
        fclose(f2);
        printf("[SISTEMA] CSV guardado exitosamente como 'resultados_experimento2.csv'.\n");
    } else {
        printf("[ERROR] No se pudo crear el archivo CSV.\n");
    }

    return 0;
}