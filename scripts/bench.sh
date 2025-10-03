#!/usr/bin/env bash
set -euo pipefail

# Ejecuta el binario varias veces midiendo tiempos (útil para micro-bench)
cd "$(dirname "$0")"

if [ ! -x bin/bmssp ]; then
  echo "Binary not found, building release..."
  ./build.sh
fi

REPS=${1:-5}

echo "Benchmarking bin/bmssp for $REPS runs"
for i in $(seq 1 $REPS); do
  echo "--- Run $i ---"
  /usr/bin/time -f "Run $i: real %e user %U sys %S" ./bin/bmssp
done

echo "Benchmark completed"