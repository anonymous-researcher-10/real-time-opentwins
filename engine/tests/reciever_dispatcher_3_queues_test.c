#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "event_queue.h"
#include "event_dispatcher.h"
#include "event_processor.h"
#include "rule_engine.h"
#include "event_reciever.h" 

// 1. Declaramos las 3 colas globales para este test
event_queue_t queue_in;
event_queue_t queue_out_notify;
event_queue_t queue_out_user;
event_processor_t processor;
// --- HILO FALSO DE SALIDA 1: Nube / UI ---
void *mock_notify_sender(void *arg) {
    printf("\n[HILO NUBE] ☁️  Simulando servicio en la nube esperando datos...\n");
    event_t ev;
    while(1) {
        if (pop_event(&queue_out_notify, &ev) == 0) {
            printf("\n[HILO NUBE] ☁️  Recibido lote para notificar. Tipo: %d\n", ev.header.type);
            if(ev.header.type == 0) {
                printf("   -> Variables en el lote: %d\n", ev.payload.update.count);
                for(int i=0; i<ev.payload.update.count; i++) {
                    printf("      - Var ID: %u\n", ev.payload.update.updates[i].var_id);
                }
            }
        }
        usleep(100);
    }
    return NULL;
}

// --- HILO FALSO DE SALIDA 2: Servicio del Usuario ---
void *mock_user_sender(void *arg) {
    printf("\n[HILO USUARIO] 🐍 Simulando script local esperando datos...\n");
    event_t ev;
    while(1) {
        if (pop_event(&queue_out_user, &ev) == 0) {
            printf("\n[HILO USUARIO] 🐍 Avisando al script Python local. Tipo: %d\n", ev.header.type);
            if(ev.header.type == 0) {
                printf("   -> Variables en el lote: %d\n", ev.payload.update.count);
                for(int i=0; i<ev.payload.update.count; i++) {
                    printf("      - Var ID: %u\n", ev.payload.update.updates[i].var_id);
                }
            }
        }
        usleep(100);
    }
    return NULL;
}


int main() {

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
    set_update_rule(1, 0, ACTION_NOTIFY_UPWARDS);     // Var 0 va a la nube
    set_update_rule(1, 1, ACTION_CALL_USER_SERVICE);  // Var 1 va al script del usuario
    set_update_rule(1, 2, ACTION_NOTIFY_UPWARDS);     // Var 2 va a la nube
    set_update_rule(1, 3, ACTION_CALL_USER_SERVICE);  // Var 3 va al script del usuario
    // Las variables 4, 5, 6, 7, 8 y 9 se quedarán por defecto en ACTION_IGNORE (solo RAM)

    // 3. Enchufar las tuberías (Inyectar dependencias)
    event_dispatcher_init(&queue_out_notify, &queue_out_user);
    event_processor_init(&processor, &queue_in);

    // 4. Arrancar los hilos de salida (Mocks por ahora)
    pthread_t tid_notify, tid_user;
    printf("Arrancando hilos de salida simulados (nube y usuario)...\n");
    pthread_create(&tid_notify, NULL, mock_notify_sender, NULL);
    pthread_create(&tid_user, NULL, mock_user_sender, NULL);

    // 5. Arrancar el hilo de recepción de ZeroMQ
    printf("Arrancando el receptor ZeroMQ para recibir eventos...\n");
    start_zmq_receiver(&queue_in);
    // 6. Arrancar el procesador principal (Bloquea el main infinitamente)
    // A partir de aquí, el hilo principal de C se dedica EXCLUSIVAMENTE a procesar datos.
    printf("Arrancando el bucle principal del procesador de eventos...\n");
    event_processor_loop(&processor);

    return 0;
}