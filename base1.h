#ifndef TTP_BASE_H
#define TTP_BASE_H

#include "reader.cpp"
#include <vector>
#include <string>
#include <limits>
#include <algorithm>
#include <cmath>

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
        return weight <= inst.capacity && tour.size() == (size_t)inst.dimension;
    }
};

class TTPHeuristic {
protected:
    const TTPInstance& instance;
    
public:
    TTPHeuristic(const TTPInstance& inst) : instance(inst) {}
    virtual ~TTPHeuristic() {}
    
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

        if (sol.weight > instance.capacity) {
            sol.objective = -1e9;
            sol.time = 1e9;
            return;
        }

        double nu = (instance.max_speed - instance.min_speed) / instance.capacity;

        int currentWeight = 0;
        for (int i = 0; i < instance.dimension; i++) {
            int from = sol.tour[i];
            int to = sol.tour[(i + 1) % instance.dimension];
            
            double velocity = instance.max_speed - nu * currentWeight;

            if (velocity < instance.min_speed) {
                velocity = instance.min_speed;
            }

            sol.time += instance.distances[from][to] / velocity;
            
            for (int k = 0; k < instance.num_items; k++) {
                if (sol.pickingPlan[k] == 1 && instance.items[k].node == to) {
                    currentWeight += instance.items[k].weight;
                }
            }
        }
        
