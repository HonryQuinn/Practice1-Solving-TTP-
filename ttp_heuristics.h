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

// ============================================================================
// HEURÍSTICAS OPTIMIZADAS CON 2-OPT + OR-OPT
// ============================================================================

class OptimizedTTPHeuristic : public TTPHeuristic {
protected:
    // 2-Opt limitado: solo revisa vecinos cercanos
    bool improve2OptLimited(TTPSolution& sol, int maxNeighbors = 20) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            // Limitar j para reducir el espacio de búsqueda
            int jMax = min(i + maxNeighbors, n);
            
            for (int j = i + 1; j < jMax; j++) {
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
    
    // Or-Opt: mueve segmentos de 1, 2, o 3 ciudades
    bool improveOrOpt(TTPSolution& sol, int maxSegmentSize = 3) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int segSize = 1; segSize <= maxSegmentSize; segSize++) {
            for (int i = 1; i < n - segSize; i++) {
                vector<int> segment(sol.tour.begin() + i, sol.tour.begin() + i + segSize);
                
                for (int j = 1; j < n - segSize; j++) {
                    if (j >= i && j < i + segSize) continue;
                    
                    vector<int> newTour = sol.tour;
                    newTour.erase(newTour.begin() + i, newTour.begin() + i + segSize);
                    
                    int insertPos = (j > i) ? j - segSize : j;
                    newTour.insert(newTour.begin() + insertPos, segment.begin(), segment.end());
                    
                    double oldObj = sol.objective;
                    vector<int> oldTour = sol.tour;
                    
                    sol.tour = newTour;
                    evaluateSolution(sol);
                    
                    if (sol.objective > oldObj) {
                        improved = true;
                        goto next_segment;
                    } else {
                        sol.tour = oldTour;
                        sol.objective = oldObj;
                    }
                }
            }
            next_segment:;
        }
        return improved;
    }
    
    // Mejora híbrida: 2-Opt limitado + Or-Opt
    void hybridImprovement(TTPSolution& sol, int maxIter = 3) {
        for (int iter = 0; iter < maxIter; iter++) {
            bool improved = false;
            
            if (improve2OptLimited(sol, 15)) {
                improved = true;
                sol.pickingPlan = createGreedyPickingPlan(sol.tour);
                evaluateSolution(sol);
            }
            
            if (improveOrOpt(sol, 2)) {
                improved = true;
                sol.pickingPlan = createGreedyPickingPlan(sol.tour);
                evaluateSolution(sol);
            }
            
            if (!improved) break;
        }
    }

public:
    OptimizedTTPHeuristic(const TTPInstance& inst) : TTPHeuristic(inst) {}
};

// HEURÍSTICA D: Mejora local con 2-opt en tour
class LocalSearch2Opt : public OptimizedTTPHeuristic {
public:
    LocalSearch2Opt(const TTPInstance& inst) : OptimizedTTPHeuristic(inst) {}
    
    string getName() const override {
        return "2-Opt Local Search + Greedy Picking";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createNearestNeighborTour(0);
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        
        hybridImprovement(sol, 5);
        
        return sol;
    }
};

// HEURÍSTICA F: Probabilistic Nearest Neighbor + 2-Opt Local Search
class ProbabilisticNearestNeighbor2Opt : public OptimizedTTPHeuristic {
private:
    double temperature;
    
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
    
public:
    ProbabilisticNearestNeighbor2Opt(const TTPInstance& inst, double temp = 0.5) 
        : OptimizedTTPHeuristic(inst), temperature(temp) {
        srand(time(0));
    }
    
    string getName() const override {
        return "Probabilistic NN + 2-Opt+OrOpt (T=" + 
               to_string(temperature) + ")";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createProbabilisticNearestNeighborTour(0);
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        
        int iterations = 0;
        while (improve2OptLimited(sol, 15) && iterations < 100) {
            iterations++;
            sol.pickingPlan = createGreedyPickingPlan(sol.tour);
            evaluateSolution(sol);
        }
        
        return sol;
    }
};

