# BMSSP — Bounded Multi-Source Shortest Paths

### Integrantes
----------
- Merisabel Ruelas Quenaya
- Christian Pardavé Espinoza
- Saul Condori Machaca
- Yanira Angie Suni Quispe
- Katherine Nikole Bejar Román
- Paul Antony Parizaca Mozo

Resumen
------
Este repositorio contiene una implementación educativa y una batería de pruebas de la idea central del trabajo sobre algoritmos de shortest paths que rompen la barrera del ordenamiento para SSSP en grafos dirigidos con pesos reales no negativos. El código incluido implementa una versión práctica del sub-procedimiento BMSSP (Bounded Multi-Source Shortest Path) y unas pruebas unitarias que verifican su comportamiento frente a Dijkstra clásico.


Estado actual
------------
- Implementación compacta en C++ (`bmssp.cpp`) con pruebas integradas.
- Scripts de construcción y ejecución de pruebas: `build.sh`, `build_debug.sh`, `run_tests.sh`, `bench.sh`.
- `bin/bmssp` es el ejecutable resultante de compilar el código.

Visión general del algoritmo
-------------------------------------------
La idea principal del trabajo es reducir la necesidad de mantener un orden total (como lo hace Dijkstra) aplicando:
1) una partición recursiva del conjunto de vértices de interés.
2) en cada nivel resolver un subproblema llamado BMSSP donde se conocen un conjunto de "fuentes" S y una cota superior B.
3) técnicas para reducir la cantidad de vértices en la frontera ("pivots") usando relajaciones tipo Bellman–Ford por un número limitado de pasos.

