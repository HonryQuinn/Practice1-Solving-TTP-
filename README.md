# Practica 1 - Resolucion del Travelling Thief Problem

Este proyecto implementa y evalua un conjunto de heuristicas para resolver el **Travelling Thief Problem (TTP)** en C++. El TTP es un problema de optimizacion combinatoria que combina dos problemas clasicos NP-Dificiles: el Problema del Viajante (TSP) y el Problema de la Mochila (KP). El objetivo es maximizar la ganancia neta, definida como el valor total de los objetos recogidos menos el costo de alquiler de la mochila durante el tiempo total de viaje.

---

## Descripcion del Problema

Un ladron debe visitar un conjunto de ciudades exactamente una vez (tour ciclico) y recoger selectivamente objetos rentables almacenados en esas ciudades. El ladron lleva una mochila con capacidad fija y la alquila a una tarifa fija por unidad de tiempo. La velocidad de viaje disminuye linealmente a medida que la mochila se llena, creando una dependencia directa entre los componentes TSP y KP. El objetivo es encontrar un tour y un plan de recogida que maximicen conjuntamente:
```
Objetivo = Ganancia Total - Ratio de Alquiler * Tiempo Total de Viaje
```

---

## Estructura del Proyecto
```
.
├── main.cpp            # Punto de entrada: carga la instancia, configura y ejecuta el experimento
├── reader.cpp          # Parser de archivos de instancia TTP y construccion de matriz de distancias
├── base1.h             # Estructuras de datos, clase base de heuristicas, logica de evaluacion y experimento
├── ttp_heuristics.h    # Implementacion de todas las heuristicas
├── simulador           # Binario compilado (Linux x86-64)
├── textos              # Salida de ejemplo de ejecuciones sobre distintas instancias
├── trt                 # Comandos de shell de ejemplo para ejecutar el simulador
└── README.md
```

---

## Requisitos

- Compilador de C++ con soporte C++11 (se recomienda g++)
- Unicamente biblioteca estandar (sin dependencias externas)

---

## Compilacion
```bash
g++ -O2 -std=c++11 -o simulador main.cpp
```

---

## Uso
```bash
./simulador <archivo_instancia_ttp> [num_ejecuciones]
```

- `archivo_instancia_ttp`: Ruta a un archivo de benchmark `.ttp`.
- `num_ejecuciones`: Opcional. Numero de veces que se ejecuta cada heuristica (por defecto: 1).

**Ejemplo:**
```bash
./simulador "./Instancias/fl1577_n1576_uncorr_01.ttp" 5
```

---

## Formato de Instancia

El solver lee archivos TTP estandar con la siguiente estructura:
```
PROBLEM NAME:         <nombre>
DIMENSION:            <numero de ciudades>
NUMBER OF ITEMS:      <numero de objetos>
CAPACITY OF KNAPSACK: <peso maximo>
MIN SPEED:            <vmin>
MAX SPEED:            <vmax>
RENTING RATIO:        <R>
NODE_COORD_SECTION
<id> <x> <y>
...
ITEMS SECTION
<id> <ganancia> <peso> <id_ciudad>
...
```

Las instancias de benchmark de la competencia CEC 2014 TTP pueden descargarse desde:
https://cs.adelaide.edu.au/~optlog/CEC2014COMP_InstancesNew/

---

## Heuristicas Implementadas

Todas las heuristicas heredan de la clase abstracta `TTPHeuristic` definida en `base1.h`.

### Baselines Constructivas

| Clase | Nombre | Estrategia de Tour | Estrategia de Recogida |
|---|---|---|---|
| `SequentialNoItems` | Tour Secuencial + Sin Items | Ciudades en orden 0..n-1 | Ninguna |
| `NearestNeighborGreedy` | Vecino mas Cercano + Picking Greedy | Vecino mas cercano desde ciudad 0 | Greedy por ratio ganancia/peso |
| `RandomTourGreedy` | Tour Aleatorio + Picking Greedy | Tour aleatorio | Greedy por ratio ganancia/peso |
| `HighProfitPicking` | Vecino mas Cercano + Picking por Ganancia | Vecino mas cercano | Ordenado por ganancia absoluta |
| `HillClimbingPicking` | Tour NN + Hill Climbing en Picking | Vecino mas cercano | Hill climbing con bit-flip sobre items |

