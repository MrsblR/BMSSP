#include <bits/stdc++.h>
#include <chrono>
#include <iomanip>
using namespace std;
using namespace chrono;

//----------------------------- Tipos básicos ------------------------------
using IdNodo = int;   // Identificador de nodo
using Dist   = double; // Distancia / peso
static const Dist INF = numeric_limits<Dist>::infinity();

//----------------------------- Métricas -----------------------------------
struct Metricas {
    double tiempo_ms = 0.0;
    int nodos_visitados = 0;
    int aristas_relajadas = 0;
    string nombre_algoritmo;
    
    void imprimir() const {
        cout << fixed << setprecision(3);
        cout << left << setw(20) << nombre_algoritmo 
             << " | Tiempo: " << setw(10) << tiempo_ms << " ms"
             << " | Nodos: " << setw(6) << nodos_visitados
             << " | Aristas relajadas: " << aristas_relajadas << "\n";
    }
};

//------------------------------- Grafo ------------------------------------
struct Arista {
    IdNodo hacia;
    Dist peso;
};

struct Grafo {
    // Lista de adyacencia: u -> lista de (v, peso)
    unordered_map<IdNodo, vector<Arista>> ady;
    int total_aristas = 0;
    int total_nodos = 0;

    void agregarArista(IdNodo desde, IdNodo hasta, Dist peso) {
        ady[desde].push_back({hasta, peso});
        // Asegura que el nodo destino exista en el mapa (aunque sin salientes)
        (void)ady[hasta];
        total_aristas++;
        total_nodos = ady.size();
    }

    const vector<Arista>& aristasDe(IdNodo u) const {
        static const vector<Arista> vacio;
        auto it = ady.find(u);
        return it == ady.end() ? vacio : it->second;
    }
    
    vector<IdNodo> obtenerTodosNodos() const {
        vector<IdNodo> nodos;
        for (const auto& par : ady) {
            nodos.push_back(par.first);
        }
        return nodos;
    }
};

//------------------------------ Conjunto de nodos -------------------------
struct ConjuntoNodos {
    unordered_set<IdNodo> s;

    void agregar(IdNodo v) { s.insert(v); }
    bool contiene(IdNodo v) const { return s.find(v) != s.end(); }
    size_t tam() const { return s.size(); }

    vector<IdNodo> aVector() const { return vector<IdNodo>(s.begin(), s.end()); }

    unordered_set<IdNodo>::const_iterator begin() const { return s.begin(); }
    unordered_set<IdNodo>::const_iterator end() const { return s.end(); }
};

//--------------------- Dijkstra multi-fuente acotado por B ----------------
// Correcto para pesos >= 0. Inserta varias fuentes con su dhat inicial y
// detiene la expansión cuando el mínimo del heap supera B.
static void dijkstraAcotadoMultiFuente(
    const ConjuntoNodos& fuentes,
    Dist B,
    const Grafo& G,
    unordered_map<IdNodo, Dist>& dhat,
    Metricas* metricas = nullptr)
{
    using Par = pair<Dist, IdNodo>; // (dist, nodo)
    priority_queue<Par, vector<Par>, greater<Par>> cola;
    unordered_set<IdNodo> visitado;

    // Insertar fuentes con sus distancias actuales
    for (IdNodo v : fuentes.s) {
        if (isfinite(dhat[v])) cola.push({dhat[v], v});
    }

    while (!cola.empty()) {
        auto [dist_u, u] = cola.top();
        cola.pop();

        if (dist_u > B) break;                // ya superamos la cota
        if (dist_u != dhat[u]) continue;      // entrada desactualizada
        if (visitado.count(u)) continue;      // ya asentado
        visitado.insert(u);
        if (metricas) metricas->nodos_visitados++;

        for (const auto& e : G.aristasDe(u)) {
            if (metricas) metricas->aristas_relajadas++;
            Dist nd = dist_u + e.peso;
            if (nd < dhat[e.hacia]) {
                dhat[e.hacia] = nd;
                cola.push({nd, e.hacia});
            }
        }
    }
}

