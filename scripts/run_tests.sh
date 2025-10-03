#!/usr/bin/env bash
set -euo pipefail

# Ejecuta las pruebas integradas que vienen en bmssp.cpp
cd "$(dirname "$0")"

if [ ! -x bin/bmssp ]; then
  echo "Binary not found, building release..."
  ./build.sh
fi

echo "Running tests (bin/bmssp)"
./bin/bmssp
RC=$?
if [ $RC -eq 0 ]; then
  echo "All tests passed (exit code $RC)"
else
  echo "Tests failed (exit code $RC)"
fi
exit $RC