### Heuristicas Optimizadas

| Clase | Nombre | Descripcion |
|---|---|---|
| `LocalSearch2Opt` | Busqueda Local 2-Opt + Picking Greedy | Tour NN mejorado con 2-opt limitado y Or-opt |
| `ProbabilisticNearestNeighbor2Opt` | NN Probabilistico + 2-Opt | Seleccion probabilistica de ciudades con softmax y refinamiento 2-opt |
| `ImprovedHillClimbing` | Hill Climbing Mejorado (Picking Adaptativo 75%) | Picking adaptativo al 75% de capacidad con mejora conjunta 2-opt/picking |
| `Balanced2Opt` | 2-Opt + Picking Balanceado (70%) | Mejora del tour con 2-opt seguida de repicking adaptativo al 70% de capacidad |
| `BalancedLNS` | LNS Balanceado | Busqueda de Gran Vecindad: destruccion y reconstruccion del tour con picking adaptativo |
| `BalancedVNS` | VNS Balanceado | Busqueda de Vecindad Variable con sacudidas aleatorias y mejora conjunta |

### Operadores de Busqueda Local

- **2-opt (limitado):** Invierte subsegmentos del tour dentro de una ventana de vecinos configurable.
- **Or-opt:** Reubica segmentos de 1 a 3 ciudades consecutivas en mejores posiciones del tour.
- **Picking adaptativo:** Selecciona objetos de forma greedy hasta una fraccion dada de la capacidad, ponderada por ratio ganancia/peso ajustado por la distancia al final del tour.
- **Mejora conjunta:** Alterna entre mejora del tour con 2-opt y optimizacion del picking hasta que no haya mejoras.

---

## Configuracion del Experimento

Abrir `main.cpp` y descomentar las heuristicas deseadas dentro del bloque `TTPExperiment`:
```cpp
TTPExperiment experiment(instance, num_runs);

// Descomentar para activar:
// experiment.addHeuristic(new NearestNeighborGreedy(instance));
// experiment.addHeuristic(new HillClimbingPicking(instance));
// experiment.addHeuristic(new ImprovedHillClimbing(instance));
// experiment.addHeuristic(new BalancedLNS(instance, 10, 20));
experiment.addHeuristic(new BalancedLNS(instance, 20, 40));   // activo por defecto
// experiment.addHeuristic(new BalancedVNS(instance, 50, 5));
```

Parametros de `BalancedLNS`: `BalancedLNS(instancia, tamano_destruccion, max_iteraciones)`
Parametros de `BalancedVNS`: `BalancedVNS(instancia, max_iteraciones, k_max)`

---

## Salida del Programa

El programa imprime un resumen por heuristica y un ranking global al final:
```
EXPERIMENTO TTP - HEURISTICAS
Numero de ejecuciones por heuristica: 1

>>> Ejecutando: Balanced LNS (destroy=20, iter=40) <

  RESULTADOS:
    Objetivo: 57389.3
    Ganancia: 671108
    Tiempo: 45292.9
    Peso: 970380/1439258

MEJOR SOLUCION GLOBAL:
Heuristica: Balanced LNS (destroy=20, iter=40)
Objetivo: 57389.3
```

Cuando se solicitan multiples ejecuciones, la salida incluye ademas el promedio, mejor, peor y desviacion estandar del objetivo.

---

## Resultado de Ejemplo

Instancia: `fl1577-TTP` (1577 ciudades, 1576 objetos, capacidad 1,439,258)

| Metrica | Valor |
|---|---|
| Heuristica | Balanced LNS (destroy=20, iter=40) |
| Objetivo | 57,389.3 |
| Ganancia Total | 671,108 |
| Tiempo Total | 45,292.9 |
| Peso Utilizado | 970,380 / 1,439,258 |

---

## Referencias

- Polyakovskiy et al., "A Comprehensive Benchmark Set and Heuristics for the Traveling Thief Problem," GECCO 2014.
- Mei, Li, Yao, "Improving Efficiency of Heuristics for the Large Scale Traveling Thief Problem," SEAL 2014.
- Namazi et al., "Solving Travelling Thief Problems using Coordination Based Methods," Journal of Heuristics, 2023.