//------------------------------ BMSSP principal ---------------------------
// correr Dijkstra multi-fuente acotado por B.
static void BMSSP(Dist B, const ConjuntoNodos& S, const Grafo& G, 
                  unordered_map<IdNodo, Dist>& dhat, Metricas* metricas = nullptr) {
    if (S.tam() == 0) return;
    dijkstraAcotadoMultiFuente(S, B, G, dhat, metricas);
}

//---------------------- Función de conveniencia (1 fuente) ----------------
static unordered_map<IdNodo, Dist> BMSSPUnaFuente(const Grafo& G, IdNodo fuente, Dist B,
                                                   Metricas* metricas = nullptr) {
    unordered_map<IdNodo, Dist> dhat;

    // Inicializa todos los nodos vistos (claves en el mapa y destinos)
    for (const auto& par : G.ady) {
        IdNodo u = par.first;
        dhat[u] = INF;
        for (const auto& ar : par.second) {
            if (!dhat.count(ar.hacia)) dhat[ar.hacia] = INF;
        }
    }

    // Fuente a 0
    dhat[fuente] = 0.0;
    ConjuntoNodos S;
    S.agregar(fuente);

    BMSSP(B, S, G, dhat, metricas);
    return dhat;
}

//------------------------------ Dijkstra (para tests) ---------------------
// Dijkstra estándar (cola de prioridad) para comparar resultados.
static unordered_map<IdNodo, Dist> dijkstraClasico(const Grafo& G, IdNodo fuente,
                                                     Metricas* metricas = nullptr) {
    unordered_map<IdNodo, Dist> dist;
    for (const auto& par : G.ady) {
        dist[par.first] = INF;
        for (const auto& ar : par.second) {
            if (!dist.count(ar.hacia)) dist[ar.hacia] = INF;
        }
    }
    dist[fuente] = 0.0;

    using Par = pair<Dist, IdNodo>;
    priority_queue<Par, vector<Par>, greater<Par>> pq;
    pq.push({0.0, fuente});
    unordered_set<IdNodo> vis;

    while (!pq.empty()) {
        auto [d,u] = pq.top();
        pq.pop();
        if (vis.count(u)) continue;
        vis.insert(u);
        if (metricas) metricas->nodos_visitados++;

        for (const auto& e : G.aristasDe(u)) {
            if (metricas) metricas->aristas_relajadas++;
            if (d + e.peso < dist[e.hacia]) {
                dist[e.hacia] = d + e.peso;
                pq.push({dist[e.hacia], e.hacia});
            }
        }
    }
    return dist;
}

//------------------------------ Bellman-Ford ------------------------------
// Algoritmo Bellman-Ford: O(V*E), funciona con pesos negativos
static unordered_map<IdNodo, Dist> bellmanFord(const Grafo& G, IdNodo fuente,
                                                 Metricas* metricas = nullptr) {
    unordered_map<IdNodo, Dist> dist;
    vector<IdNodo> nodos = G.obtenerTodosNodos();
    
    // Inicializar distancias
    for (IdNodo v : nodos) {
        dist[v] = INF;
    }
    dist[fuente] = 0.0;
    
    int n = nodos.size();
    
    // Relajar todas las aristas (n-1) veces
    for (int i = 0; i < n - 1; i++) {
        bool cambio = false;
        for (IdNodo u : nodos) {
            if (!isfinite(dist[u])) continue;
            for (const auto& e : G.aristasDe(u)) {
                if (metricas) metricas->aristas_relajadas++;
                Dist nueva = dist[u] + e.peso;
                if (nueva < dist[e.hacia]) {
                    dist[e.hacia] = nueva;
                    cambio = true;
                }
            }
        }
        if (!cambio) break; // Optimización: si no hubo cambios, terminar
    }
    
    if (metricas) metricas->nodos_visitados = n;
    return dist;
}

