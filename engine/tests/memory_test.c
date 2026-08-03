#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "twins_memory.h"

// --- HILO 1: Simula el Motor RT (Escritor) ---
void* motor_writer_thread(void* arg) {
    printf("[MOTOR] Hilo de escritura arrancado.\n");
    
    for (int i = 1; i <= 5; i++) {
        float nueva_temperatura = 20.0f + (i * 2.5f); // Sube la temperatura
        
        // Escritura atómica (O(1))
        twins_memory_update_var(4, 0, nueva_temperatura);
        printf("[MOTOR] ✍️ Gemelo 4, Var 0 actualizada a: %.2f\n", nueva_temperatura);
        
        usleep(400000); // Pausa de 400ms
    }
    return NULL;
}

// --- HILO 2: Simula el Servidor API (Lector) ---
void* api_reader_thread(void* arg) {
    printf("[API] Hilo de lectura arrancado.\n");
    
    for (int i = 0; i < 10; i++) {
        // Lectura atómica (O(1))
        float temp_leida = twins_memory_get_var(4, 0);
        printf("   -> [API] 👁️ Consulta externa leída: %.2f\n", temp_leida);
        
        usleep(200000); // Pausa de 200ms (Lee el doble de rápido de lo que se escribe)
    }
    return NULL;
}

int main() {
    printf("Arrancando Test de Concurrencia de Memoria Atómica...\n\n");

    // 1. Inicializamos la base de datos en RAM
    twins_memory_init();

    // Comprobamos el estado inicial
    float initial_val = twins_memory_get_var(4, 0);
    printf("[MAIN] Valor inicial del Gemelo 4, Var 0: %.2f\n\n", initial_val);

    // 2. Creamos los hilos
    pthread_t writer, reader;

    // Arrancamos a la vez el que escribe y el que lee
    pthread_create(&writer, NULL, motor_writer_thread, NULL);
    pthread_create(&reader, NULL, api_reader_thread, NULL);

    // 3. Esperamos a que terminen
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    printf("\n[MAIN] Test finalizado. Cero bloqueos detectados.\n");
    return 0;
}