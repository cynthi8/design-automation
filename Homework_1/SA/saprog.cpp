#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "SA_algorithm.hpp"

using namespace std;

int MilisecondsPassed(chrono::system_clock::time_point start) {
    // Helper function for time passed
    auto end = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int main(int argc, char* argv[]) {
    if(argc != 3)
    {
        cout << "Error: wrong number of inputs.\n" << "saprog InputFile OutputFile" << endl;
        exit(1);
    }
    vector<string> vargv(argv, argv + argc);

    auto start = chrono::system_clock::now();
    Graph myGraph(vargv[1]);
    myGraph.SimulatedAnealing(
        20000,
        .1,
        .975,
        myGraph.getNodes()*10
    );
    int msPassed = MilisecondsPassed(start);

    cout << "Netlist: " << vargv[1] << 
    " Solution Found = " << myGraph.getCost() << 
    " in " << msPassed << "ms" << endl; 

    myGraph.PrintSolutionToFile(vargv[2]);

    return 0;
}