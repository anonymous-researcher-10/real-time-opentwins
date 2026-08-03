#!/usr/bin/env bash

mkdir -p build

echo "Compilando el motor..."
gcc -I include -o build/processor_test tests/processor_test.c src/event_queue.c src/event_dispatcher.c src/event_processor.c
echo "¡Compilación terminada!"

# g++ src/**/*.cpp -I include -o build/main