        sol.objective = sol.profit - sol.time * instance.renting_ratio;
    }
    
    vector<int> createSequentialTour() {
        vector<int> tour(instance.dimension);
        for (int i = 0; i < instance.dimension; i++) {
            tour[i] = i;
        }
        return tour;
    }
    
    vector<int> createRandomTour() {
        vector<int> tour = createSequentialTour();
        random_shuffle(tour.begin() + 1, tour.end());
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
                evaluateSolution(sol);
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

// ============================================================
// ESTADÍSTICAS Y EXPERIMENTOS CON MÚLTIPLES EJECUCIONES
// ============================================================

struct HeuristicStats {
    string name;
    double avg_objective;
    double avg_profit;
    double avg_time;
    double avg_weight;
    double best_objective;
    double worst_objective;
    double std_dev_objective;
    
    HeuristicStats() : avg_objective(0), avg_profit(0), avg_time(0), 
                       avg_weight(0), best_objective(-1e9), 
                       worst_objective(1e9), std_dev_objective(0) {}
};

class TTPExperiment {
private:
    const TTPInstance& instance;
    vector<TTPHeuristic*> heuristics;
    int num_runs;
    
    double calculateStdDev(const vector<double>& values, double mean) {
        double sum = 0.0;
        for (double val : values) {
            sum += (val - mean) * (val - mean);
        }
        return sqrt(sum / values.size());
    }
    
public:
    TTPExperiment(const TTPInstance& inst, int runs = 1) 
        : instance(inst), num_runs(runs) {}
    
    ~TTPExperiment() {
        for (auto h : heuristics) {
            delete h;
        }
    }
    
    void addHeuristic(TTPHeuristic* heuristic) {
        heuristics.push_back(heuristic);
    }
    
    void runAll() {
        cout << "\n========================================" << endl;
        cout << "       EXPERIMENTO TTP" << endl;
        cout << "========================================" << endl;
        cout << "Instancia: " << instance.name << endl;
        cout << "Ciudades: " << instance.dimension << endl;
        cout << "Items: " << instance.num_items << endl;
        cout << "Capacidad: " << instance.capacity << endl;
        cout << "Ejecuciones por heuristica: " << num_runs << endl;
        cout << "========================================\n" << endl;
        
        vector<HeuristicStats> allStats;
        TTPSolution globalBest;
        string globalBestHeuristic;
        
        for (auto heuristic : heuristics) {
            cout << ">>> Ejecutando: " << heuristic->getName() << " <<<" << endl;
            
            HeuristicStats stats;
            stats.name = heuristic->getName();
            
            vector<double> objectives;
            vector<double> profits;
            vector<double> times;
            vector<double> weights;
            
            // Ejecutar múltiples veces
            for (int run = 1; run <= num_runs; run++) {
                if (num_runs > 1) {
                    cout << "  [Run " << run << "/" << num_runs << "] ";
                }
                
                TTPSolution solution = heuristic->solve();
                
                objectives.push_back(solution.objective);
                profits.push_back(solution.profit);
                times.push_back(solution.time);
                weights.push_back(solution.weight);
                
                if (num_runs > 1) {
                    cout << "Objetivo: " << solution.objective << endl;
                }
                
                // Actualizar mejor global
                if (solution.objective > globalBest.objective) {
                    globalBest = solution;
                    globalBestHeuristic = heuristic->getName();
                }
                
                // Actualizar mejor/peor de esta heurística
                if (solution.objective > stats.best_objective) {
                    stats.best_objective = solution.objective;
                }
                if (solution.objective < stats.worst_objective) {
                    stats.worst_objective = solution.objective;
                }
            }
            
            // Calcular promedios
            for (double val : objectives) stats.avg_objective += val;
            for (double val : profits) stats.avg_profit += val;
            for (double val : times) stats.avg_time += val;
            for (double val : weights) stats.avg_weight += val;
            
            stats.avg_objective /= num_runs;
            stats.avg_profit /= num_runs;
            stats.avg_time /= num_runs;
            stats.avg_weight /= num_runs;
            
            // Calcular desviación estándar
            if (num_runs > 1) {
                stats.std_dev_objective = calculateStdDev(objectives, stats.avg_objective);
            }
            
            allStats.push_back(stats);
            
            // Mostrar resultados de esta heurística
            cout << "\n  RESULTADOS:" << endl;
            if (num_runs > 1) {
                cout << "    Objetivo Promedio: " << stats.avg_objective 
                     << " (±" << stats.std_dev_objective << ")" << endl;
                cout << "    Mejor: " << stats.best_objective << endl;
                cout << "    Peor: " << stats.worst_objective << endl;
                cout << "    Ganancia Promedio: " << stats.avg_profit << endl;
                cout << "    Tiempo Promedio: " << stats.avg_time << endl;
                cout << "    Peso Promedio: " << stats.avg_weight 
                     << "/" << instance.capacity << endl;
            } else {
                cout << "    Objetivo: " << stats.avg_objective << endl;
                cout << "    Ganancia: " << stats.avg_profit << endl;
                cout << "    Tiempo: " << stats.avg_time << endl;
                cout << "    Peso: " << stats.avg_weight 
                     << "/" << instance.capacity << endl;
            }
            cout << endl;
        }
        
        // Resumen final
        cout << "\n========================================" << endl;
        cout << "       RESUMEN FINAL" << endl;
        cout << "========================================" << endl;
        
        // Ordenar por mejor objetivo promedio
        sort(allStats.begin(), allStats.end(), 
             [](const HeuristicStats& a, const HeuristicStats& b) {
                 return a.avg_objective > b.avg_objective;
             });
        
        cout << "\nRanking por Objetivo Promedio:\n" << endl;
        for (size_t i = 0; i < allStats.size(); i++) {
            cout << (i+1) << ". " << allStats[i].name << endl;
            cout << "   Objetivo: " << allStats[i].avg_objective;
            if (num_runs > 1) {
                cout << " (±" << allStats[i].std_dev_objective << ")";
            }
            cout << endl;
        }
        
        cout << "\n========================================" << endl;
        cout << "MEJOR SOLUCION GLOBAL:" << endl;
        cout << "Heuristica: " << globalBestHeuristic << endl;
        cout << "Objetivo: " << globalBest.objective << endl;
        cout << "Ganancia: " << globalBest.profit << endl;
        cout << "Tiempo: " << globalBest.time << endl;
        cout << "Peso: " << globalBest.weight << "/" << instance.capacity << endl;
        cout << "========================================\n" << endl;
    }
};

#endif