class BalancedTTPHeuristic : public TTPHeuristic {
protected:
    // Picking adaptativo: ajusta capacidad según longitud del tour
    vector<int> createAdaptivePickingPlan(const vector<int>& tour, double fillRatio = 0.70) {
        vector<int> pickingPlan(instance.num_items, 0);
        
        // Calcular distancia total del tour
        double distanciaTotal = 0;
        for (int i = 0; i < instance.dimension; i++) {
            int from = tour[i];
            int to = tour[(i + 1) % instance.dimension];
            distanciaTotal += instance.distances[from][to];
        }
        
        // Ajustar capacidad según longitud del tour
        double tourFactor = 1.0;
        if (distanciaTotal > 50000) tourFactor = 0.6;
        else if (distanciaTotal > 45000) tourFactor = 0.7;
        else if (distanciaTotal > 40000) tourFactor = 0.8;
        
        // CORRECCIÓN CRÍTICA: asegurar que capacidadObjetivo nunca exceda instance.capacity
        int capacidadObjetivo = min((int)(instance.capacity * fillRatio * tourFactor), instance.capacity);
        
        // Greedy por ratio ganancia/peso
        vector<pair<double, int>> itemRatios;
        for (int i = 0; i < instance.num_items; i++) {
            double ratio = (double)instance.items[i].profit / instance.items[i].weight;
            itemRatios.push_back({ratio, i});
        }
        sort(itemRatios.rbegin(), itemRatios.rend());
        
        int currentWeight = 0;
        for (auto& p : itemRatios) {
            int itemIdx = p.second;
            // CORRECCIÓN: doble verificación contra capacidad real
            if (currentWeight + instance.items[itemIdx].weight <= capacidadObjetivo &&
                currentWeight + instance.items[itemIdx].weight <= instance.capacity) {
                pickingPlan[itemIdx] = 1;
                currentWeight += instance.items[itemIdx].weight;
            }
        }
        
        return pickingPlan;
    }
    
    // Mejora del picking evaluando objetivo completo
    bool improvePickingWithObjective(TTPSolution& sol, int maxFlips = 50) {
        bool improved = false;
        
        // CORRECCIÓN: Asegurar que empezamos con solución válida
        if (!sol.isValid(instance)) {
            evaluateSolution(sol);
            return false;
        }
        
        for (int flip = 0; flip < maxFlips; flip++) {
            int bestItem = -1;
            double bestImprovement = 0;
            double currentObj = sol.objective;
            
            for (int i = 0; i < instance.num_items; i++) {
                int originalValue = sol.pickingPlan[i];
                sol.pickingPlan[i] = 1 - sol.pickingPlan[i];
                
                evaluateSolution(sol);
                
                // CORRECCIÓN: Solo considerar si mejora Y es válida
                if (sol.isValid(instance) && sol.objective > currentObj) {
                    double improvement = sol.objective - currentObj;
                    if (improvement > bestImprovement) {
                        bestImprovement = improvement;
                        bestItem = i;
                    }
                }
                
                sol.pickingPlan[i] = originalValue;
            }
            
            if (bestItem != -1) {
                sol.pickingPlan[bestItem] = 1 - sol.pickingPlan[bestItem];
                evaluateSolution(sol);
                
                // CORRECCIÓN CRÍTICA: verificar que sigue siendo válida después del cambio
                if (!sol.isValid(instance)) {
                    sol.pickingPlan[bestItem] = 1 - sol.pickingPlan[bestItem];
                    evaluateSolution(sol);
                    break;
                }
                improved = true;
            } else {
                break;
            }
        }
        
        // CORRECCIÓN FINAL: asegurar que terminamos con solución válida
        if (!sol.isValid(instance)) {
            evaluateSolution(sol);
        }
        
        return improved;
    }
    
    // 2-Opt limitado (igual que OptimizedTTPHeuristic)
    bool improve2OptLimited(TTPSolution& sol, int maxNeighbors = 20) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            int jMax = min(i + maxNeighbors, n);
            
            for (int j = i + 1; j < jMax; j++) {
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
    
    // Mejora conjunta: tour + picking
    void jointImprovement(TTPSolution& sol, int maxIter = 3) {
        for (int iter = 0; iter < maxIter; iter++) {
            bool improved = false;
            
            if (improve2OptLimited(sol, 15)) {
                improved = true;
            }
            
            if (improvePickingWithObjective(sol, 20)) {
                improved = true;
            }
            
            if (!improved) break;
        }
    }

public:
    BalancedTTPHeuristic(const TTPInstance& inst) : TTPHeuristic(inst) {}
};

// Hill Climbing mejorado con picking adaptativo
class ImprovedHillClimbing : public BalancedTTPHeuristic {
public:
    ImprovedHillClimbing(const TTPInstance& inst) : BalancedTTPHeuristic(inst) {}
    
