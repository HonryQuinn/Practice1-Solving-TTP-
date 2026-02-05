#ifndef TTP_BASE_H
#define TTP_BASE_H

#include "reader.cpp"
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

struct TTPSolution {
    vector<int> tour;       
    vector<int> pickingPlan;   
    double objective;         
    double profit;           
    double time;              
    int weight;                 
    
    TTPSolution() : objective(-numeric_limits<double>::infinity()), 
                    profit(0), time(0), weight(0) {}
    
    bool isValid(const TTPInstance& inst) const {
        return weight <= inst.capacity && tour.size() == inst.dimension;
    }
    
};


class TTPHeuristic {
protected:
    const TTPInstance& instance;
    
public:
    TTPHeuristic(const TTPInstance& inst) : instance(inst) {}  //cons
    virtual ~TTPHeuristic() {} // des
    
    virtual TTPSolution solve() = 0;
    
    virtual string getName() const = 0;
    
    void evaluateSolution(TTPSolution& sol) {
        sol.profit = 0.0;
        sol.time = 0.0;
        sol.weight = 0;
        
        for (int i = 0; i < instance.num_items; i++) {
            if (sol.pickingPlan[i] == 1) {
                sol.profit += instance.items[i].profit;
                sol.weight += instance.items[i].weight;
            }
        }

        double nu = (instance.max_speed - instance.min_speed) / instance.capacity;

        int currentWeight = 0;
        for (int i = 0; i < instance.dimension; i++) {
            int from = sol.tour[i];
            int to = sol.tour[(i + 1) % instance.dimension];
            
            double velocity = instance.max_speed - nu * currentWeight;
            sol.time += instance.distances[from][to] / velocity;
            
            for (int k = 0; k < instance.num_items; k++) {
                if (sol.pickingPlan[k] == 1 && instance.items[k].node == to) {
                    currentWeight += instance.items[k].weight;
                }
            }
        }
        
        sol.objective = sol.profit - sol.time * instance.renting_ratio;
    }
    
    // FUNCIONES AUXILIARES PARA CREAR TOURS Y PICKING PLANS

    vector<int> createSequentialTour() {
        vector<int> tour(instance.dimension);
        for (int i = 0; i < instance.dimension; i++) {
            tour[i] = i;
        }
        return tour;
    }
    
    vector<int> createRandomTour() {
        vector<int> tour = createSequentialTour();
        random_shuffle(tour.begin() + 1, tour.end()); // mantener ciudad 0 al inicio
        return tour;
    }
    
    vector<int> createNearestNeighborTour(int start = 0) {
        vector<int> tour;
        vector<bool> visited(instance.dimension, false);
        
        int current = start;
        tour.push_back(current);
        visited[current] = true;
        
        for (int i = 1; i < instance.dimension; i++) {
            double minDist = numeric_limits<double>::infinity();
            int nearest = -1;
            
            for (int j = 0; j < instance.dimension; j++) {
                if (!visited[j] && instance.distances[current][j] < minDist) {
                    minDist = instance.distances[current][j];
                    nearest = j;
                }
            }
            
            tour.push_back(nearest);
            visited[nearest] = true;
            current = nearest;
        }
        
        return tour;
    }
    
    vector<int> createEmptyPickingPlan() {
        return vector<int>(instance.num_items, 0);
    }
    
    vector<int> createGreedyPickingPlan(const vector<int>& tour) {
        vector<int> pickingPlan(instance.num_items, 0);
        
        vector<pair<double, int>> itemRatios;
        for (int i = 0; i < instance.num_items; i++) {
            double ratio = (double)instance.items[i].profit / instance.items[i].weight;
            itemRatios.push_back({ratio, i});
        }
        sort(itemRatios.rbegin(), itemRatios.rend());

        int currentWeight = 0;
        for (auto& p : itemRatios) {
            int itemIdx = p.second;
            if (currentWeight + instance.items[itemIdx].weight <= instance.capacity) {
                pickingPlan[itemIdx] = 1;
                currentWeight += instance.items[itemIdx].weight;
            }
        }
        
        return pickingPlan;
    }
};

class HillClimbingPicking : public TTPHeuristic {
private:
    // mejora el picking plan haciendo flip de items (one-flip neighborhood)
    bool improvePicking(TTPSolution& sol) {
        bool improved = false;
        
        for (int i = 0; i < instance.num_items; i++) {
            sol.pickingPlan[i] = 1 - sol.pickingPlan[i];
            
            double oldObj = sol.objective;
            evaluateSolution(sol);
            
            if (sol.isValid(instance) && sol.objective > oldObj) {
                improved = true;
            } else {
                sol.pickingPlan[i] = 1 - sol.pickingPlan[i];
                sol.objective = oldObj;
            }
        }
        return improved;
    }

public:
    HillClimbingPicking(const TTPInstance& inst) : TTPHeuristic(inst) {}
    
    string getName() const override {
        return "Nearest Neighbor Tour + Hill Climbing Picking";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        
        sol.tour = createNearestNeighborTour(0);
        
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        
        int iterations = 0;
        while (improvePicking(sol) && iterations < 100) {
            iterations++;
        }
        
        return sol;
    }
};


class TTPExperiment {
private:
    const TTPInstance& instance;
    vector<TTPHeuristic*> heuristics;
    
public:
    TTPExperiment(const TTPInstance& inst) : instance(inst) {}   // constructor
    
    ~TTPExperiment() {                        // destructor
        for (auto h : heuristics) {
            delete h;
        }
    }
    
    void addHeuristic(TTPHeuristic* heuristic) {
        heuristics.push_back(heuristic);
    }
    
    void runAll() {
        cout << "  Experimento TTP" << endl;
        cout << "Instancia: " << instance.name << endl;
        cout << "Ciudades: " << instance.dimension << endl;
        cout << "Items: " << instance.num_items << endl;
        cout << "Capacidad: " << instance.capacity << endl;
        
        TTPSolution bestSolution;
        string bestHeuristic;
        
        for (auto heuristic : heuristics) {
            cout << " Ejecutando: " << heuristic->getName() << endl;
            
            TTPSolution solution = heuristic->solve();
            
            cout << "  Objetivo: " << solution.objective << endl;
            cout << "  Ganancia: " << solution.profit << endl;
            cout << "  Tiempo: " << solution.time << endl;
            cout << "  Peso usado: " << solution.weight << "/" << instance.capacity << endl;
            cout << "  Válida: " << (solution.isValid(instance) ? "Sí" : "No") << endl;
            cout << endl;
            
            if (solution.objective > bestSolution.objective) {
                bestSolution = solution;
                bestHeuristic = heuristic->getName();
            }
        }
        
        cout << " Mejor solución: " << endl;
        cout << "Heurística: " << bestHeuristic << endl;
        cout << "Objetivo: " << bestSolution.objective << endl;
        cout << "Ganancia: " << bestSolution.profit << endl;
        cout << "Tiempo: " << bestSolution.time << endl;
        cout << "Peso: " << bestSolution.weight << endl;
    }
};

#endif