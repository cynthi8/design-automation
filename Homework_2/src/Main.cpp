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

const vector<Benchmark> TestBenchmarks{
    {"benchmarks/b_routing_easy", 2},
    {"benchmarks/b_tiny", 2},
    {"benchmarks/b_feedthrough_0", 2},
    {"benchmarks/b_feedthrough_1_left", 2},
    {"benchmarks/b_feedthrough_1_right", 2},
    {"benchmarks/b_feedthrough_2_left", 2},
    {"benchmarks/b_feedthrough_2_right", 2},
    {"benchmarks/b_feedthrough_3_right", 2},
    {"benchmarks/b_feedthrough_multi", 2},
    {"benchmarks/b_vertical_dependency", 2},
    {"benchmarks/b_basic_dogleg", 1},
    {"benchmarks/b_dogleg_with_space", 2},
    {"benchmarks/b_double_doglegs", 2}};

const vector<Benchmark> Benchmarks{
    {"benchmarks/b_50_50", (int)(sqrt(50) * 1.5)},
    {"benchmarks/b_100_100", (int)(sqrt(100) * 1.5)},
    {"benchmarks/b_400_400", (int)(sqrt(400) * 1.7)},
    {"benchmarks/b_600_1000", (int)sqrt(600) * 2},
    {"benchmarks/b_900_800", (int)(sqrt(900) * 1.8)},
    {"benchmarks/b_1000_1000", (int)(sqrt(1000) * 1.9)},
    {"benchmarks/b_1000_1200", (int)sqrt(1000) * 2},
    {"benchmarks/b_1200_1500", (int)sqrt(1200) * 2},
    {"benchmarks/b_1500_1500", (int)sqrt(1500) * 2},
    {"benchmarks/b_2000_2000", (int)sqrt(2000) * 2}};

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

    // Do Placement
    Placement placement(graph, benchmark.gridWidth);
    Test_FixPlacement(placement);
    placement.InsertFeedthroughs();
    placement.Print();

    // Do Routing
    Routing routing(placement);
    routing.Print();

    Magic magic(placement, routing);
    magic.Output("output", "output.mag");
}

void PlaceAndRoute(Benchmark benchmark, int BenchNum)
{
    auto start = chrono::system_clock::now();

    // Process Graph
    Graph graph(benchmark.fileName);

    // Do Placement
    Placement placement(graph, benchmark.gridWidth);
    placement.SimulatedAnealingPlace(1000, 1, .975, 10000);
    //placement.SimulatedAnealingPlace(1000, 1, .95, 100); //debug only
    placement.GreedyFlipping(10);
    placement.InsertFeedthroughs();
    int feedthroughCount = placement.m_feedthroughCount;

    // Do Routing
    Routing routing(placement);

    // Do Magic
    Magic magic(placement, routing);
    magic.Output("output/", to_string(BenchNum) + ".mag");
    int msPassed = MilisecondsPassed(start);

    // Collect Info
    pair<int, int> boundingBox = magic.CalculateBoundingBox();
    int boundingBoxArea = boundingBox.first * boundingBox.second;
    string boundingBoxDimensions = to_string(boundingBox.first) + "x" + to_string(boundingBox.second);
    int wirelength = magic.CalculateWirelength();
    int viaCount = magic.CalculateViaCount();

    // Print out
    cout << benchmark.fileName << "\t" << boundingBoxDimensions << "\t" << boundingBoxArea << "\t"
         << feedthroughCount << "\t" << wirelength << "\t" << viaCount << "\t" << msPassed << endl;
}

int gridWidthCalc(Graph& graph)
{

    const vector<pair<int, double>> BenchVals{
    {   50,     1.5 },
    {   100,    1.5 },
    {   400,    1.7 },
    {   600,    2   },
    {   900,    1.8 },
    {   1000,   1.9 },
    {   1200,   2   },
    {   1200,   2   },
    {   1500,   2   },
    {   2000,   2   } };

    if (graph.m_cellCount > BenchVals.back().first) {
        return (int)(sqrt(graph.m_cellCount) * BenchVals.back().second);
    }
    else if (graph.m_cellCount < BenchVals[0].first) {
        return (int)(sqrt(graph.m_cellCount) * BenchVals[0].second);
    }
    
    for (int i = 0; i < BenchVals.size(); i++)
    {
        int size = BenchVals[i].first;
        double adjust = BenchVals[i].second;
        if (graph.m_cellCount == size)
        {
            return (int)(sqrt(size) * adjust);
        }
        else if (i < BenchVals.size() - 1 && 
                 graph.m_cellCount > size && 
                 graph.m_cellCount < BenchVals[i + 1].first)
        {
            double adjust2 = (adjust + BenchVals[i + 1].second) / 2;
            return (int)(sqrt(graph.m_cellCount) * adjust2);
        }
    }
    
    
    return (int)(sqrt(graph.m_cellCount) * 1.75);;
}

void PlaceAndRoute(string szfilenameUse, string szfilenameSave)
{
    auto start = chrono::system_clock::now();

    // Process Graph
    Graph graph(szfilenameUse);
    int gridWidth = gridWidthCalc(graph);

    // Do Placement
    Placement placement(graph, gridWidth);
    placement.SimulatedAnealingPlace(1000, 1, .975, 10000);
    //placement.SimulatedAnealingPlace(1000, 1, .95, 100); //debug only
    placement.GreedyFlipping(10);
    placement.InsertFeedthroughs();
    int feedthroughCount = placement.m_feedthroughCount;

    // Do Routing
    Routing routing(placement);

    // Do Magic
    Magic magic(placement, routing);
    magic.Output("", szfilenameSave);
    int msPassed = MilisecondsPassed(start);

    // Collect Info
    pair<int, int> boundingBox = magic.CalculateBoundingBox();
    int boundingBoxArea = boundingBox.first * boundingBox.second;
    string boundingBoxDimensions = to_string(boundingBox.first) + "x" + to_string(boundingBox.second);
    int wirelength = magic.CalculateWirelength();
    int viaCount = magic.CalculateViaCount();

    // Print out
    cout << szfilenameUse << "\t" << boundingBoxDimensions << "\t" << boundingBoxArea << "\t"
        << feedthroughCount << "\t" << wirelength << "\t" << viaCount << "\t" << msPassed << endl;
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
    //placement.Print();

    // Do Routing
    Routing routing(placement);
    routing.Print();

    Magic magic(placement, routing);
    magic.Output("output", "output.mag");
}



// Entry point for code
int main(int argc, char *argv[])
{
    vector<string> vargv(argv, argv + argc);
    string szfilenameUse = "";
    string szfilenameSave = "";

    cout << "fileName boundingBoxDimensions boundingBoxArea feedthroughCount wirelength viaCount msPassed" << endl;

    if (argc == 3) {
        szfilenameUse = vargv[1];  //what file to load
        szfilenameSave = vargv[2]; //what file to save
        cout << szfilenameUse << " " << szfilenameSave << endl;
        PlaceAndRoute(szfilenameUse, szfilenameSave);
        return 0;
    }


    int BenchNum = 1;
    for (auto benchmark : Benchmarks)
    {
        PlaceAndRoute(benchmark, BenchNum);
        BenchNum++;
    }
    return 0;
}