    string getName() const override {
        return "Improved Hill Climbing (Adaptive Picking 75%)";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createNearestNeighborTour(0);
        sol.pickingPlan = createAdaptivePickingPlan(sol.tour, 0.75);
        evaluateSolution(sol);
        
        jointImprovement(sol, 5);
        
        return sol;
    }
};

// 2-Opt con picking balanceado
class Balanced2Opt : public BalancedTTPHeuristic {
public:
    Balanced2Opt(const TTPInstance& inst) : BalancedTTPHeuristic(inst) {}
    
    string getName() const override {
        return "2-Opt + Balanced Picking (70%)";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createNearestNeighborTour(0);
        
        // IMPORTANTE: Inicializar picking ANTES de 2-Opt
        sol.pickingPlan = createAdaptivePickingPlan(sol.tour, 0.70);
        evaluateSolution(sol);
        
        // Mejorar tour
        improve2OptLimited(sol, 20);
        
        // Re-optimizar picking
        sol.pickingPlan = createAdaptivePickingPlan(sol.tour, 0.70);
        evaluateSolution(sol);
        
        // Mejora conjunta
        jointImprovement(sol, 5);
        
        return sol;
    }
};

// LNS con picking balanceado
class BalancedLNS : public BalancedTTPHeuristic {
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

public:
    BalancedLNS(const TTPInstance& inst, int k = 10, int maxIter = 30) 
        : BalancedTTPHeuristic(inst), destroySize(k), maxIterations(maxIter) {
        srand(time(0));
    }
    
    string getName() const override {
        return "Balanced LNS (destroy=" + to_string(destroySize) + 
               ", iter=" + to_string(maxIterations) + ")";
    }
    
    TTPSolution solve() override {
        TTPSolution best;
        best.tour = createNearestNeighborTour(0);
        best.pickingPlan = createAdaptivePickingPlan(best.tour, 0.70);
        evaluateSolution(best);
        
        TTPSolution current = best;
        int noImproveCount = 0;
        
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
            current.pickingPlan = createAdaptivePickingPlan(current.tour, 0.70);
            evaluateSolution(current);
            
            jointImprovement(current, 2);
            
            if (current.objective > best.objective) {
                best = current;
                noImproveCount = 0;
            } else {
                noImproveCount++;
                if (noImproveCount >= 5) {
                    current = best;
                    noImproveCount = 0;
                }
            }
        }
        
        return best;
    }
};

// VNS con picking balanceado
class BalancedVNS : public BalancedTTPHeuristic {
private:
    int maxIterations;
    int kmax;
    
    void shaking(TTPSolution& sol, int k) {
        for (int i = 0; i < k; i++) {
            int pos1 = 1 + rand() % (sol.tour.size() - 1);
            int pos2 = 1 + rand() % (sol.tour.size() - 1);
            swap(sol.tour[pos1], sol.tour[pos2]);
        }
    }

public:
    BalancedVNS(const TTPInstance& inst, int maxIter = 50, int k_max = 5)
        : BalancedTTPHeuristic(inst), maxIterations(maxIter), kmax(k_max) {
        srand(time(0));
    }
    
    string getName() const override {
        return "Balanced VNS (kmax=" + to_string(kmax) + 
               ", iter=" + to_string(maxIterations) + ")";
    }
    
    TTPSolution solve() override {
        TTPSolution best;
        best.tour = createNearestNeighborTour(0);
        best.pickingPlan = createAdaptivePickingPlan(best.tour, 0.70);
        evaluateSolution(best);
        
        int iter = 0;
        int k = 1;
        int noImproveCount = 0;
        
        while (iter < maxIterations) {
            TTPSolution current = best;
            
            shaking(current, k);
            current.pickingPlan = createAdaptivePickingPlan(current.tour, 0.70);
            evaluateSolution(current);
            
            jointImprovement(current, 2);
            
            if (current.objective > best.objective) {
                best = current;
                k = 1;
                noImproveCount = 0;
            } else {
                k++;
                noImproveCount++;
                
                if (k > kmax) k = 1;
                if (noImproveCount >= maxIterations / 4) break;
            }
            
            iter++;
        }
        
        return best;
    }
};

#endif