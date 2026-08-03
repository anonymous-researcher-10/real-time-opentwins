#!/usr/bin/env bash

mkdir -p build


echo "Compilando el motor..."
gcc -I include tests/memory_test.c src/twins_memory.c -o build/memory_test -lpthread
echo "¡Compilación terminada!"