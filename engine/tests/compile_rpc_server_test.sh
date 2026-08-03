#!/usr/bin/env bash

mkdir -p build

echo "Compilando el motor..."
gcc -I include -I include/external tests/rpc_server_test.c src/api_server.c src/twins_memory.c include/external/cwpack.c -o build/rpc_server_test -lzmq -lpthread

echo "¡Compilación terminada!"

# g++ src/**/*.cpp -I include -o build/main