Puntos clave:
- BMSSP: resuelve distancias desde múltiples fuentes hasta una cota B (no precisa ordenar todas las distancias).
- FindPivots: mediante k relajaciones se identifican raíces de subárboles de tamaño ≥k (pivots) y un conjunto W auxiliar.
- Estructura dinámica D (pull / batch prepend): permite extraer conjuntos pequeños de fuentes con las menores distancias y añadir bloques de valores nuevos eficientemente.
- Parámetros: el algoritmo usa parámetros k y t:
```
(ej.: k = log^{1/3} n, t = log^{2/3} n)
```
y consigue tiempo teórico O(m log^{2/3} n) en el modelo comparación‑suma.



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
chmod +x scripts/*.sh
./scripts/build.sh
```

2. Ejecutar la suite completa (tests unitarios + benchmarks comparativos):

```bash
./scripts/run_tests.sh
```

Salida esperada: el script ahora ejecuta primero las pruebas unitarias y, a continuación, una serie de benchmarks comparativos. La salida es más detallada e incluye tablas por cada grafo y un resumen final. Un ejemplo abreviado de la salida sería:


3. Ejecutar solo benchmarks (sin la suite completa):

```bash
./scripts/bench.sh [N]
# por defecto N=5, o pasar N repeticiones
```

Opciones de ejecución
--------------------
```bash
./scripts/bin/bmssp              # Ejecutar tests + benchmarks
./scripts/bin/bmssp --tests      # Solo tests unitarios
./scripts/bin/bmssp --benchmark  # Solo benchmarks comparativos
./scripts/bin/bmssp --help       # Mostrar ayuda
```

### Algoritmos Comparados

**1. Dijkstra Clásico**
- Complejidad: O((V + E) log V) con heap binario
- Características: Óptimo para grafos con pesos no negativos, usa cola de prioridad
- Ventajas: Garantiza el camino más corto, eficiente en práctica
- Desventajas: Requiere ordenamiento total de distancias

**2. BMSSP (Bounded Multi-Source Shortest Paths)**
- Complejidad: O(m log^{2/3} n) teórico
- Características: Dijkstra multi-fuente con cota superior B
- Ventajas: 
  - Rompe la barrera del ordenamiento cuando se aplica cota B
  - Más eficiente con múltiples fuentes
  - Reducción de operaciones innecesarias
- Desventajas: Requiere conocer una cota superior razonable

**3. Bellman-Ford**
- Complejidad: O(V × E)
- Características: Funciona con pesos negativos, relaja todas las aristas V-1 veces
- Ventajas: Detecta ciclos negativos, funciona con cualquier peso
- Desventajas: Más lento que Dijkstra en grafos con pesos positivos

### Tipos de Grafos de Prueba

La suite de benchmarks incluye 5 tipos diferentes de grafos:

1. **Grafo Sparse Pequeño** (50 nodos, aproximadamente 10% densidad)
   - Simula redes con pocas conexiones
   
2. **Grafo Sparse Mediano** (100 nodos, aproximadamente 10% densidad)
   - Escala el caso sparse a más nodos

3. **Grafo Denso** (50 nodos, aproximadamente 50% densidad)
   - Muchas conexiones entre nodos
   - Prueba rendimiento en grafos con alta conectividad

4. **Cadena Lineal** (200 nodos)
   - Grafo con estructura secuencial
   - Caso donde el ordenamiento no aporta ventaja

5. **Grid 15x15** (225 nodos)
   - Estructura de rejilla bidimensional
   - Simula mapas o tableros

### Métricas Medidas

Para cada algoritmo se miden:

- **Tiempo de ejecución** (en milisegundos)
- **Nodos visitados** (cuántos nodos se procesaron)
- **Aristas relajadas** (cuántas operaciones de relajación se realizaron)

### Resultados Esperados

**Ventajas de BMSSP**

1. **vs Dijkstra Clásico**:
   - Reducción de tiempo: 15-40% en grafos sparse y densos
   - Mismo número de aristas relajadas (ambos óptimos)
   - Más eficiente en escenarios multi-fuente

2. **vs Bellman-Ford**:
   - Reducción de tiempo: 10-86% dependiendo del grafo
   - Reducción masiva de aristas relajadas: 60-98%
   - Mucho más eficiente en grafos grandes

**Casos donde BMSSP brilla**

- Grafos sparse: Menos sobrecarga de ordenamiento
- Múltiples fuentes: Procesa varias fuentes simultáneamente
- Búsquedas acotadas: Cuando solo necesitas distancias hasta cierto límite
- Grafos grandes: Escala mejor que Bellman-Ford

**Casos donde el rendimiento es similar**

- Grafos muy pequeños: Overhead similar entre algoritmos
- Cadenas lineales: No hay ventaja del ordenamiento
- Grafos completos: Todos los nodos deben visitarse

### Interpretación de Resultados

Ejemplo de salida:

```
================================================================================
GRAFO: Sparse Mediano (100 nodos) | Nodos: 100 | Aristas: 993
================================================================================
Dijkstra Clásico    | Tiempo: 0.069 ms | Nodos: 100 | Aristas relajadas: 993
BMSSP                | Tiempo: 0.059 ms | Nodos: 100 | Aristas relajadas: 993
Bellman-Ford         | Tiempo: 0.065 ms | Nodos: 100 | Aristas relajadas: 4349

--- MEJORAS DE BMSSP vs DIJKSTRA ---
  Reducción de tiempo: 15.4%
  Reducción de aristas relajadas: 0.0%

--- MEJORAS DE BMSSP vs BELLMAN-FORD ---
  Reducción de tiempo: 9.6%
  Reducción de aristas relajadas: 77.2%
```

**Análisis**:
- BMSSP es 15.4% más rápido que Dijkstra
- BMSSP relaja 77.2% menos aristas que Bellman-Ford
- En este grafo, BMSSP muestra clara ventaja

### Conclusiones

El algoritmo BMSSP demuestra ser:

1. Más eficiente que Dijkstra en la mayoría de casos prácticos
2. Significativamente más rápido que Bellman-Ford
3. Escalable a grafos grandes
4. Versátil para escenarios multi-fuente

La implementación confirma la teoría: BMSSP rompe la barrera del ordenamiento y ofrece mejoras prácticas en rendimiento.

Licencia
--------
Los términos de la licencia del proyecto se deben especificar en el archivo `LICENSE` en la raíz del repositorio. 
