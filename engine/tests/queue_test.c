#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>  // para size_t si lo usas
#include "event_queue.h"

int main() {
    event_queue_t queue;
    init_queue(&queue);

    printf("=== Test push/pop de event_queue ===\n");

    // 1️⃣ Crear evento de tipo update
    event_t evt_update;
    evt_update.header.type = EVENT_UPDATE;
    evt_update.header.twin_id = 42;
    evt_update.header.timestamp = 123456789;
    evt_update.header.deviation = 5.0f; // Ejemplo de desviación
    evt_update.payload.update.updates[0].var_id = 1;
    evt_update.payload.update.updates[0].value.f32 = 25.5f;
    evt_update.payload.update.updates[1].var_id = 2;
    evt_update.payload.update.updates[1].value.f32 = 101.3f;


    // 2️⃣ Push update
    if(push_event(&queue, evt_update) == 0) {
        printf("Push evento UPDATE OK\n");
    } else {
        printf("Push evento UPDATE FALLÓ\n");
    }

    // 3️⃣ Crear evento de tipo error
    event_t evt_error;
    evt_error.header.type = EVENT_ERROR;
    evt_error.header.twin_id = 42;
    evt_error.header.timestamp = 123456790;
    evt_update.header.deviation = 5.0f; // Ejemplo de desviación
    evt_error.payload.error.error_code = ERROR_SENSOR_FAILURE;
    evt_error.payload.error.message[0] = '\0';  // Initialize the message as an empty string
    strncpy(evt_error.payload.error.message, "Sensor desconectado", sizeof(evt_error.payload.error.message));

    // 4️⃣ Push error
    if(push_event(&queue, evt_error) == 0) {
        printf("Push evento ERROR OK\n");
    } else {
        printf("Push evento ERROR FALLÓ\n");
    }

    // 5️⃣ Pop y verificar update
    event_t popped;
    if(pop_event(&queue, &popped) == 0) {
        if(popped.header.type == EVENT_UPDATE) {
            printf("Pop UPDATE correcto: twin_id=%u, temp=%.1f, pres=%.1f, deviation=%f\n",
                popped.header.twin_id,
                popped.payload.update.updates[0].value.f32,
                popped.payload.update.updates[1].value.f32,
                popped.header.deviation);
        } else {
            printf("Pop ERROR inesperado\n");
        }
    } else {
        printf("Pop FALLÓ\n");
    }

    // 6️⃣ Pop y verificar error
    if(pop_event(&queue, &popped) == 0) {
        if(popped.header.type == EVENT_ERROR) {
            printf("Pop ERROR correcto: twin_id=%u, code=%d, msg=%s\n",
                popped.header.twin_id,
                popped.payload.error.error_code,
                popped.payload.error.message);
        } else {
            printf("Pop UPDATE inesperado\n");
        }
    } else {
        printf("Pop FALLÓ\n");
    }

    // 7️⃣ Intentar pop en cola vacía
    if(pop_event(&queue, &popped) != 0) {
        printf("Pop en cola vacía correctamente devuelto -1\n");
    }

    return 0;
}