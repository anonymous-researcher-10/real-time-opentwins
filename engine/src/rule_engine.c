#include "rule_engine.h"
#include <stdio.h>

static rule_action_t error_rules[ERROR_MAX];
static rule_action_t update_rules[MAX_TWINS][MAX_VARS_PER_TWINS];

void rule_engine_init(void)
{
    error_rules[ERROR_DEVIATION] = ACTION_SET_TWIN_ALARM;
    error_rules[ERROR_SENSOR_FAILURE] = ACTION_HALT_ENGINE;
    error_rules[ERROR_COMMUNICATION] = ACTION_NOTIFY_UPWARDS;
    error_rules[ERROR_UNKNOWN] = ACTION_IGNORE;


    // Initialize update rules

    for (int t = 0; t < MAX_TWINS; t++) {
        for (int v = 0; v < MAX_VARS_PER_TWINS; v++) {
            update_rules[t][v] = ACTION_IGNORE;
        }
    } 



}

void set_error_rule(error_type_t error_code, rule_action_t action) 
{
    if (error_code < ERROR_MAX) {
        error_rules[error_code] = action;
    }
}


// Guarda directamente la acción a ejecutar cuando llegue este update
void set_update_rule(uint32_t twin_id, uint16_t var_id, rule_action_t action) 
{
    if (twin_id < MAX_TWINS && var_id < MAX_VARS_PER_TWINS) {
        printf("Regla actualizada: Twin %u, Var %u -> Acción %d\n", twin_id, var_id, action);
        update_rules[twin_id][var_id] = action;
    }
}


rule_action_t rule_engine_get_action_for_error(error_type_t error_code)
{
    if (error_code < ERROR_MAX) {
        return error_rules[error_code]; // Default action for unknown error codes
    }
    return ACTION_IGNORE;
}

rule_action_t get_update_rule(uint32_t twin_id, uint16_t var_id) 
{
    if (twin_id < MAX_TWINS && var_id < MAX_VARS_PER_TWINS) {
        return update_rules[twin_id][var_id];
    }
    return ACTION_IGNORE; 
}