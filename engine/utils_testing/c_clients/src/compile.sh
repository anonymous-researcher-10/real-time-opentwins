#!/usr/bin/env bash

mkdir -p build

echo "Compilando el motor..."
gcc -g -O0 -I include -I include/external src/zmq_recieve.c include/external/cwpack.c -o build/zmq_recieve -lzmq -lpthread -lrt

gcc -I include -I include/external src/zmq_send.c include/external/cwpack.c -o build/zmq_send -lzmq -lpthread -lrt
echo "¡Compilación terminada!"

# g++ src/**/*.cpp -I include -o build/main