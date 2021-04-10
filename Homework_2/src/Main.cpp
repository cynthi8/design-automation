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
#include "Routing.hpp"
#include "Magic.hpp"

using namespace std;

struct Benchmark
{
    string fileName;
    int gridWidth;
};

vector<Benchmark> TestBenchmarks{
    {"Benchmarks/b_routing_easy", 2},
    {"Benchmarks/b_tiny", 2},
    {"Benchmarks/b_feedthrough_0", 2},
    {"Benchmarks/b_feedthrough_1_left", 2},
    {"Benchmarks/b_feedthrough_1_right", 2},
    {"Benchmarks/b_feedthrough_2_left", 2},
    {"Benchmarks/b_feedthrough_2_right", 2},
    {"Benchmarks/b_feedthrough_3_right", 2},
    {"Benchmarks/b_feedthrough_multi", 2}};

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
        int gridWidth = placement.m_grid.m_grid[0].size(); // assume all rows are same size
        string cellId = to_string(i + 1);
        int row = i / gridWidth;
        int col = i % gridWidth;
        Location newLocation = {row, col};
        placement.PlaceCell(newLocation, cellId);
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

void Test_Placement(string fileName, const int gridWidth)
{
    auto start = chrono::system_clock::now();
    Graph graph(fileName);
    Placement placement(graph, gridWidth);
    int originalPlacementCost = placement.CalculatePlacementCost();
    placement.SimulatedAnealingPlace(1000, 1, .975, 10000);
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

void Test_FeedthroughCount(string fileName, const int gridWidth)
{
    Graph graph(fileName);
    Placement placement(graph, gridWidth);
    placement.InsertFeedthroughs();
    int originalFeedthroughCount = placement.m_feedthroughCount;

    Placement placement1(graph, gridWidth);
    placement1.SimulatedAnealingPlace(1000, 1, .975, 10000);
    placement1.InsertFeedthroughs();
    int postPlacementFeedthroughCount = placement1.m_feedthroughCount;

    Placement placement2(graph, gridWidth);
    placement2.SimulatedAnealingPlace(1000, 1, .975, 10000);
    placement2.GreedyFlipping(10);
    placement2.InsertFeedthroughs();
    int postFlippingFeedthroughCount = placement2.m_feedthroughCount;

    cout << fileName << "\t" << originalFeedthroughCount << "\t\t\t" << postPlacementFeedthroughCount << "\t\t\t" << postFlippingFeedthroughCount << endl;
}

void Test_FeedthroughCounts()
{
    cout << "fileName \t"
         << "originalFeedthroughCount "
         << "postPlacementFeedthroughCount "
         << "postFlippingFeedthroughCount " << endl;
    for (auto benchmark : Benchmarks)
    {
        Test_FeedthroughCount(benchmark.fileName, benchmark.gridWidth);
    }
    cout << endl;
}

void Test_Routing(Benchmark benchmark)
{
    // Process Graph
    Graph graph(benchmark.fileName);

    // Do Placement and Collect Data
    Placement placement(graph, benchmark.gridWidth);
    placement.Print();

    // Do Routing
    Routing routing(placement);
    routing.Print();
}

void PlaceAndRoute(Benchmark benchmark)
{
    auto start = chrono::system_clock::now();

    // Process Graph
    Graph graph(benchmark.fileName);

    // Do Placement and Collect Data
    Placement placement(graph, benchmark.gridWidth);
    int originalPlacementCost = placement.CalculatePlacementCost();
    //placement.SimulatedAnealingPlace(1000, 1, .975, 10000);
    placement.SimulatedAnealingPlace(1000, 1, .95, 100); //debug only
    int postSimulatedAnealingCost = placement.CalculatePlacementCost();
    placement.GreedyFlipping(10);
    int postFlippingCost = placement.CalculatePlacementCost();
    placement.InsertFeedthroughs();
    int postFeedthroughsCost = placement.CalculatePlacementCost();
    int feedthroughCount = placement.m_feedthroughCount;

    // Do Routing
    Routing routing(placement);
    routing.Print();

    int msPassed = MilisecondsPassed(start);

    // Print out
    cout << benchmark.fileName << "\t" << originalPlacementCost << "\t\t\t" << postSimulatedAnealingCost << "\t\t\t"
         << postFlippingCost << "\t\t\t" << postFeedthroughsCost << "\t\t" << feedthroughCount << "\t\t" << msPassed << endl;
}

void Test_Magic(Benchmark benchmark)
{
    // Process Graph
    Graph graph(benchmark.fileName);

    // Do Placement and Collect Data
    Placement placement(graph, benchmark.gridWidth);
    placement.SimulatedAnealingPlace(1000, 1, .95, 100);
    placement.GreedyFlipping(10);
    placement.InsertFeedthroughs();
    placement.Print();

    // Do Routing
    Routing routing(placement);
    routing.Print();

    Magic magic(placement, routing);
    magic.Output("output", "output.mag");
}

// Entry point for code
int main(int argc, char *argv[])
{
    try
    {
        //Test_Placements();
        //Test_FeedthroughCounts();
        Test_Magic(TestBenchmarks[0]);
        //PlaceAndRoute({ "Benchmarks/b_tiny", 4 });

        PlaceAndRoute(Benchmarks[0]);
    }
    catch (invalid_argument &e)
    {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}