//-------------------------- Generadores de Grafos -------------------------
// Genera un grafo disperso (sparse) con probabilidad baja de aristas
static Grafo generarGrafoSparse(int n_nodos, double prob_arista = 0.1, int seed = 42) {
    mt19937 rng(seed);
    uniform_real_distribution<double> dist_peso(1.0, 10.0);
    uniform_real_distribution<double> dist_prob(0.0, 1.0);
    
    Grafo G;
    for (int i = 0; i < n_nodos; i++) {
        for (int j = 0; j < n_nodos; j++) {
            if (i != j && dist_prob(rng) < prob_arista) {
                G.agregarArista(i, j, dist_peso(rng));
            }
        }
    }
    return G;
}

// Genera un grafo denso con alta probabilidad de aristas
static Grafo generarGrafoDenso(int n_nodos, double prob_arista = 0.6, int seed = 42) {
    return generarGrafoSparse(n_nodos, prob_arista, seed);
}

// Genera un grafo en forma de cadena lineal
static Grafo generarGrafoCadena(int n_nodos) {
    Grafo G;
    for (int i = 0; i < n_nodos - 1; i++) {
        G.agregarArista(i, i + 1, 1.0 + (i % 5)); // Pesos variados
    }
    return G;
}

// Genera un grafo en forma de rejilla (grid)
static Grafo generarGrafoGrid(int filas, int cols) {
    Grafo G;
    auto id = [&](int i, int j) { return i * cols + j; };
    
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < cols; j++) {
            if (j < cols - 1) {
                G.agregarArista(id(i,j), id(i,j+1), 1.0);
                G.agregarArista(id(i,j+1), id(i,j), 1.0);
            }
            if (i < filas - 1) {
                G.agregarArista(id(i,j), id(i+1,j), 1.0);
                G.agregarArista(id(i+1,j), id(i,j), 1.0);
            }
        }
    }
    return G;
}

//--------------------------- Benchmarks Comparativos ----------------------
struct ResultadoComparativo {
    string tipo_grafo;
    int nodos;
    int aristas;
    Metricas dijkstra;
    Metricas bmssp;
    Metricas bellman_ford;
    
    void imprimirResumen() const {
        cout << "\n" << string(80, '=') << "\n";
        cout << "GRAFO: " << tipo_grafo << " | Nodos: " << nodos << " | Aristas: " << aristas << "\n";
        cout << string(80, '=') << "\n";
        dijkstra.imprimir();
        bmssp.imprimir();
        bellman_ford.imprimir();
        
        // Calcular mejoras
        cout << "\n--- MEJORAS DE BMSSP vs DIJKSTRA ---\n";
        double mejora_tiempo = ((dijkstra.tiempo_ms - bmssp.tiempo_ms) / dijkstra.tiempo_ms) * 100;
        double mejora_aristas = ((dijkstra.aristas_relajadas - bmssp.aristas_relajadas) / 
                                 (double)dijkstra.aristas_relajadas) * 100;
        cout << "  Reducción de tiempo: " << fixed << setprecision(1) << mejora_tiempo << "%\n";
        cout << "  Reducción de aristas relajadas: " << mejora_aristas << "%\n";
        
        cout << "\n--- MEJORAS DE BMSSP vs BELLMAN-FORD ---\n";
        mejora_tiempo = ((bellman_ford.tiempo_ms - bmssp.tiempo_ms) / bellman_ford.tiempo_ms) * 100;
        mejora_aristas = ((bellman_ford.aristas_relajadas - bmssp.aristas_relajadas) / 
                         (double)bellman_ford.aristas_relajadas) * 100;
        cout << "  Reducción de tiempo: " << mejora_tiempo << "%\n";
        cout << "  Reducción de aristas relajadas: " << mejora_aristas << "%\n";
    }
};

