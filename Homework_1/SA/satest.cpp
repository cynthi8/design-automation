#include <iostream>

#include "SA_algorithm.hpp"

using namespace std;


struct Netlist {
    string fileName;
    int optimalCost;
};

vector<Netlist> devNetlists {
    {"../development_netlists/bench_2.net", 1}, 
    {"../development_netlists/bench_4.net", 3}, 
    {"../development_netlists/bench_6.net", 6}, 
    {"../development_netlists/bench_11.net", 25}, 
    {"../development_netlists/bench_12.net", 35}, 
    {"../development_netlists/bench_16.net", 150}
};

void testGraph() {
    bool pass = true;
    Graph myGraph(devNetlists[0].fileName);
    pass = pass && (myGraph.getEdgeWeight(10, 2) == 0);
    pass = pass && (myGraph.getEdgeWeight(2, 3) == 1);
    pass = pass && (myGraph.getEdgeWeight(9, 5) == 2);
    pass = pass && (myGraph.getEdgeWeight(5, 9) == 2);
    if (pass == false) {
        cout << "Fail: testGraph" << endl;
    }
}

void testSingleSA() {
    Graph myGraph(devNetlists[0].fileName);
    myGraph.SimulatedAnealing(100, 1, .95, 1);
    if (myGraph.getCost() != devNetlists[0].optimalCost) {
        cout << "Fail: testSA. Cost = " << myGraph.getCost() << endl;
        myGraph.m_log.PrintLog();
        myGraph.PrintSolution();
    }
}

void testAllSA() {
    for(auto it = devNetlists.begin(); it != devNetlists.end(); it++) {
        Graph myGraph((*it).fileName);
        myGraph.SimulatedAnealing(100, 1, .95, 10000);
        cout << (*it).fileName << ": Optimal Cost = " << (*it).optimalCost << " Solution Found = " << myGraph.getCost() << endl; 
    }
}

int main() {
    testGraph();
    testSingleSA();
    testAllSA();

    return 0;
}