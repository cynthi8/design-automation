#include <iostream>
#include <vector>
#include <string>

#include "SA_algorithm.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if(argc != 3)
    {
        cout << "Error: wrong number of inputs.\n" << "saprog InputFile OutputFile" << endl;
        exit(1);
    }

    vector<string> vargv(argv, argv + argc);

    Graph myGraph(vargv[1]);

    myGraph.SimulatedAnealing(
        40000,
        1,
        .95,
        myGraph.getNodes()*10
    );

    myGraph.PrintSolutionToFile(vargv[2]);

    return 0;
}