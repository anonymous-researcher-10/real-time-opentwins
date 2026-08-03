#!/usr/bin/env bash

mkdir -p build

echo "Compilando el motor..."
gcc -I include -I include/external tests/full_test_v1.c src/twins_memory.c src/event_queue.c src/event_dispatcher.c src/event_processor.c src/event_reciever.c src/event_sender.c src/event_parser.c src/rule_engine.c src/core_binding.c include/external/cwpack.c -o build/full_test_v1 -lzmq -lpthread -lrt
echo "¡Compilación terminada!"

# g++ src/**/*.cpp -I include -o build/main