#ifndef TWINS_MEMORY_H
#define TWINS_MEMORY_H


#include "config.h"
#include <stdint.h>
#include <stdatomic.h>

typedef struct {
    _Atomic float vars[MAX_VARS_PER_TWINS];
    _Atomic uint32_t var_num;
    _Atomic uint32_t status;           // 0: OK, 1: ALARMA, 2: DESCONECTADO
    _Atomic uint64_t last_update_ms;   // Timestamp de la última vez que "habló"
} twin_state_t;

// API de la Memoria
void twins_memory_init(void);

// Escritura rápida (usada por el Dispatcher)
void twins_memory_update_var(uint32_t twin_id, uint8_t var_id, float value);

// Lectura rápida (usada por la API ZMQ/REST)
float twins_memory_get_var(uint32_t twin_id, uint8_t var_id);

int twins_memory_set_status(uint32_t target_id, uint32_t status);

int twins_memory_get_status(uint32_t target_id);


// int twin_memory_num_vars(uint32_t target_id);
#endif