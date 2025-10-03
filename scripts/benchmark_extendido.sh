#!/usr/bin/env bash
set -euo pipefail

# Script para ejecutar benchmarks extendidos con diferentes tamaños

echo "╔════════════════════════════════════════════════════════════════════════════╗"
echo "║                    BENCHMARK EXTENDIDO - BMSSP                             ║"
echo "╚════════════════════════════════════════════════════════════════════════════╝"
echo ""

# Compilar si es necesario
if [ ! -f "bin/bmssp" ] || [ "bmssp.cpp" -nt "bin/bmssp" ]; then
    echo "Compilando..."
    mkdir -p bin
    cd "$(dirname "$0")"
    g++ -std=c++17 -O2 -Wall -Wextra -pedantic ../bmssp.cpp -o bin/bmssp
    cd ..
    echo "✓ Compilación completa"
    echo ""
fi

# Ejecutar benchmark
echo "Ejecutando benchmarks comparativos..."
echo ""
./scripts/bin/bmssp --benchmark

echo ""
echo "═══════════════════════════════════════════════════════════════════════════"
echo "Benchmark completado. Para ver más detalles, revisa README_BENCHMARK.md"
echo "═══════════════════════════════════════════════════════════════════════════"
