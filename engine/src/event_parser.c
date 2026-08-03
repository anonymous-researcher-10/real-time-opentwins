#include "event_parser.h"
#include "event_queue.h"
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


static inline uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

int deserialize_event(event_queue_t *queue, uint8_t *buffer, size_t size)
{
    // printf("[PARSER] Deserializing event...\n");


    // Esto es justo para el test, para medir el tiempo que tarda la deserialización. Lo eliminaremos después.
    unsigned int idx = atomic_fetch_add_explicit(&log_index, 1, memory_order_relaxed);
    uint64_t start_time = get_time_ns();


    cw_unpack_context ctx;
    cw_unpack_context_init(&ctx, buffer, size, 0);

    cw_unpack_next(&ctx);
    if (ctx.return_code != 0) {
        return -1; // Unpacking error
    }

    uint32_t array_size = ctx.item.as.array.size;

    if (array_size < 4){
        return -1; // Not enough items for header
    }

    event_header_t header;

    cw_unpack_next(&ctx);
    header.type = (event_type_t)ctx.item.as.u64;
    cw_unpack_next(&ctx);
    header.timestamp = (uint64_t)ctx.item.as.u64;
    cw_unpack_next(&ctx);
    header.deviation = (float)ctx.item.as.u64;
    cw_unpack_next(&ctx);
    header.twin_id = (uint32_t)ctx.item.as.u64;


    if (header.type == EVENT_UPDATE){
        // printf("[PARSER] Event type: UPDATE\n");
        
        cw_unpack_next(&ctx);
        uint32_t var_count = (uint32_t)ctx.item.as.u64;

        if (array_size != 5 + var_count * 2){
            return -1; // Mismatch in expected size
        }


        uint32_t vars_processed = 0;

        while(vars_processed < var_count){

            event_t event;
            event.header = header;

            uint8_t batch_count = 0;

            while(batch_count < MAX_VARS_PER_EVENT && vars_processed < var_count){
                cw_unpack_next(&ctx);
                event.payload.update.updates[batch_count].var_id = (uint16_t)ctx.item.as.u64;
                cw_unpack_next(&ctx);

                if (ctx.item.type == CWP_ITEM_FLOAT || ctx.item.type == CWP_ITEM_DOUBLE) {
                    event.payload.update.updates[batch_count].value.f32 = (float)ctx.item.as.real;
                    if (idx < MAX_HISTORY_RECORDS) { // Solo para pruebas, lo eliminaremos después
                        history_log[idx].timestamp_ns = start_time;
                        history_log[idx].twin_id = header.twin_id;
                        history_log[idx].value = (float)ctx.item.as.real;

                        // printf("[PARSER] Logged event in history_log: msg=%u timestamp_ns=%lu, twin_id=%u, value=%f\n",
                        //        idx,
                        //        history_log[idx].timestamp_ns,
                        //        history_log[idx].twin_id,
                        //        history_log[idx].value);
                    }
                } else {
                    event.payload.update.updates[batch_count].value.i32 = (int32_t)ctx.item.as.i64;
                }

                batch_count++;
                vars_processed++;
            }
            event.payload.update.count = batch_count;

            if (push_event(queue, event) != 0){
                printf("[PARSER] Event queue is full, dropping event\n");
            }
        }

        
    }else if (header.type == EVENT_ERROR){
        printf("[PARSER] Event type: ERROR\n");
        event_t event;
        event.header = header;

        cw_unpack_next(&ctx);
        event.payload.error.error_code = (error_type_t)ctx.item.as.u64;
        cw_unpack_next(&ctx);
        if (ctx.item.type == CWP_ITEM_STR) {
            uint32_t len = ctx.item.as.str.length;
            if (len >= sizeof(event.payload.error.message)) {
                len = sizeof(event.payload.error.message) - 1;
            }
            memcpy(event.payload.error.message, ctx.item.as.str.start, len);
            event.payload.error.message[len] = '\0';
        } else {
            event.payload.error.message[0] = '\0';
        }

        push_event(queue, event);
    }else{
        printf("[PARSER] Unknown event type: %d\n", header.type);
        return -1; // Unknown event type
    }


    return 0;
}