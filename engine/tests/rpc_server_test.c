#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "twins_memory.h"
#include "api_server.h"

int main() {
    printf("========================================\n");
    printf("🧪 ARRANCANDO SERVIDOR DE TEST RPC\n");
    printf("========================================\n");

    // 1. Inicializamos la memoria atómica
    twins_memory_init();

    // 2. Inyectamos un dato de prueba: Gemelo 42, Variable 0 = 85.5
    twins_memory_update_var(6, 0, 85.5f);
    printf("[MAIN] Dato inyectado: Gemelo 6, Var 0 = 85.5\n");

    // 3. Arrancamos el servidor API en el hilo principal (bloqueará aquí)
    api_server_thread(NULL);

    return 0;
}