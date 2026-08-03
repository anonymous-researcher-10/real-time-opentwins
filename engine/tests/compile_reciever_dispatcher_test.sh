#!/usr/bin/env bash

mkdir -p build

echo "Compilando el motor..."
gcc -I include -I include/external tests/reciever_dispatcher_test.c src/event_queue.c src/event_dispatcher.c src/event_processor.c src/event_reciever.c src/event_parser.c include/external/cwpack.c -o build/reciever_dispatcher -lzmq -lpthread
echo "¡Compilación terminada!"

# g++ src/**/*.cpp -I include -o build/main