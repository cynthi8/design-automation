#pragma once
#ifndef HW2_HPP
#define HW2_HPP

#include <algorithm> 
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <random>
#include <numeric>
#include <limits>

using namespace std;

struct Connectivity
{
    int external;
    int internal;
};



class Log {
public:
    Log() { m_points = 0; };
    void LogStep(float temperature, float boltzmanLimit, float acceptProportion, int cost, int bestCost);
    void Print(std::ostream& outputStream);
    void PrintToFile(string fileName);
private:
    int m_points;
    vector<float> m_temperatures;
    vector<float> m_boltzmanLimits;
    vector<float> m_acceptProportions;
    vector<int> m_costs;
    vector<int> m_bestCosts;
};

class Solution {
public:
    void AcceptSwap(int node1, int node2, int deltaCost);
    void AcceptSwap(int node1, int node2, int deltaCost, int& numAccepted);
    void Initialize();
    void InitializeCost(vector<Node>& adjList);
    void Print(std::ostream& outputStream = std::cout);
    void PrintToFile(string fileName);
    Connectivity CalculateConnectivity(int from, vector<Node>& adjList);
    int getCost() { return m_cost; };
    vector<bool> m_bitVector;
private:
    int m_cost;
};

class GraphRandom {
public:
    GraphRandom();
    void setNodes(int nodes);
    int randomNode() { return m_nodeDistribution(m_generator); };
    float randomUnit() { return m_unitDistribution(m_generator); };
private:
    std::mt19937 m_generator; //Mersenne_Twister and not seeded for reproducibility
    std::uniform_int_distribution<int> m_nodeDistribution;
    std::uniform_real_distribution<float> m_unitDistribution;
};

class Graph {
public:
    Graph(string fileName);
    //void SimulatedAnealing(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep);
    //float CalculateInitialTemperature(float desiredAcceptedProportion);
    void FindOpposingNodes(int& node1, int& node2);
    //int getEdgeWeight(int from, int to) { return m_adjList[from].getEdgeWeight(to); };
    int getCost() { return m_solution.getCost(); };
    int getNodes() { return m_nodes; };
    void PrintSolution() { m_solution.Print(std::cout); };
    void PrintLog() { m_log.Print(std::cout); };
    void PrintSolutionToFile(string fileName) { m_solution.PrintToFile(fileName); };
    void PrintLogToFile(string fileName) { m_log.PrintToFile(fileName); };
private:
    Solution m_solution;
    Log m_log;
    vector<Node> m_adjList;
    GraphRandom m_graphRandom;
    int m_nodes;
    int m_edges;
    int CalculateDeltaCost(int node1, int node2);
    int CalculateDisparity(int node);
};

#endif