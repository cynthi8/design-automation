/*
Main.cpp
VLSI Design: HW2
Date: 2021-03-12
Group: Nathaniel Hernandez; Erin Cold
*/
#include <iostream>
#include <cassert>
#include <chrono>
#include <cmath>

#include "Graph.hpp"
#include "Placement.hpp"

//#include "Magic.hpp"

using namespace std;

struct Benchmark
{
    string fileName;
    int gridWidth;
};

vector<Benchmark> Benchmarks{
    {"Benchmarks/b_50_50", (int)sqrt(50) * 2},
    {"Benchmarks/b_100_100", (int)sqrt(100) * 2},
    {"Benchmarks/b_400_400", (int)sqrt(400) * 2},
    {"Benchmarks/b_600_1000", (int)sqrt(600) * 2},
    {"Benchmarks/b_900_800", (int)sqrt(900) * 2},
    {"Benchmarks/b_1000_1000", (int)sqrt(1000) * 2},
    {"Benchmarks/b_1000_1200", (int)sqrt(1000) * 2},
    {"Benchmarks/b_1200_1500", (int)sqrt(1200) * 2},
    {"Benchmarks/b_1500_1500", (int)sqrt(1500) * 2},
    {"Benchmarks/b_2000_2000", (int)sqrt(2000) * 2}};

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

void Test_Placement(string fileName, const int gridWidth)
{
    auto start = chrono::system_clock::now();
    Graph graph(fileName);
    Placement placement(graph, gridWidth);
    int originalPlacementCost = placement.CalculatePlacementCost();
    placement.SimulatedAnealingPlace(1000, 1, .95, 10000);
    int postSimulatedAnealingCost = placement.CalculatePlacementCost();
    placement.GreedyFlipping(10);
    int postFlippingCost = placement.CalculatePlacementCost();
    placement.InsertFeedthroughs();
    int postFeedthroughsCost = placement.CalculatePlacementCost();
    int feedthroughCount = placement.m_feedthroughCount;
    int msPassed = MilisecondsPassed(start);

    cout << fileName << "\t" << originalPlacementCost << "\t\t\t" << postSimulatedAnealingCost << "\t\t\t"
         << postFlippingCost << "\t\t\t" << postFeedthroughsCost << "\t\t" << feedthroughCount << "\t\t" << msPassed << endl;
}

void Test_Placements()
{
    cout << "fileName \t"
         << "originalPlacementCost "
         << "postSimulatedAnealingCost "
         << "postFlippingCost "
         << "postFeedthroughsCost "
         << "feedthroughCount "
         << "msPassed " << endl;
    for (auto benchmark : Benchmarks)
    {
        Test_Placement(benchmark.fileName, benchmark.gridWidth);
    }
    cout << endl;
}

// Entry point for code
int main(int argc, char *argv[])
{
    try
    {
        Test_Placements();
    }
    catch (invalid_argument &e)
    {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}