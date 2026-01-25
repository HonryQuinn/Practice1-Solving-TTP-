#include "reader.cpp"
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// ========================================
// AQUÍ IMPLEMENTAS TUS HEURÍSTICAS GREEDY
// ========================================

// Ejemplo 1: Tour greedy usando vecino más cercano
vector<int> greedyNearestNeighbor(const TTPInstance& inst) {
    vector<int> tour;
    vector<bool> visited(inst.dimension, false);
    
    int current = 0;  // Empezar en la ciudad 0
    tour.push_back(current);
    visited[current] = true;
    
    for (int step = 1; step < inst.dimension; step++) {
        double minDist = 1e9;
        int nearest = -1;
        
        // Buscar la ciudad más cercana no visitada
        for (int j = 0; j < inst.dimension; j++) {
            if (!visited[j] && inst.distances[current][j] < minDist) {
                minDist = inst.distances[current][j];
                nearest = j;
            }
        }
        
        current = nearest;
        tour.push_back(current);
        visited[current] = true;
    }
    
    return tour;
}

// Ejemplo 2: Selección greedy de items por ratio profit/weight
vector<int> greedyKnapsackByRatio(const TTPInstance& inst, const vector<int>& tour) {
    vector<int> pickingPlan(inst.num_items, 0);
    
    // Crear vector de índices de items ordenados por ratio profit/weight
    vector<pair<double, int>> ratios;
    for (int i = 0; i < inst.num_items; i++) {
        double ratio = (double)inst.items[i].profit / inst.items[i].weight;
        ratios.push_back({ratio, i});
    }
    
    // Ordenar de mayor a menor ratio
    sort(ratios.begin(), ratios.end(), greater<pair<double, int>>());
    
    // Seleccionar items mientras no se exceda la capacidad
    int currentWeight = 0;
    for (auto& p : ratios) {
        int itemIdx = p.second;
        if (currentWeight + inst.items[itemIdx].weight <= inst.capacity) {
            pickingPlan[itemIdx] = 1;
            currentWeight += inst.items[itemIdx].weight;
        }
    }
    
    return pickingPlan;
}

// Ejemplo 3: Selección greedy solo por profit
vector<int> greedyKnapsackByProfit(const TTPInstance& inst, const vector<int>& tour) {
    vector<int> pickingPlan(inst.num_items, 0);
    
    vector<pair<int, int>> profits;
    for (int i = 0; i < inst.num_items; i++) {
        profits.push_back({inst.items[i].profit, i});
    }
    
    sort(profits.begin(), profits.end(), greater<pair<int, int>>());
    
    int currentWeight = 0;
    for (auto& p : profits) {
        int itemIdx = p.second;
        if (currentWeight + inst.items[itemIdx].weight <= inst.capacity) {
            pickingPlan[itemIdx] = 1;
            currentWeight += inst.items[itemIdx].weight;
        }
    }
    
    return pickingPlan;
}

// Función auxiliar para imprimir solución
void printSolution(const TTPInstance& inst, const vector<int>& tour, 
                   const vector<int>& pickingPlan, const string& name) {
    cout << "\n=== " << name << " ===" << endl;
    
    // Mostrar tour
    cout << "Tour: ";
    for (int i = 0; i < min(10, (int)tour.size()); i++) {
        cout << tour[i] << " ";
    }
    if (tour.size() > 10) cout << "...";
    cout << endl;
    
    // Contar items seleccionados
    int itemsSelected = 0;
    int totalWeight = 0;
    int totalProfit = 0;
    for (int i = 0; i < inst.num_items; i++) {
        if (pickingPlan[i] == 1) {
            itemsSelected++;
            totalWeight += inst.items[i].weight;
            totalProfit += inst.items[i].profit;
        }
    }
    
    cout << "Items seleccionados: " << itemsSelected << "/" << inst.num_items << endl;
    cout << "Peso total: " << totalWeight << "/" << inst.capacity << endl;
    cout << "Ganancia total: " << totalProfit << endl;
    
    // Calcular objetivo
    double objective = calculateObjective(inst, tour, pickingPlan);
    cout << "Función objetivo: " << objective << endl;
}

// ========================================
// MAIN - PROBAR TUS ALGORITMOS
// ========================================

int main() {
    TTPInstance instance;
    
    // Leer archivo TTP
    string filename = "eil76-TTP.ttp";
    cout << "Leyendo archivo: " << filename << endl;
    
    if (!readTTPFile(filename, instance)) {
        return 1;
    }
    
    // Mostrar información de la instancia
    printInstanceInfo(instance);
    
    cout << "\n" << string(50, '=') << endl;
    cout << "PROBANDO ALGORITMOS GREEDY" << endl;
    cout << string(50, '=') << endl;
    
    // Probar diferentes combinaciones
    
    // 1. Tour greedy + selección por ratio
    vector<int> tour1 = greedyNearestNeighbor(instance);
    vector<int> picking1 = greedyKnapsackByRatio(instance, tour1);
    printSolution(instance, tour1, picking1, "Vecino Más Cercano + Ratio P/W");
    
    // 2. Tour greedy + selección por profit
    vector<int> tour2 = greedyNearestNeighbor(instance);
    vector<int> picking2 = greedyKnapsackByProfit(instance, tour2);
    printSolution(instance, tour2, picking2, "Vecino Más Cercano + Mayor Profit");
    
    // 3. Tour simple + selección por ratio
    vector<int> tour3(instance.dimension);
    for (int i = 0; i < instance.dimension; i++) tour3[i] = i;
    vector<int> picking3 = greedyKnapsackByRatio(instance, tour3);
    printSolution(instance, tour3, picking3, "Tour Simple + Ratio P/W");
    
    cout << "\n" << string(50, '=') << endl;
    cout << "AQUÍ PUEDES AGREGAR TUS PROPIAS HEURÍSTICAS" << endl;
    cout << string(50, '=') << endl;
    
    return 0;
}