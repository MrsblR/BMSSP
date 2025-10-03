#!/usr/bin/env bash
set -euo pipefail

# Compila bmssp.cpp en modo debug con sanitizers (address + undefined)
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

g++ -std=c++17 -O0 -g -fsanitize=address,undefined -Wall -Wextra "$SRC" -o bin/bmssp_debug

echo "Built debug: $(pwd)/bin/bmssp_debug (from $SRC) (with ASAN/UBSAN)"