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

void Test_FixPlacement(Placement &placement)
{
    //Overwrite the placement grid with a fixed placement (linear scan from 1 to m_cellCount)
    for (int i = 0; i < placement.m_netlist.m_cellCount; i++)
    {
        string cellId = to_string(i + 1);
        int row = i / placement.m_gridWidth;
        int col = i % placement.m_gridWidth;
        Location newLocation = {row, col};
        placement.m_grid.PlaceCell(newLocation, cellId);
        placement.UpdateCellLocation(newLocation, cellId);
    }
}

void Test_InsertFeedThrough(string fileName, const int gridWidth)
{
    cout << "Testing " << fileName << endl;
    Graph graph(fileName);
    Placement placement(graph, gridWidth);
    Test_FixPlacement(placement);

    placement.Print();
    cout << endl;
}

void Test_InsertFeedThroughs()
{
    // These should have Feedthrough cells inserted. No automated testing yet.
    Test_InsertFeedThrough("Benchmarks/b_feedthrough_d1_even", 2);
    Test_InsertFeedThrough("Benchmarks/b_feedthrough_d2_even", 2);
    Test_InsertFeedThrough("Benchmarks/b_feedthrough_d2_odd", 2);
    Test_InsertFeedThrough("Benchmarks/b_feedthrough_d3_even", 2);
    Test_InsertFeedThrough("Benchmarks/b_feedthrough_d3_odd", 2);
}

// Entry point for code
int main(int argc, char *argv[])
{
    cout << "Hello World\n";

    /*
    Graph graph("Benchmarks/b_50_50");
    Placement placement(graph, 10);
    placement.ForceDirectedPlace();
    placement.ForceDirectedFlip();
    //placement.InsertFeedThroughs();
    placement.Print();
    */

    Test_InsertFeedThroughs();

    cout << "Goodbye World\n";
    return 0;
}