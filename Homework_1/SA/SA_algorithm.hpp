#ifndef SA_algorithm_HPP
#define SA_algorithm_HPP

#include <string>
#include <vector>

using namespace std;

struct WeightedEdge
{
    int to;
    int weight;
};

class Graph {
public:
    Graph(string fileName);
    void SimulatedAnealing(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep);
    void PrintSolution();
private:
    vector<vector<WeightedEdge>> m_adjList;
    int m_nodes;
    int m_edges;

    void InitializeSolution();
    void InitializeCost();
    int CalculateDeltaCost(int node1, int node2);
    int CalculateDisparity(int node); // Disparity is how stronly a node is pulled to the other set = External - Internal connectivity
    void AcceptSwap(int node1, int node2, int deltaCost);
    int m_cost;
    vector<bool> m_solution;
};

#endif