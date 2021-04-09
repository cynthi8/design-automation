/*
Main.cpp
VLSI Design: HW2
Date: 2021-03-12
Group: Nathaniel Hernandez; Erin Cold
*/
#include <iostream>
#include <cassert>
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

void Test_FeedthroughRouting()
{
    Graph graph("Benchmarks/b_feedthrough_1_left");
    Placement placement(graph, 2);
    Test_FixPlacement(placement);
    placement.InsertFeedthroughs();
    assert(placement.m_netlist.m_cells["2"].m_id == "2");
    Cell &cell = placement.m_netlist.m_cells["1"];
    placement.m_netlist.printTrace(cell.getTerminal(1));
}

void Test_InsertFeedthrough(string fileName, const int gridWidth)
{
    cout << "Testing " << fileName << endl;
    Graph graph(fileName);
    Placement placement(graph, gridWidth);
    Test_FixPlacement(placement);
    placement.InsertFeedthroughs();
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

// Entry point for code
int main(int argc, char *argv[])
{
    cout << "Hello World\n";

    Test_FeedthroughRouting();

    Test_InsertFeedthroughs();

    Graph graph("Benchmarks/b_50_50");
    Placement placement(graph, 10);
    cout << "Original Placement Cost: " << placement.CalculatePlacementCost() << endl;
    placement.ForceDirectedPlace(10);
    cout << "Placement Cost after force directed: " << placement.CalculatePlacementCost() << endl;
    placement.ForceDirectedFlip(10);
    cout << "Placement Cost after flipping: " << placement.CalculatePlacementCost() << endl;
    placement.InsertFeedthroughs();
    cout << "Placement Cost after feedthrough insertion: " << placement.CalculatePlacementCost() << endl;

    placement.Print();
    placement.m_netlist.printTrace(placement.m_netlist.m_cells["1"].getTerminal(3));

    cout << "Goodbye World\n";
    return 0;
}