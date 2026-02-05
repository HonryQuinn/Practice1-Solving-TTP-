#ifndef TTP_HEURISTICS_H
#define TTP_HEURISTICS_H

#include "base1.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// /*
// // HEURÍSTICA A: Tour secuencial + Sin recoger items
// class SequentialNoItems : public TTPHeuristic {
// public:
//     SequentialNoItems(const TTPInstance& inst) : TTPHeuristic(inst) {}
//     
//     string getName() const override {
//         return "Sequential Tour + No Items";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createSequentialTour();
//         sol.pickingPlan = createEmptyPickingPlan();
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */

// /*
// // HEURÍSTICA B: Vecino más cercano + Picking greedy
// class NearestNeighborGreedy : public TTPHeuristic {
// public:
//     NearestNeighborGreedy(const TTPInstance& inst) : TTPHeuristic(inst) {}
//     
//     string getName() const override {
//         return "Nearest Neighbor + Greedy Picking";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createNearestNeighborTour(0);
//         sol.pickingPlan = createGreedyPickingPlan(sol.tour);
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */

// /*
// // HEURÍSTICA C: Tour aleatorio + Picking greedy
// class RandomTourGreedy : public TTPHeuristic {
// public:
//     RandomTourGreedy(const TTPInstance& inst) : TTPHeuristic(inst) {
//         srand(time(0));
//     }
//     
//     string getName() const override {
//         return "Random Tour + Greedy Picking";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createRandomTour();
//         sol.pickingPlan = createGreedyPickingPlan(sol.tour);
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */

// HEURÍSTICA D: Mejora local con 2-opt en tour
class LocalSearch2Opt : public TTPHeuristic {
private:
    bool improve2Opt(TTPSolution& sol) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                // Hacer swap 2-opt
                reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                
                double oldObj = sol.objective;
                evaluateSolution(sol);
                
                if (sol.objective > oldObj) {
                    improved = true;
                } else {
                    // Revertir si no mejoró
                    reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                    sol.objective = oldObj;
                }
            }
        }
        return improved;
    }
    
public:
    LocalSearch2Opt(const TTPInstance& inst) : TTPHeuristic(inst) {}
    
    string getName() const override {
        return "2-Opt Local Search + Greedy Picking";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createNearestNeighborTour(0);
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        
        // Aplicar mejora local
        int iterations = 0;
        while (improve2Opt(sol) && iterations < 100) {
            iterations++;
            // Recalcular picking plan después de cambiar tour
            sol.pickingPlan = createGreedyPickingPlan(sol.tour);
            evaluateSolution(sol);
        }
        
        return sol;
    }
};

// /*
// // HEURÍSTICA E: Picking basado en profit absoluto
// class HighProfitPicking : public TTPHeuristic {
// public:
//     HighProfitPicking(const TTPInstance& inst) : TTPHeuristic(inst) {}
//     
//     string getName() const override {
//         return "Nearest Neighbor + High Profit Picking";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createNearestNeighborTour(0);
//         
//         // Crear picking plan basado en profit absoluto
//         vector<pair<int, int>> itemsByProfit; // (profit, index)
//         for (int i = 0; i < instance.num_items; i++) {
//             itemsByProfit.push_back({instance.items[i].profit, i});
//         }
//         sort(itemsByProfit.rbegin(), itemsByProfit.rend());
//         
//         sol.pickingPlan = createEmptyPickingPlan();
//         int currentWeight = 0;
//         
//         for (auto& p : itemsByProfit) {
//             int itemIdx = p.second;
//             if (currentWeight + instance.items[itemIdx].weight <= instance.capacity) {
//                 sol.pickingPlan[itemIdx] = 1;
//                 currentWeight += instance.items[itemIdx].weight;
//             }
//         }
//         
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */

// HEURÍSTICA F: Probabilistic Nearest Neighbor + 2-Opt Local Search
class ProbabilisticNearestNeighbor2Opt : public TTPHeuristic {
private:
    double temperature;  // Parámetro para controlar la aleatoriedad
    
    vector<int> createProbabilisticNearestNeighborTour(int start = 0) {
        vector<int> tour;
        vector<bool> visited(instance.dimension, false);
        
        int current = start;
        tour.push_back(current);
        visited[current] = true;
        
        for (int i = 1; i < instance.dimension; i++) {
            vector<int> candidates;
            vector<double> distances;
            
            for (int j = 0; j < instance.dimension; j++) {
                if (!visited[j]) {
                    candidates.push_back(j);
                    distances.push_back(instance.distances[current][j]);
                }
            }
            
            vector<double> probabilities;
            double sumExp = 0.0;
            
            for (double dist : distances) {
                double expValue = exp(-dist / temperature);
                probabilities.push_back(expValue);
                sumExp += expValue;
            }
            
            for (double& prob : probabilities) {
                prob /= sumExp;
            }
            
            double randValue = ((double)rand() / RAND_MAX);
            double cumulative = 0.0;
            int selectedIdx = 0;
            
            for (int k = 0; k < probabilities.size(); k++) {
                cumulative += probabilities[k];
                if (randValue <= cumulative) {
                    selectedIdx = k;
                    break;
                }
            }
            
            int nextCity = candidates[selectedIdx];
            tour.push_back(nextCity);
            visited[nextCity] = true;
            current = nextCity;
        }
        
        return tour;
    }
    
    bool improve2Opt(TTPSolution& sol) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                
                double oldObj = sol.objective;
                evaluateSolution(sol);
                
                if (sol.objective > oldObj) {
                    improved = true;
                } else {
                    reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                    sol.objective = oldObj;
                }
            }
        }
        return improved;
    }
    
public:
    ProbabilisticNearestNeighbor2Opt(const TTPInstance& inst, double temp = 0.5) 
        : TTPHeuristic(inst), temperature(temp) {
        srand(time(0));
    }
    
    string getName() const override {
        return "Probabilistic Nearest Neighbor + 2-Opt (T=" + 
               to_string(temperature) + ")";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        
        sol.tour = createProbabilisticNearestNeighborTour(0);

        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        
        int iterations = 0;
        while (improve2Opt(sol) && iterations < 100) {
            iterations++;
            sol.pickingPlan = createGreedyPickingPlan(sol.tour);
            evaluateSolution(sol);
        }
        
        return sol;
    }
    
    double getTemperature() const {
        return temperature;
    }
};

//Limitar vecindario (el espacio de búsqueda), 2-opt + or-opt para mejorar la mejora xd y SA como opcion de 2-opt - PROBARTODO Y TENER TABLAS

class LNS_TTP : public TTPHeuristic {
private:
    int destroySize;
    int maxIterations;
    
    vector<int> destroyTour(const vector<int>& tour, int k) {
        vector<int> removed;
        vector<int> partial = tour;
        
        for (int i = 0; i < k; i++) {
            if (partial.size() <= 1) break;
            int idx = 1 + rand() % (partial.size() - 1);
            removed.push_back(partial[idx]);
            partial.erase(partial.begin() + idx);
        }
        
        return removed;
    }
    
    vector<int> reconstructTour(vector<int> partial, const vector<int>& removed) {
        for (int city : removed) {
            int bestPos = 1;
            double bestCost = numeric_limits<double>::infinity();
            
            for (int pos = 1; pos < partial.size(); pos++) {
                int prev = partial[pos - 1];
                int next = partial[pos];
                
                double cost = instance.distances[prev][city] + 
                             instance.distances[city][next] -
                             instance.distances[prev][next];
                
                if (cost < bestCost) {
                    bestCost = cost;
                    bestPos = pos;
                }
            }
            
            partial.insert(partial.begin() + bestPos, city);
        }
        
        return partial;
    }
    
