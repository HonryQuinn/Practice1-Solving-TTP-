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
    
    TTPExperiment experiment(instance);
    
    experiment.addHeuristic(new LocalSearch2Opt(instance));
    
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 0.3));
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 0.5));
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 1.0));
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 2.0));
    
    // experiment.addHeuristic(new SequentialNoItems(instance));
    // experiment.addHeuristic(new NearestNeighborGreedy(instance));
    // experiment.addHeuristic(new RandomTourGreedy(instance));
    // experiment.addHeuristic(new HighProfitPicking(instance));
    
    experiment.addHeuristic(new HillClimbingPicking(instance));

    experiment.addHeuristic(new LNS_TTP(instance, 15, 100));
    experiment.addHeuristic(new LNS_TTP(instance, 30, 150)); 
    experiment.addHeuristic(new LNS_TTP(instance, 50, 200));

    experiment.addHeuristic(new VNS_TTP(instance, 100, 5));
    experiment.addHeuristic(new VNS_TTP(instance, 200, 7)); 
    experiment.addHeuristic(new VNS_TTP(instance, 300, 10)); 




    experiment.runAll();
    
    return 0;
}