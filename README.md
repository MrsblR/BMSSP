# BMSSP — Bounded Multi-Source Shortest Paths (impl.)

Resumen
------
Este repositorio contiene una implementación educativa y una batería de pruebas de la idea central del trabajo sobre algoritmos de shortest paths que rompen la barrera del ordenamiento para SSSP en grafos dirigidos con pesos reales no negativos. El código incluido implementa una versión práctica del sub-procedimiento BMSSP (Bounded Multi-Source Shortest Path) y unas pruebas unitarias que verifican su comportamiento frente a Dijkstra clásico.

Estado actual
------------
- Implementación compacta en C++ (`bmssp.cpp`) con pruebas integradas.
- Scripts de construcción y ejecución de pruebas: `build.sh`, `build_debug.sh`, `run_tests.sh`, `bench.sh`.
- `bin/bmssp` es el ejecutable resultante de compilar el código.

Visión general del algoritmo (resumen técnico)
-------------------------------------------
La idea principal del trabajo es reducir la necesidad de mantener un orden total (como lo hace Dijkstra) aplicando: 1) una partición recursiva del conjunto de vértices de interés, 2) en cada nivel resolver un subproblema llamado BMSSP donde se conocen un conjunto de "fuentes" S y una cota superior B, y 3) técnicas para reducir la cantidad de vértices en la frontera ("pivots") usando relajaciones tipo Bellman–Ford por un número limitado de pasos.

Puntos clave:
- BMSSP: resuelve distancias desde múltiples fuentes hasta una cota B (no precisa ordenar todas las distancias).
- FindPivots: mediante k relajaciones se identifican raíces de subárboles de tamaño ≥k (pivots) y un conjunto W auxiliar.
- Estructura dinámica D (pull / batch prepend): permite extraer conjuntos pequeños de fuentes con las menores distancias y añadir bloques de valores nuevos eficientemente.
- Parámetros: el algoritmo usa parámetros k y t (ej.: k = log^{1/3} n, t = log^{2/3} n) y consigue tiempo teórico O(m log^{2/3} n) en el modelo comparación‑suma.

Nota importante sobre esta implementación
----------------------------------------
El código incluido en este repositorio proporciona una implementación práctica y reducida del subprocedimiento BMSSP (una Dijkstra multi‑fuente acotada) y un conjunto de pruebas que comparan su resultado con Dijkstra clásico. No es una implementación completa del algoritmo teórico con todas las estructuras avanzadas y optimizaciones descritas en el artículo; sirve como base de verificación y referencia para experimentar.

Archivos y scripts
------------------
- `bmssp.cpp` / `main.cpp` — Implementación en C++ con pruebas unitarias integradas (funciones `prueba_*`). Contienen:
  - Un Dijkstra clásico para verificación.
  - Una implementación de BMSSP simplificada: Dijkstra multi‑fuente acotado por B.
  - Tests: lineal, convergente, desconectado y multi‑fuentes equivalentes.
- `build.sh` — Compila el código en `bin/bmssp` en modo optimizado (release, `-O2`, `-std=c++17`).
  - Nota: se añadió un pequeño fallback para compilar `main.cpp` si `bmssp.cpp` no está presente.
- `build_debug.sh` — (intención) compila en modo debug con sanitizers (ASAN/UBSAN) y genera `bin/bmssp_debug`.
- `run_tests.sh` — Ejecuta `bin/bmssp` (compila primero si es necesario) y reporta el resultado de las pruebas.
- `bench.sh` — Ejecuta el binario repetidas veces y mide tiempos con `/usr/bin/time` (útil para micro‑benchmarks).
- `README_TESTS.md` — Documentación adicional sobre pruebas y uso rápido.

Cómo compilar y ejecutar las pruebas
-----------------------------------
1. Preparar permisos (si es necesario) y compilar:

```bash
cd /ruta/al/repositorio/BMSSP
chmod +x build.sh run_tests.sh
./build.sh
```

2. Ejecutar las pruebas integradas:

```bash
./run_tests.sh
```

Salida esperada: `✓ Todas las pruebas unitarias pasaron correctamente` y exit code 0.

3. **Ejecutar benchmarks comparativos** (nuevo):

```bash
./scripts/bin/bmssp --benchmark
# O usar el script completo:
./scripts/benchmark_extendido.sh
```

Esto ejecutará comparaciones de rendimiento entre BMSSP, Dijkstra Clásico y Bellman-Ford en diferentes tipos de grafos.

**Métricas comparadas:**
- ⏱️ Tiempo de ejecución (ms)
- 🔍 Nodos visitados
- 🔄 Aristas relajadas

**Tipos de grafos probados:**
- Grafos sparse (dispersos)
- Grafos densos
- Cadenas lineales
- Grids (rejillas)

Ver `README_BENCHMARK.md` para detalles completos de los resultados y análisis.

Opciones de ejecución
--------------------
```bash
./scripts/bin/bmssp              # Ejecutar tests + benchmarks
./scripts/bin/bmssp --tests      # Solo tests unitarios
./scripts/bin/bmssp --benchmark  # Solo benchmarks comparativos
./scripts/bin/bmssp --help       # Mostrar ayuda
```

Opciones avanzadas
------------------
- Compilar en modo debug (si existe `build_debug.sh`):

```bash
./build_debug.sh
./bin/bmssp_debug
```

- Benchmark simple (si desea medir tiempos):

```bash
./bench.sh [N]
# por defecto N=5, o pasar N repeticiones
```


Licencia
--------
Los términos de la licencia del proyecto se deben especificar en el archivo `LICENSE` en la raíz del repositorio. 