    bool improve2Opt(TTPSolution& sol) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                
                double oldObj = sol.objective;
                evaluateSolution(sol);
                
                if (sol.objective > oldObj) {
                    improved = true;
                } else {
                    reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                    sol.objective = oldObj;
                }
            }
        }
        return improved;
    }

public:
    LNS_TTP(const TTPInstance& inst, int k = 10, int maxIter = 50) 
        : TTPHeuristic(inst), destroySize(k), maxIterations(maxIter) {
        srand(time(0));
    }
    
    string getName() const override {
        return "LNS (destroy=" + to_string(destroySize) + 
               ", iter=" + to_string(maxIterations) + ") + 2-Opt + Greedy";
    }
    
    TTPSolution solve() override {
        TTPSolution best;
        best.tour = createNearestNeighborTour(0);
        best.pickingPlan = createGreedyPickingPlan(best.tour);
        evaluateSolution(best);
        
        TTPSolution current = best;
        
        for (int iter = 0; iter < maxIterations; iter++) {
            vector<int> removed = destroyTour(current.tour, destroySize);
            
            vector<int> partial = current.tour;
            for (int city : removed) {
                auto it = find(partial.begin(), partial.end(), city);
                if (it != partial.end()) {
                    partial.erase(it);
                }
            }

            current.tour = reconstructTour(partial, removed);
            
            current.pickingPlan = createGreedyPickingPlan(current.tour);
            evaluateSolution(current);
            
            int localIter = 0;
            while (improve2Opt(current) && localIter < 5) {
                current.pickingPlan = createGreedyPickingPlan(current.tour);
                evaluateSolution(current);
                localIter++;
            }

            if (current.objective > best.objective) {
                best = current;
            }

            if (iter % 10 == 0) {
                current = best;
            }
        }
        
        return best;
    }
};

class VNS_TTP : public TTPHeuristic {
private:
    int maxIterations;
    int kmax;
    
    // shaking: perturbación aleatoria de tamaño k
    void shaking(TTPSolution& sol, int k) {
        for (int i = 0; i < k; i++) {
            int pos1 = 1 + rand() % (sol.tour.size() - 1);
            int pos2 = 1 + rand() % (sol.tour.size() - 1);
            swap(sol.tour[pos1], sol.tour[pos2]);
        }
    }
    
    bool improve2Opt(TTPSolution& sol) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                
                double oldObj = sol.objective;
                evaluateSolution(sol);
                
                if (sol.objective > oldObj) {
                    improved = true;
                } else {
                    reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                    sol.objective = oldObj;
                }
            }
        }
        return improved;
    }

public:
    VNS_TTP(const TTPInstance& inst, int maxIter = 100, int k_max = 5)
        : TTPHeuristic(inst), maxIterations(maxIter), kmax(k_max) {
        srand(time(0));
    }
    
    string getName() const override {
        return "VNS (kmax=" + to_string(kmax) + 
               ", iter=" + to_string(maxIterations) + ") + 2-Opt + Greedy";
    }
    
    TTPSolution solve() override {
        TTPSolution best;
        best.tour = createNearestNeighborTour(0);
        best.pickingPlan = createGreedyPickingPlan(best.tour);
        evaluateSolution(best);
        
        int iter = 0;
        int k = 1;
        
        while (iter < maxIterations) {

            TTPSolution current = best;
            shaking(current, k);
            
            current.pickingPlan = createGreedyPickingPlan(current.tour);
            evaluateSolution(current);
            
            int localIter = 0;
            while (improve2Opt(current) && localIter < 5) {
                current.pickingPlan = createGreedyPickingPlan(current.tour);
                evaluateSolution(current);
                localIter++;
            }
            
            if (current.objective > best.objective) {
                best = current;
                k = 1; 
            } else {
                k++;
                if (k > kmax) {
                    k = 1;
                }
            }
            
            iter++;
        }
        
        return best;
    }
};

#endif