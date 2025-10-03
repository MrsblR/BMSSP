#include <bits/stdc++.h>
using namespace std;

//----------------------------- Tipos básicos ------------------------------
using IdNodo = int;   // Identificador de nodo
using Dist   = double; // Distancia / peso
static const Dist INF = numeric_limits<Dist>::infinity();

//------------------------------- Grafo ------------------------------------
struct Arista {
    IdNodo hacia;
    Dist peso;
};

struct Grafo {
    // Lista de adyacencia: u -> lista de (v, peso)
    unordered_map<IdNodo, vector<Arista>> ady;

    void agregarArista(IdNodo desde, IdNodo hasta, Dist peso) {
        ady[desde].push_back({hasta, peso});
        // Asegura que el nodo destino exista en el mapa (aunque sin salientes)
        (void)ady[hasta];
    }

    const vector<Arista>& aristasDe(IdNodo u) const {
        static const vector<Arista> vacio;
        auto it = ady.find(u);
        return it == ady.end() ? vacio : it->second;
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
    unordered_map<IdNodo, Dist>& dhat)
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

        for (const auto& e : G.aristasDe(u)) {
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
static void BMSSP(Dist B, const ConjuntoNodos& S, const Grafo& G, unordered_map<IdNodo, Dist>& dhat) {
    if (S.tam() == 0) return;
    dijkstraAcotadoMultiFuente(S, B, G, dhat);
}

//---------------------- Función de conveniencia (1 fuente) ----------------
static unordered_map<IdNodo, Dist> BMSSPUnaFuente(const Grafo& G, IdNodo fuente, Dist B) {
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

    BMSSP(B, S, G, dhat);
    return dhat;
}

//------------------------------ Dijkstra (para tests) ---------------------
// Dijkstra estándar (cola de prioridad) para comparar resultados.
static unordered_map<IdNodo, Dist> dijkstraClasico(const Grafo& G, IdNodo fuente) {
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

        for (const auto& e : G.aristasDe(u)) {
            if (d + e.peso < dist[e.hacia]) {
                dist[e.hacia] = d + e.peso;
                pq.push({dist[e.hacia], e.hacia});
            }
        }
    }
    return dist;
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

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    prueba_lineal();
    prueba_convergente();
    prueba_desconectado();
    prueba_multi_fuentes_equivalente();

    cout << "Pruebas correctas\n";
    return 0;
}
