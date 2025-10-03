#!/usr/bin/env bash
set -euo pipefail

# Compila bmssp.cpp (si existe) o main.cpp y coloca el binario en ./bin
CD_DIR="$(dirname "$0")"
cd "$CD_DIR"
mkdir -p bin

SRC=""
if [ -f bmssp.cpp ]; then
	SRC="bmssp.cpp"
elif [ -f main.cpp ]; then
	SRC="main.cpp"
else
	# try parent directory (project root)
	if [ -f "../bmssp.cpp" ]; then
		SRC="../bmssp.cpp"
	elif [ -f "../main.cpp" ]; then
		SRC="../main.cpp"
	else
		echo "Error: no source file (bmssp.cpp or main.cpp) found in ${PWD} or parent directory" >&2
		exit 1
	fi
fi

g++ -std=c++17 -O2 -Wall -Wextra -pedantic "$SRC" -o bin/bmssp

echo "Built: $(pwd)/bin/bmssp (from $SRC)"