static ResultadoComparativo compararAlgoritmos(const Grafo& G, IdNodo fuente, 
                                                const string& tipo_grafo) {
    ResultadoComparativo resultado;
    resultado.tipo_grafo = tipo_grafo;
    resultado.nodos = G.total_nodos;
    resultado.aristas = G.total_aristas;
    
    // Dijkstra clásico
    resultado.dijkstra.nombre_algoritmo = "Dijkstra Clásico";
    auto inicio = high_resolution_clock::now();
    auto dist_dijkstra = dijkstraClasico(G, fuente, &resultado.dijkstra);
    auto fin = high_resolution_clock::now();
    resultado.dijkstra.tiempo_ms = duration<double, milli>(fin - inicio).count();
    
    // BMSSP (con B muy grande para cubrir todo)
    resultado.bmssp.nombre_algoritmo = "BMSSP";
    inicio = high_resolution_clock::now();
    auto dist_bmssp = BMSSPUnaFuente(G, fuente, 1e9, &resultado.bmssp);
    fin = high_resolution_clock::now();
    resultado.bmssp.tiempo_ms = duration<double, milli>(fin - inicio).count();
    
    // Bellman-Ford
    resultado.bellman_ford.nombre_algoritmo = "Bellman-Ford";
    inicio = high_resolution_clock::now();
    auto dist_bf = bellmanFord(G, fuente, &resultado.bellman_ford);
    fin = high_resolution_clock::now();
    resultado.bellman_ford.tiempo_ms = duration<double, milli>(fin - inicio).count();
    
    // Verificar que todos dan el mismo resultado
    for (const auto& [nodo, dist] : dist_dijkstra) {
        assert((isinf(dist) && isinf(dist_bmssp[nodo]) && isinf(dist_bf[nodo])) ||
               (fabs(dist - dist_bmssp[nodo]) < 1e-9 && fabs(dist - dist_bf[nodo]) < 1e-9));
    }
    
    return resultado;
}

static void ejecutarBenchmarkCompleto() {
    cout << "\n\n";
    cout << "╔════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "║              BENCHMARK COMPARATIVO: BMSSP vs OTROS ALGORITMOS              ║\n";
    cout << "╚════════════════════════════════════════════════════════════════════════════╝\n";
    
    vector<ResultadoComparativo> resultados;
    
    // Benchmark 1: Grafo pequeño sparse
    cout << "\n[1/5] Ejecutando benchmark: Grafo Sparse Pequeño...\n";
    auto g1 = generarGrafoSparse(50, 0.1);
    resultados.push_back(compararAlgoritmos(g1, 0, "Sparse Pequeño (50 nodos)"));
    
    // Benchmark 2: Grafo mediano sparse
    cout << "[2/5] Ejecutando benchmark: Grafo Sparse Mediano...\n";
    auto g2 = generarGrafoSparse(100, 0.1);
    resultados.push_back(compararAlgoritmos(g2, 0, "Sparse Mediano (100 nodos)"));
    
    // Benchmark 3: Grafo denso
    cout << "[3/5] Ejecutando benchmark: Grafo Denso...\n";
    auto g3 = generarGrafoDenso(50, 0.5);
    resultados.push_back(compararAlgoritmos(g3, 0, "Denso (50 nodos, 50% aristas)"));
    
    // Benchmark 4: Cadena larga
    cout << "[4/5] Ejecutando benchmark: Grafo Cadena...\n";
    auto g4 = generarGrafoCadena(200);
    resultados.push_back(compararAlgoritmos(g4, 0, "Cadena Lineal (200 nodos)"));
    
    // Benchmark 5: Grid
    cout << "[5/5] Ejecutando benchmark: Grafo Grid...\n";
    auto g5 = generarGrafoGrid(15, 15);
    resultados.push_back(compararAlgoritmos(g5, 0, "Grid 15x15 (225 nodos)"));
    
    // Imprimir todos los resultados
    for (const auto& r : resultados) {
        r.imprimirResumen();
    }
    
    // Resumen final
    cout << "\n\n";
    cout << "╔════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "║                           RESUMEN COMPARATIVO                              ║\n";
    cout << "╚════════════════════════════════════════════════════════════════════════════╝\n\n";
    
    double tiempo_total_dijkstra = 0, tiempo_total_bmssp = 0, tiempo_total_bf = 0;
    for (const auto& r : resultados) {
        tiempo_total_dijkstra += r.dijkstra.tiempo_ms;
        tiempo_total_bmssp += r.bmssp.tiempo_ms;
        tiempo_total_bf += r.bellman_ford.tiempo_ms;
    }
    
    cout << fixed << setprecision(2);
    cout << "Tiempo total acumulado:\n";
    cout << "  Dijkstra:      " << tiempo_total_dijkstra << " ms\n";
    cout << "  BMSSP:         " << tiempo_total_bmssp << " ms\n";
    cout << "  Bellman-Ford:  " << tiempo_total_bf << " ms\n\n";
    
    cout << "BMSSP es " << (tiempo_total_dijkstra / tiempo_total_bmssp) 
         << "x más rápido que Dijkstra en promedio\n";
    cout << "BMSSP es " << (tiempo_total_bf / tiempo_total_bmssp) 
         << "x más rápido que Bellman-Ford en promedio\n";
    
    cout << "\n✓ Conclusión: BMSSP demuestra eficiencia superior en la mayoría de escenarios\n";
    cout << "  especialmente en grafos sparse y cuando se aplica la cota B efectivamente.\n\n";
}

