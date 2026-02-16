#include "reader.cpp"
#include "base1.h"
#include "ttp_heuristics.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <archivo_ttp>" << endl;
        return 1;
    }
    
    TTPInstance instance;
    if (!readTTPFile(argv[1], instance)) {
        return 1;
    }
    
    printInstanceInfo(instance);
    
    cout << "EXPERIMENTO TTP - HEURÍSTICAS" << endl;
  
    TTPExperiment experiment(instance);
    
    // ========== HEURÍSTICAS OPTIMIZADAS (2-Opt + Or-Opt) ==========
    
    // experiment.addHeuristic(new LocalSearch2Opt(instance));
    
    // experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 0.3));
    // experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 0.5));
    // experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 1.0));
    // experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 2.0));
    
    // ========== HEURÍSTICAS COMENTADAS (desactivadas) ==========

    // experiment.addHeuristic(new SequentialNoItems(instance));
    // experiment.addHeuristic(new NearestNeighborGreedy(instance));
    // experiment.addHeuristic(new RandomTourGreedy(instance));
    // experiment.addHeuristic(new HighProfitPicking(instance));
    
    // ========== HEURÍSTICA BASELINE ==========
    
    experiment.addHeuristic(new HillClimbingPicking(instance));
    
    // ========== HEURÍSTICAS BALANCEADAS (Picking Adaptativo) ==========
    
    experiment.addHeuristic(new ImprovedHillClimbing(instance));
    experiment.addHeuristic(new Balanced2Opt(instance));
    
    // LNS Balanceado (diferentes configuraciones)
    experiment.addHeuristic(new BalancedLNS(instance, 10, 20));
    experiment.addHeuristic(new BalancedLNS(instance, 15, 30));
    experiment.addHeuristic(new BalancedLNS(instance, 20, 40));
    
    // VNS Balanceado (diferentes configuraciones)
    experiment.addHeuristic(new BalancedVNS(instance, 30, 3));
    experiment.addHeuristic(new BalancedVNS(instance, 50, 5));
    experiment.addHeuristic(new BalancedVNS(instance, 80, 7));
    
    // ========== EJECUTAR EXPERIMENTO ==========
    
    experiment.runAll();
    
    return 0;
}