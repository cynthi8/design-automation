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

    Graph graph("Benchmarks/b_50_50");
    Placement placement(graph, 10);
    /*
    Placement placement(graph, 10);
    placement.ForceDirectedPlace();
    placement.ForceDirectedFlip();
    //Grid grid = placement.Export();
    */

    return 0;
}