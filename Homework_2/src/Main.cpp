/*
Main.cpp
VLSI Design: HW2
Date: 2021-03-12
Group: Nathaniel Hernandez; Erin Cold
*/
#include <iostream>
#include "Graph.hpp"
#include "Placement.hpp"

//#include "Routing.hpp"
//#include "Magic.hpp"
//#include "Test.cpp"

using namespace std;

// Entry point for code
int main(int argc, char *argv[])
{
    cout << "Hello World\n";

    /*
    if(argc != 3)
    {
        cout << "Error: wrong number of inputs.\n" << "What is this called? InputFile OutputFile" << endl;
        exit(1);
    }
    */
    vector<string> vargv(argv, argv + argc);

    Graph graph("Benchmarks/b_50_50");

    Placement placement(graph, 10);
    placement.ForceDirectedPlace();
    placement.ForceDirectedFlip();
    //Grid grid = placement.Export();


    //int test = tests(argc, argv);
    /*
    Graph graph(vargv[1]);
    Placement place(graph);
    Routing Route(graph, place);
    WriteToMagic(routing, vargv[2]);
    */

    return 0;
}