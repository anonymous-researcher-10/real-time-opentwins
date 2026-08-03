#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H
#include "config.h"
#include <stdint.h>  // para uint8_t, uint16_t, uint32_t, uint64_t
#include <stddef.h>  // opcional, si usas size_t
#include <string.h>  // opcional, para strncpy
#include <stdatomic.h> // para atomic_uint
#include <signal.h>

typedef enum {
    ACTION_IGNORE,          // Ignore event
    ACTION_NOTIFY_UPWARDS,  // Notify parent engine
    ACTION_HALT_ENGINE,     // Stop the engine
    ACTION_SET_TWIN_ALARM,   // Set an alarm on the digital twin
    ACTION_CALL_USER_SERVICE
} rule_action_t;

typedef enum {
    EVENT_UPDATE,
    EVENT_ERROR,
    EVENT_MAX
} event_type_t;

typedef enum {
    ERROR_DEVIATION,
    ERROR_SENSOR_FAILURE,
    ERROR_COMMUNICATION,
    ERROR_UNKNOWN
} error_type_t;


typedef union {
    float    f32;
    int32_t  i32;
} var_value_t;

typedef struct {
    uint16_t    var_id;
    var_value_t value;
} property_update_t;


typedef struct {
    uint8_t           count; 
    property_update_t updates[MAX_VARS_PER_EVENT];
} update_event_t;


typedef struct {
    error_type_t error_code;
    char message[64];
} error_event_t;


typedef struct {
    event_type_t type;     
    uint64_t     timestamp;   
    float     deviation; 
    uint32_t     twin_id;      
} event_header_t;

typedef struct {
    event_header_t header;
    union {
        update_event_t update;
        error_event_t error;
    } payload;
} event_t;


// ESTO ES PARA PRUEBAS, LO BORRAREMOS DESPUÉS
// 1. Estructura del registro
typedef struct {
    uint64_t timestamp_ns;
    float value;
    uint32_t twin_id;
} TimeSeriesRecord;
#define MAX_HISTORY_RECORDS 5000000
extern TimeSeriesRecord history_log[MAX_HISTORY_RECORDS];
extern atomic_uint log_index;

extern TimeSeriesRecord history_log2[MAX_HISTORY_RECORDS];
extern atomic_uint log_index2;

extern volatile sig_atomic_t keep_running;



#endif // EVENT_TYPES_H