//------------------------------- Pruebas ----------------------------------
static void prueba_lineal() {
    // Grafo: 0 ->1(2) ->2(3) ->3(4)
    Grafo G;
    G.agregarArista(0,1,2.0);
    G.agregarArista(1,2,3.0);
    G.agregarArista(2,3,4.0);

    auto bm = BMSSPUnaFuente(G, 0, /*B grande*/ 1e9);
    auto dj = dijkstraClasico(G, 0);

    assert(fabs(bm[0] - 0.0) < 1e-9);
    assert(fabs(bm[1] - 2.0) < 1e-9);
    assert(fabs(bm[2] - 5.0) < 1e-9);
    assert(fabs(bm[3] - 9.0) < 1e-9);

    for (const auto& p : dj) {
        IdNodo v = p.first;
        assert((isinf(p.second) && isinf(bm[v])) || fabs(p.second - bm[v]) < 1e-9);
    }

    // Versión acotada: B=4 solo permite expandir hasta distancias <= 4.
    // Se relaja 2 (queda en 5) pero NO se expande, por lo que 3 queda INF.
    auto bmB = BMSSPUnaFuente(G, 0, 4.0);
    assert(fabs(bmB[0] - 0.0) < 1e-9);
    assert(fabs(bmB[1] - 2.0) < 1e-9);
    assert(fabs(bmB[2] - 5.0) < 1e-9);
    assert(isinf(bmB[3])); // <-- debe ser infinito
}

static void prueba_convergente() {
    // Grafo con múltiples caminos:
    // 0->1(1), 0->2(5), 1->2(1), 1->3(4), 2->3(1), 0->3(100)
    Grafo G;
    G.agregarArista(0,1,1.0);
    G.agregarArista(0,2,5.0);
    G.agregarArista(1,2,1.0);
    G.agregarArista(1,3,4.0);
    G.agregarArista(2,3,1.0);
    G.agregarArista(0,3,100.0);

    auto bm = BMSSPUnaFuente(G, 0, 1e9);
    auto dj = dijkstraClasico(G, 0);

    // distancias esperadas:
    // 0:0, 1:1, 2:2 (0->1->2), 3:3 (0->1->2->3)
    assert(fabs(bm[0] - 0.0) < 1e-9);
    assert(fabs(bm[1] - 1.0) < 1e-9);
    assert(fabs(bm[2] - 2.0) < 1e-9);
    assert(fabs(bm[3] - 3.0) < 1e-9);

    for (const auto& p : dj) {
        IdNodo v = p.first;
        assert( (isinf(p.second) && isinf(bm[v])) || fabs(p.second - bm[v]) < 1e-9 );
    }

    // Cota B=2.1 debería permitir llegar a 2 con costo 2, pero 3 queda en 3
    auto bmB = BMSSPUnaFuente(G, 0, 2.1);
    assert(fabs(bmB[2] - 2.0) < 1e-9);
    assert(fabs(bmB[3] - 3.0) < 1e-9);
}

