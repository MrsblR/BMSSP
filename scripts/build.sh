#!/usr/bin/env bash
set -euo pipefail

# =============================================================================
# BMSSP Build Script - Improved Version
# =============================================================================

CD_DIR="$(dirname "$0")"
cd "$CD_DIR"
mkdir -p bin

# Configuración
SRC_FILES=("bmssp.cpp" "main.cpp")
COMPILER="g++"
STD="c++17"
OUTPUT="bin/bmssp"

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Funciones de logging
log_info() { echo -e "${BLUE}  INFO:${NC} $1"; }
log_success() { echo -e "${GREEN} SUCCESS:${NC} $1"; }
log_warning() { echo -e "${YELLOW}  WARNING:${NC} $1"; }
log_error() { echo -e "${RED} ERROR:${NC} $1" >&2; }

# Buscar archivo fuente
find_source_file() {
    for file in "${SRC_FILES[@]}"; do
        if [[ -f "$file" ]]; then
            echo "$file"
            return 0
        fi
    done
    
    # Buscar en directorio padre
    for file in "${SRC_FILES[@]}"; do
        if [[ -f "../$file" ]]; then
            echo "../$file"
            return 0
        fi
    done
    
    return 1
}

# Mostrar ayuda
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo "Build the BMSSP project"
    echo ""
    echo "OPTIONS:"
    echo "  -d, --debug      Build in debug mode"
    echo "  -r, --release    Build in release mode (default)"
    echo "  -t, --tests      Build with test flags"
    echo "  -c, --clean      Clean build directory"
    echo "  -h, --help       Show this help message"
    echo ""
    echo "EXAMPLES:"
    echo "  $0               # Build release version"
    echo "  $0 --debug       # Build debug version"
    echo "  $0 --clean       # Clean build directory"
}

# Parsear argumentos
BUILD_TYPE="release"
CLEAN_ONLY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="release"
            shift
            ;;
        -t|--tests)
            BUILD_TYPE="tests"
            shift
            ;;
        -c|--clean)
            CLEAN_ONLY=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Limpiar si se solicita
if [[ "$CLEAN_ONLY" == true ]]; then
    log_info "Cleaning build directory..."
    rm -rf bin/*
    log_success "Build directory cleaned"
    exit 0
fi

# Flags de compilación según el tipo de build
case "$BUILD_TYPE" in
    "debug")
        CXX_FLAGS="-g -O0 -DDEBUG -Wall -Wextra -pedantic -fsanitize=address"
        log_info "Building in DEBUG mode"
        ;;
    "tests")
        CXX_FLAGS="-g -O0 -DTEST_BUILD -Wall -Wextra -pedantic"
        log_info "Building in TEST mode"
        ;;
    "release")
        CXX_FLAGS="-O3 -march=native -DNDEBUG -Wall -Wextra -pedantic"
        log_info "Building in RELEASE mode"
        ;;
esac

# Buscar archivo fuente
log_info "Looking for source files..."
SRC_FILE=$(find_source_file)

if [[ -z "$SRC_FILE" ]]; then
    log_error "No source file found (tried: ${SRC_FILES[*]})"
    exit 1
fi

log_info "Found source: $SRC_FILE"

# Compilar
log_info "Compiling with flags: $CXX_FLAGS"
set +e
$COMPILER -std=$STD $CXX_FLAGS "$SRC_FILE" -o "$OUTPUT"
COMPILE_RESULT=$?
set -e

if [[ $COMPILE_RESULT -eq 0 ]]; then
    log_success "Build successful: $OUTPUT"
    
    # Mostrar información del binario
    if command -v file &> /dev/null; then
        log_info "Binary info: $(file "$OUTPUT")"
    fi
    
    if command -v size &> /dev/null; then
        log_info "Binary size:"
        size "$OUTPUT" || true
    fi
else
    log_error "Compilation failed with exit code: $COMPILE_RESULT"
    exit $COMPILE_RESULT
fi
