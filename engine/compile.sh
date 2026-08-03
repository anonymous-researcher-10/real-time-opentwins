#!/usr/bin/env bash

mkdir -p build

echo "Compilando el motor..."
gcc src/main.c -lzmq -I include -o build/main
echo "¡Compilación terminada!"