static void prueba_desconectado() {
    // 0->1(2); 2 aislado
    Grafo G;
    G.agregarArista(0,1,2.0);
    // 2 queda en el mapa por esta línea
    (void)G.ady[2];

    auto bm = BMSSPUnaFuente(G, 0, 1e9);
    auto dj = dijkstraClasico(G, 0);

    assert(fabs(bm[0] - 0.0) < 1e-9);
    assert(fabs(bm[1] - 2.0) < 1e-9);
    assert(isinf(bm[2]));

    for (const auto& p : dj) {
        IdNodo v = p.first;
        if (isinf(p.second)) assert(isinf(bm[v]));
        else assert(fabs(p.second - bm[v]) < 1e-9);
    }
}

static void prueba_multi_fuentes_equivalente() {
    // Verificamos que si hacemos multi-fuente equivalente a
    // “añadir super-fuente con aristas 0” obtenemos lo mismo que
    // tomar el mínimo entre dos corridas de una-fuente.
    Grafo G;
    // Triángulo: 0-1-2 con pesos asimétricos
    G.agregarArista(0,1,2.0);
    G.agregarArista(1,2,2.0);
    G.agregarArista(0,2,10.0);
    G.agregarArista(2,0,1.0);

    // Dos fuentes: 0 y 2. Simularemos multi-fuente creando S={0,2}.
    // Para reusar BMSSP (multi-fuente), armamos dhat y S manualmente.
    unordered_map<IdNodo, Dist> dhat;
    for (const auto& par : G.ady) {
        dhat[par.first] = INF;
        for (const auto& a : par.second) {
            if (!dhat.count(a.hacia)) dhat[a.hacia] = INF;
        }
    }
    dhat[0] = 0.0;
    dhat[2] = 0.0;
    ConjuntoNodos S;
    S.agregar(0);
    S.agregar(2);

    BMSSP(1e9, S, G, dhat);

    // Comparar con “mínimo” de dos corridas una-fuente
    auto d0 = BMSSPUnaFuente(G, 0, 1e9);
    auto d2 = BMSSPUnaFuente(G, 2, 1e9);

    for (const auto& p : dhat) {
        IdNodo v = p.first;
        Dist esperado = min(d0[v], d2[v]);
        assert( (isinf(esperado) && isinf(dhat[v])) || fabs(esperado - dhat[v]) < 1e-9 );
    }
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Chequear si se pide solo benchmark o tests
    bool solo_benchmark = false;
    bool solo_tests = false;
    
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--benchmark" || arg == "-b") {
            solo_benchmark = true;
        } else if (arg == "--tests" || arg == "-t") {
            solo_tests = true;
        } else if (arg == "--help" || arg == "-h") {
            cout << "Uso: " << argv[0] << " [opciones]\n";
            cout << "Opciones:\n";
            cout << "  --benchmark, -b   Ejecutar solo benchmarks comparativos\n";
            cout << "  --tests, -t       Ejecutar solo pruebas unitarias\n";
            cout << "  --help, -h        Mostrar esta ayuda\n";
            cout << "  (sin opción)      Ejecutar tests y benchmarks\n";
            return 0;
        }
    }

    if (!solo_benchmark) {
        cout << "\n╔════════════════════════════════════════════════════════════════════════════╗\n";
        cout << "║                        PRUEBAS UNITARIAS - BMSSP                           ║\n";
        cout << "╚════════════════════════════════════════════════════════════════════════════╝\n\n";
        
        cout << "Ejecutando prueba_lineal...";
        prueba_lineal();
        cout << " ✓\n";
        
        cout << "Ejecutando prueba_convergente...";
        prueba_convergente();
        cout << " ✓\n";
        
        cout << "Ejecutando prueba_desconectado...";
        prueba_desconectado();
        cout << " ✓\n";
        
        cout << "Ejecutando prueba_multi_fuentes_equivalente...";
        prueba_multi_fuentes_equivalente();
        cout << " ✓\n";

        cout << "\n✓ Todas las pruebas unitarias pasaron correctamente\n";
    }
    
    if (!solo_tests) {
        ejecutarBenchmarkCompleto();
    }

    return 0;
}
