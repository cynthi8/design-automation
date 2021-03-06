#include <iostream>
#include <chrono>
#include <thread>

#include "SA_algorithm.hpp"

using namespace std;

struct Netlist {
    string fileName;
    int optimalCost;
};

struct Performance {
    int cost;
    int miliseconds;
};

vector<Netlist> devNetlists {
    {"../development_netlists/bench_2.net", 1}, 
    {"../development_netlists/bench_4.net", 3}, 
    {"../development_netlists/bench_6.net", 6}, 
    {"../development_netlists/bench_11.net", 25}, 
    {"../development_netlists/bench_12.net", 35}, 
    {"../development_netlists/bench_16.net", 150}
};

vector<Netlist> benchNetlists {
    {"../Benchmarks/b_100_500", 17},
    {"../Benchmarks/b_500_20000", 63},
    {"../Benchmarks/b_1000_20000", 34},
    {"../Benchmarks/b_10000_100000", 163},
    {"../Benchmarks/b_50000_400000", 665},
    {"../Benchmarks/b_100000_500000", 2761},
    {"../Benchmarks/b_100000_2000000", 12450},
    {"../Benchmarks/b_200000_2000000", 11764},
    {"../Benchmarks/b_250000_1000000", 179},
    {"../Benchmarks/b_500000_3000000", 21220}
};

int MilisecondsPassed(chrono::system_clock::time_point start) {
    // Helper function for time passed
    auto end = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

void PrintPerformance(Netlist netlist, Performance performance) {
    cout << netlist.fileName << 
    ": Optimal Cost = " << netlist.optimalCost <<
    " Solution Found = " << performance.cost << 
    " in " << performance.miliseconds << "ms" << endl; 
}

void TestGraph() {
    bool pass = true;
    Graph myGraph(devNetlists[0].fileName);
    pass = pass && (myGraph.getEdgeWeight(10, 2) == 0);
    pass = pass && (myGraph.getEdgeWeight(2, 3) == 1);
    pass = pass && (myGraph.getEdgeWeight(9, 5) == 2);
    pass = pass && (myGraph.getEdgeWeight(5, 9) == 2);
    if (pass == false) {
        cout << "Fail: TestGraph" << endl;
    }
}

void TestNetlist(Netlist netlist) {
    static int id = 1;
    auto start = chrono::system_clock::now();
    Graph myGraph(netlist.fileName);
    float initialTemperature = myGraph.CalculateInitialTemperature(.99);
    myGraph.SimulatedAnealing(
        initialTemperature,
        .1,
        .975,
        myGraph.getNodes()*10
    );
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    int msPassed = MilisecondsPassed(start);
    Performance performance {myGraph.getCost(), msPassed};
    PrintPerformance(netlist, performance);
    myGraph.PrintLogToFile(netlist.fileName + ".log");
    myGraph.PrintSolutionToFile("Results/R" + to_string(id));
    id++;
}

void TestNetlistVector(vector<Netlist> & netlists) {
    for(auto it = netlists.begin(); it != netlists.end(); it++) {
        TestNetlist(*it);
    }
}

int main() {
    //TestGraph();
    //TestNetlistVector(devNetlists);
    TestNetlistVector(benchNetlists);
    //TestNetlist(benchNetlists[9]);
    return 0;
}