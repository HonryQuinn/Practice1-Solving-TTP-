#ifndef TTP_READER_H
#define TTP_READER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;

struct Item {
    int profit;
    int weight;
    int node;  
};

struct TTPInstance {
    string name;
    int dimension;
    int num_items;
    int capacity;
    double min_speed;
    double max_speed;
    double renting_ratio;
    
    vector<pair<double, double>> coords;  // coordenadas de cada ciudad
    vector<vector<double>> distances;     // matriz de distancias
    vector<Item> items;                   // items disponibles
};

double calculateDistance(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return ceil(sqrt(dx * dx + dy * dy));
}

bool readTTPFile(const string& filename, TTPInstance& instance) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return false;
    }
    
    string line;
    
    // leer características
    while (getline(file, line)) {
        if (line.find("PROBLEM NAME:") != string::npos) {
            instance.name = line.substr(line.find(":") + 1);
        }
        else if (line.find("DIMENSION:") != string::npos) {
            sscanf(line.c_str(), "DIMENSION: %d", &instance.dimension);
        }
        else if (line.find("NUMBER OF ITEMS:") != string::npos) {
            sscanf(line.c_str(), "NUMBER OF ITEMS: %d", &instance.num_items);
        }
        else if (line.find("CAPACITY OF KNAPSACK:") != string::npos) {
            sscanf(line.c_str(), "CAPACITY OF KNAPSACK: %d", &instance.capacity);
        }
        else if (line.find("MIN SPEED:") != string::npos) {
            sscanf(line.c_str(), "MIN SPEED: %lf", &instance.min_speed);
        }
        else if (line.find("MAX SPEED:") != string::npos) {
            sscanf(line.c_str(), "MAX SPEED: %lf", &instance.max_speed);
        }
        else if (line.find("RENTING RATIO:") != string::npos) {
            sscanf(line.c_str(), "RENTING RATIO: %lf", &instance.renting_ratio);
        }
        else if (line.find("NODE_COORD_SECTION") != string::npos) {
            break;
        }
    }
    
    // coordenadas de nodos
    instance.coords.resize(instance.dimension);
    for (int i = 0; i < instance.dimension; i++) {
        int idx;
        double x, y;
        file >> idx >> x >> y;
        instance.coords[i] = {x, y};
    }
    
    // calcular matriz de distancias
    instance.distances.resize(instance.dimension, vector<double>(instance.dimension, 0.0));
    for (int i = 0; i < instance.dimension; i++) {
        for (int j = 0; j < instance.dimension; j++) {
            if (i != j) {
                instance.distances[i][j] = calculateDistance(
                    instance.coords[i].first, instance.coords[i].second,
                    instance.coords[j].first, instance.coords[j].second
                );
            }
        }
    }
    
    // buscar sección de items
    while (getline(file, line)) {
        if (line.find("ITEMS SECTION") != string::npos) {
            break;
        }
    }
    
    // leer las características de cada ítem
    instance.items.resize(instance.num_items);
    for (int i = 0; i < instance.num_items; i++) {
        int idx;
        file >> idx >> instance.items[i].profit >> instance.items[i].weight >> instance.items[i].node;
        instance.items[i].node--;  
    }
    
    file.close();
    return true;
}

void printInstanceInfo(const TTPInstance& instance) {
    cout << "=== Información de la Instancia TTP ===" << endl;
    cout << "Nombre: " << instance.name << endl;
    cout << "Ciudades: " << instance.dimension << endl;
    cout << "Items: " << instance.num_items << endl;
    cout << "Capacidad mochila: " << instance.capacity << endl;
    cout << "Velocidad mín: " << instance.min_speed << endl;
    cout << "Velocidad máx: " << instance.max_speed << endl;
    cout << "Ratio de alquiler: " << instance.renting_ratio << endl;
    cout << "\nPrimeras 5 ciudades:" << endl;
    for (int i = 0; i < min(5, instance.dimension); i++) {
        cout << "  Ciudad " << i << ": (" << instance.coords[i].first 
             << ", " << instance.coords[i].second << ")" << endl;
    }
    cout << "\nPrimeros 5 items:" << endl;
    for (int i = 0; i < min(5, instance.num_items); i++) {
        cout << "  Item " << i << ": profit=" << instance.items[i].profit 
             << ", weight=" << instance.items[i].weight 
             << ", ciudad=" << instance.items[i].node << endl;
    }
}

// función para calcular la función objetivo del TTP
double calculateObjective(const TTPInstance& inst, const vector<int>& tour, const vector<int>& pickingPlan) {
    double totalProfit = 0.0;
    double totalTime = 0.0;
    int currentWeight = 0;
    
    // calcular ganancia total
    for (int i = 0; i < inst.num_items; i++) {
        if (pickingPlan[i] == 1) {
            totalProfit += inst.items[i].profit;
        }
    }
    
    // calcular tiempo total del viaje
    double nu = (inst.max_speed - inst.min_speed) / inst.capacity;
    
    for (int i = 0; i < inst.dimension; i++) {
        int from = tour[i];
        int to = tour[(i + 1) % inst.dimension];
        
        // velocidad actual basada en el peso
        double velocity = inst.max_speed - nu * currentWeight;
        
        // tiempo para este segmento
        totalTime += inst.distances[from][to] / velocity;
        
        // actualizar peso después de visitar 'to'
        for (int k = 0; k < inst.num_items; k++) {
            if (pickingPlan[k] == 1 && inst.items[k].node == to) {
                currentWeight += inst.items[k].weight;
            }
        }
    }
    
    // objetivo = ganancia - costo de alquiler
    return totalProfit - totalTime * inst.renting_ratio;
}

#endif 