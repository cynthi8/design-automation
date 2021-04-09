/*
Main.cpp
VLSI Design: HW2
Date: 2021-03-12
Group: Nathaniel Hernandez; Erin Cold
*/
#include <iostream>
#include <cassert>
#include <chrono>

#include "Graph.hpp"
#include "Placement.hpp"

//#include "Magic.hpp"

using namespace std;

int MilisecondsPassed(chrono::system_clock::time_point start)
{
    // Helper function for time passed
    auto end = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

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

void Test_FeedthroughRouting()
{
    Graph graph("Benchmarks/b_feedthrough_multi");
    Placement placement(graph, 2);
    Test_FixPlacement(placement);
    placement.InsertFeedthroughs();
    placement.m_netlist.printTrace(placement.m_netlist.m_cells["1"].getTerminal(4));
}

void Test_InsertFeedthrough(string fileName, const int gridWidth)
{
    cout << "Testing " << fileName << endl;
    Graph graph(fileName);
    Placement placement(graph, gridWidth);
    Test_FixPlacement(placement);
    cout << "Cost before feedthrough insertion: " << placement.CalculatePlacementCost() << endl;
    placement.InsertFeedthroughs();
    cout << "Cost after feedthrough insertion: " << placement.CalculatePlacementCost() << endl;
    placement.Print();
    cout << endl;
}

void Test_InsertFeedthroughs()
{
    // These should have Feedthrough cells inserted. No automated testing yet.
    Test_InsertFeedthrough("Benchmarks/b_feedthrough_0", 2);
    Test_InsertFeedthrough("Benchmarks/b_feedthrough_1_left", 2);
    Test_InsertFeedthrough("Benchmarks/b_feedthrough_1_right", 2);
    Test_InsertFeedthrough("Benchmarks/b_feedthrough_2_left", 2);
    Test_InsertFeedthrough("Benchmarks/b_feedthrough_2_right", 2);
    Test_InsertFeedthrough("Benchmarks/b_feedthrough_3_right", 2);
    Test_InsertFeedthrough("Benchmarks/b_feedthrough_multi", 2);
}

void Test_SimulatedAnealingPlacement(string fileName, const int gridWidth)
{

    auto start = chrono::system_clock::now();
    Graph graph(fileName);
    Placement placement(graph, gridWidth);
    Test_FixPlacement(placement);
    cout << "Original Placement Cost: " << placement.CalculatePlacementCost() << endl;

    placement.SimulatedAnealingPlace(1000, 1, .95, 2000);
    cout << "Placement Cost after Simulated Anealing: " << placement.CalculatePlacementCost() << endl;

    placement.ForceDirectedFlip(10);
    cout << "Placement Cost after flipping: " << placement.CalculatePlacementCost() << endl;

    placement.InsertFeedthroughs();
    cout << "Placement Cost after feedthrough insertion: " << placement.CalculatePlacementCost() << endl;

    placement.Print();
    cout << "Feedthrough Cells: " << placement.m_feedthroughCount << endl;
    int msPassed = MilisecondsPassed(start);
    cout << "Execution Time (ms): " << msPassed << endl;

    cout << endl;
}

// Entry point for code
int main(int argc, char *argv[])
{
    cout << "Hello World\n";

    try
    {
        //Test_InsertFeedthroughs();
        Test_SimulatedAnealingPlacement("Benchmarks/b_50_50", 10);
        //Test_FeedthroughRouting();
    }
    catch (invalid_argument &e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    cout << "Goodbye World\n";
    return 0;
}