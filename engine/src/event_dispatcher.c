#include "event_dispatcher.h"
#include "rule_engine.h"
#include "twins_memory.h"
#include <stdio.h>
#include <unistd.h>

static void handle_event_update(event_t *event);
static void handle_event_error(event_t *event);
static void handle_event_unknown(event_t *event);

static event_queue_t *notify_queue;
static event_queue_t *user_actions_queue;

typedef void (*event_handler_t)(event_t *event);

static event_handler_t handlers[EVENT_MAX] = {
    [EVENT_UPDATE] = (event_handler_t)handle_event_update,
    [EVENT_ERROR] = (event_handler_t)handle_event_error
};


void event_dispatcher_init(event_queue_t *queue_out_notify, event_queue_t *queue_out_user_actions)
{
    notify_queue = queue_out_notify;
    user_actions_queue = queue_out_user_actions;
}


void dispatch_event(event_t *event)
{
    if (event->header.type >= EVENT_MAX) {
        handle_event_unknown(event);
        return;
    }

    event_handler_t handler = handlers[event->header.type];

    if (handler) {
        handler(event);
    } else {
        handle_event_unknown(event);
    }
}



// HANDLER IMPLEMENTATIONS

static void handle_event_update(event_t *event)
{
    uint32_t twin_id = event->header.twin_id;
    uint8_t count = event->payload.update.count;

    // =====================================
    // ==========UPDATE=====================
    // =====================================
    for (int i=0; i<count; i++) {
        uint8_t var_id = event->payload.update.updates[i].var_id;
        float value = event->payload.update.updates[i].value.f32;

        // Update the variable in the twins memory
        twins_memory_update_var(twin_id, var_id, value);
    }

    // =====================================
    // ==========REGLAS=====================
    // =====================================

    event_t batch_notify = *event; // Create a copy of the event for notification batches
    batch_notify.payload.update.count = 0; // Reset count for batch
    event_t batch_user_action = *event; // Create a copy of the event for user action batches
    batch_user_action.payload.update.count = 0; // Reset count for batch


    for(int i=0; i<count; i++) {

        uint16_t var_id = event->payload.update.updates[i].var_id;

        rule_action_t action = get_update_rule(twin_id, var_id);

        switch(action){
            case ACTION_IGNORE:
                printf("Ignoring update for variable %u\n", var_id);
                break;
            case ACTION_NOTIFY_UPWARDS:
                // printf("[DISPATCHER] Notifying parent engine about update for variable %u\n", var_id);
                batch_notify.payload.update.updates[batch_notify.payload.update.count] = event->payload.update.updates[i];
                batch_notify.payload.update.count = batch_notify.payload.update.count+1;
                break;
            case ACTION_HALT_ENGINE:
                printf("[DISPATCHER] Halting engine");
                break;
            case ACTION_SET_TWIN_ALARM:
                printf("[DISPATCHER] Setting alarm on twin %u",
                    event->header.twin_id);
                break;
            case ACTION_CALL_USER_SERVICE:
                printf("[DISPATCHER] Calling user service for update for variable %u\n", var_id);
                batch_user_action.payload.update.updates[batch_user_action.payload.update.count] = event->payload.update.updates[i];
                batch_user_action.payload.update.count = batch_user_action.payload.update.count+1;
                break;
            default:
                printf("[DISPATCHER] Unknown action for update for variable %u\n", var_id);
        }
    }

    if(batch_notify.payload.update.count != 0){
        push_event(notify_queue, batch_notify);
    }
    if(batch_user_action.payload.update.count != 0){
        push_event(user_actions_queue, batch_user_action);
    }

}

static void handle_event_error(event_t *event)
{
    error_type_t code = event->payload.error.error_code;
    rule_action_t action = rule_engine_get_action_for_error(code);

    switch(action){
        case ACTION_IGNORE:
            printf("[DISPATCHER] Ignoring error code %u\n", code);
            break;
        case ACTION_NOTIFY_UPWARDS:
            printf("[DISPATCHER] Notifying parent engine about error code %u\n", code);
            // push_event(notify_queue, *event);
            break;
        case ACTION_HALT_ENGINE:
            printf("[DISPATCHER] Halting engine due to error code %u\n", code);
            break;
        case ACTION_SET_TWIN_ALARM:
            printf("[DISPATCHER] Setting alarm on twin %u due to error code %u\n",
                event->header.twin_id, code);
            break;
        default:
            printf("[DISPATCHER] Unknown action for error code %u\n", code);
    }
}

static void handle_event_unknown(event_t *event)
{
    printf("[DISPATCHER] Unknown event type: %d\n", event->header.twin_id);
}