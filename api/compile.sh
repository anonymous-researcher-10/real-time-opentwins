#!/usr/bin/env bash

mkdir -p build

echo "Compilando la API..."
g++ $(find src -name "*.cpp") -I include -o build/main -pthread
echo "¡Compilación terminada!"

# g++ src/**/*.cpp -I include -o build/main