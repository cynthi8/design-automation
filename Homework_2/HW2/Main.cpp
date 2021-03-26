/*
Main.cpp
VLSI Design: HW2
Date: 2021-03-12
Group: Nathaniel Hernandez; Erin Cold
*/

#include "HW2.h"

using namespace std;

// Entry point for code
int main(int argc, char* argv[]) {
	cout << "\nHello World";

    if(argc != 3)
    {
        cout << "Error: wrong number of inputs.\n" << "What is this called? InputFile OutputFile" << endl;
        exit(1);
    }
    vector<string> vargv(argv, argv + argc);

    Graph graph(vargv[1]);
    Placement placement(graph)
    Routing routing(placement, graph)
    WriteToMagic(routing, vargv[2])
    
	return 1;
}