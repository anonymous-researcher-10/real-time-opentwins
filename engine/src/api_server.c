#include "api_server.h"
#include "cwpack.h"
#include "config.h"
#include "twins_memory.h"
#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event_types.h"

void send_error(void *responder, const char *message) {
    uint8_t buf[256];
    cw_pack_context pc;
    cw_pack_context_init(&pc, buf, sizeof(buf), 0);
    cw_pack_map_size(&pc, 2);
    cw_pack_str(&pc, "status", 6);
    cw_pack_str(&pc, "error", 5);
    cw_pack_str(&pc, "msg", 3);
    cw_pack_str(&pc, message, strlen(message));
    zmq_send(responder, buf, pc.current - pc.start, 0);

}

void *api_server_thread(void *arg) 
{
    // Implementation for starting the API server
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);

    int rc = zmq_bind(responder, "tcp://*:5556");

    if (rc != 0) {
        fprintf(stderr, "[API SERVER] Failed to bind API server: %s\n", zmq_strerror(errno));
        zmq_close(responder);
        zmq_ctx_destroy(context);
        return NULL;
    }

    printf("[API SERVER] API server started on tcp://*:5556\n");

    uint8_t req_buf[1024];
    uint8_t res_buf[4096];

    while(keep_running){
        int size = zmq_recv(responder, req_buf, sizeof(req_buf), 0);
        if(size == -1) continue;

        printf("[API SERVER] Deserializing query...\n");

        cw_unpack_context ctx;
        cw_unpack_context_init(&ctx, req_buf, size, 0);

        cw_unpack_next(&ctx);


        if (ctx.return_code != 0) {
            send_error(responder, "Failed to unpack request");
            continue;
        }

        uint32_t array_size = ctx.item.as.array.size;



        printf("\n[DEBUG] Recibidos %d bytes.\n", size);
        printf("[DEBUG] Primer byte en crudo: 0x%02X\n", req_buf[0]);
        printf("[DEBUG] Tipo detectado por CWPack: %d (8=Array, 9=Map, 6=Str, 2=Int)\n", ctx.item.type);


        if (ctx.item.type != CWP_ITEM_ARRAY || ctx.item.as.array.size == 0) {
            printf("[API SERVER] Invalid request format: Expected non-empty array\n");
            send_error(responder, "Array of arguments expected");
            continue;
        }
        
        array_size = ctx.item.as.array.size;

        cw_unpack_next(&ctx);
        uint8_t opcode = (uint8_t)ctx.item.as.u64;
        printf("[API SERVER] Received OpCode: %u\n", opcode);

        uint32_t target_id = 0;
        if (opcode == 1 || opcode == 2) {
            if (array_size < 2) {
                send_error(responder, "Not enough arguments for this opcode");
                continue;
            }
            cw_unpack_next(&ctx);
            target_id = (uint32_t)ctx.item.as.u64;
            printf("[API SERVER] Received query - OpCode: %u, Target ID: %u\n", opcode, target_id);
        }

        cw_pack_context pc;
        cw_pack_context_init(&pc, res_buf, sizeof(res_buf), 0);


        switch (opcode) {
            case 0: // GET_ALL
                cw_pack_array_size(&pc, MAX_TWINS); 
                for (uint32_t t = 0; t < MAX_TWINS; t++) {
                    cw_pack_map_size(&pc, 3);
                    cw_pack_str(&pc, "id", 2); 
                    cw_pack_unsigned(&pc, t);
                    cw_pack_str(&pc, "status", 6); 
                    cw_pack_unsigned(&pc, twins_memory_get_status(t));
                    
                    // Empaquetamos TODAS las variables de cada gemelo
                    cw_pack_str(&pc, "vars", 4); 
                    cw_pack_array_size(&pc, MAX_VARS_PER_TWINS);
                    for (uint8_t i = 0; i < MAX_VARS_PER_TWINS; i++) {
                        cw_pack_float(&pc, twins_memory_get_var(t, i));
                    }
                }
                break;

            case 1: // GET (ID Específico)
                cw_pack_map_size(&pc, 3);
                cw_pack_str(&pc, "id", 2); 
                cw_pack_unsigned(&pc, target_id);
                cw_pack_str(&pc, "status", 6); 
                cw_pack_unsigned(&pc, twins_memory_get_status(target_id));
                
                // Empaquetamos TODAS las variables de cada gemelo
                cw_pack_str(&pc, "vars", 4); 
                cw_pack_array_size(&pc, MAX_VARS_PER_TWINS);
                for (uint8_t i = 0; i < MAX_VARS_PER_TWINS; i++) {
                    cw_pack_float(&pc, twins_memory_get_var(target_id, i));
                }
                break;

            case 2: // DELETE
                twins_memory_set_status(target_id, 2); // 2 = DESCONECTADO
                
                cw_pack_map_size(&pc, 2);
                cw_pack_str(&pc, "status", 6); 
                cw_pack_str(&pc, "deleted", 7);
                cw_pack_str(&pc, "id", 2); 
                cw_pack_unsigned(&pc, target_id);
                break;
            default:
                send_error(responder, "Error: OpCode desconocido");
                continue;
        }

        zmq_send(responder, res_buf, pc.current - pc.start, 0);
    }

    printf("[API SERVER] API server shutting down...\n");

    return NULL;



}