#include "twins_memory.h"
#include <time.h>

static twin_state_t twins_memory[MAX_TWINS];

void twins_memory_init(void)
{
    for (uint32_t i = 0; i < MAX_TWINS; i++) {
        for (uint8_t j = 0; j < MAX_VARS_PER_TWINS; j++) {
            atomic_store(&twins_memory[i].vars[j], 0.0f);
        }
        atomic_store(&twins_memory[i].status, 2); // DESCONECTADO por defecto
        atomic_store(&twins_memory[i].last_update_ms, 0);
    }
}


// Escritura rápida (usada por el Dispatcher)
void twins_memory_update_var(uint32_t twin_id, uint8_t var_id, float value)
{
    if (twin_id >= MAX_TWINS || var_id >= MAX_VARS_PER_TWINS){
        return;
    }else{
        atomic_store(&twins_memory[twin_id].vars[var_id], value);
        atomic_store(&twins_memory[twin_id].status, 0); // OK
        atomic_store(&twins_memory[twin_id].last_update_ms, (uint64_t)time(NULL) * 1000);
    }
}


// Lectura rápida (usada por la API ZMQ/REST)
float twins_memory_get_var(uint32_t twin_id, uint8_t var_id)
{
    if (twin_id >= MAX_TWINS || var_id >= MAX_VARS_PER_TWINS ){
        return -1.0f;
    }else{
        return atomic_load(&twins_memory[twin_id].vars[var_id]);
    }
}

int twins_memory_set_status(uint32_t target_id, uint32_t status)
{
    if (target_id >= MAX_TWINS){
        return -1;
    }else{
        atomic_store(&twins_memory[target_id].status, status);
        return 0;
    }
}


int twins_memory_get_status(uint32_t target_id)
{
    if (target_id >= MAX_TWINS){
        return -1;
    }else{
        return atomic_load(&twins_memory[target_id].status);
    }
}