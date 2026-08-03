#define EVENT_PARSER_H
#define _POSIX_C_SOURCE 200809L
#include "cwpack.h"
#include "event_types.h"
#include "event_queue.h"
#include <stdint.h>
#include "config.h"
#include <stdatomic.h>
#include <stdint.h>




// Event deserializer using MessagePack to event_t
int deserialize_event(event_queue_t *queue, uint8_t *buffer, size_t size);