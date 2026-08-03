#!/usr/bin/env bash

mkdir -p build

echo "Compilando el motor..."
gcc -I include -o build/queue_test tests/queue_test.c src/event_queue.c
echo "¡Compilación terminada!"

# g++ src/**/*.cpp -I include -o build/main