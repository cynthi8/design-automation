#ifndef SA_algorithm_HPP
#define SA_algorithm_HPP

#include <string>
#include <vector>

using namespace std;

class Graph;

struct WeightedEdge
{
    int to;
    int weight;
};

struct Connectivity
{
    int external;
    int internal;
};

class Node {
public:
    void AddEdge(int to);
    int getEdgeWeight(int toNode);
    vector<WeightedEdge> m_edges;
};

class Log {
public:
    Log() {m_points = 0;};
    void LogStep(float temperature, float boltzmanLimit, float acceptProportion, int bestCost);
    void PrintLog();
private:
    int m_points;
    vector<float> m_temperatures;
    vector<float> m_boltzmanLimits;
    vector<float> m_acceptProportions;
    vector<int> m_bestCosts;
};

class Solution {
public:
    void AcceptSwap(int node1, int node2, int deltaCost, int * numAccepted);
    void PrintSolution();
    void Initialize();
    void InitializeCost(vector<Node> * adjList);
    Connectivity CalculateConnectivity(int from, vector<Node> * adjList);
    int getCost() {return m_cost;};
    vector<bool> m_bitVector;
private:
    int m_cost;
};

class Graph {
public:
    Graph(string fileName);
    void SimulatedAnealing(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep);
    int getEdgeWeight(int from, int to) {return m_adjList[from].getEdgeWeight(to);};
    int getCost() {return m_solution.getCost();};
    void PrintSolution() {m_solution.PrintSolution();};
    Log m_log;
    Solution m_solution;
private:
    vector<Node> m_adjList;
    int m_nodes;
    int m_edges;
    int CalculateDeltaCost(int node1, int node2);
    int CalculateDisparity(int node); // Disparity is how stronly a node is pulled to the other set = External - Internal connectivity
};

#endif