#include "event_types.h"

#define ERROR_MAX (ERROR_UNKNOWN + 1)

static rule_action_t error_rules[ERROR_MAX] = {
    [ERROR_DEVIATION] = ACTION_SET_TWIN_ALARM,
    [ERROR_SENSOR_FAILURE] = ACTION_HALT_ENGINE,
    [ERROR_COMMUNICATION] = ACTION_NOTIFY_UPWARDS,
    [ERROR_UNKNOWN] = ACTION_IGNORE
};

void rule_engine_init(void);

void set_error_rule(error_type_t error_code, rule_action_t action);
void set_update_rule(uint32_t twin_id, uint16_t var_id, rule_action_t action);

rule_action_t rule_engine_get_action_for_error(error_type_t error_code);
rule_action_t get_update_rule(uint32_t twin_id, uint16_t var_id);

