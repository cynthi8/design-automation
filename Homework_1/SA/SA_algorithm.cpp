#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <ctime>
#include <random>
#include <cmath>

#include "SA_algorithm.hpp"

// Add an edge to a node, if the edge already exist, increment by 1
void Node::AddEdge(int to) {
    auto it = m_edges.begin(); 
    for(; it != m_edges.end(); it++) {
        if((*it).to == to){
            // If the node was already connected, increment the weight
            (*it).weight++;
            break;
        }
    }
    if(it == m_edges.end()) {
        // If the node wasn't already connected, add the node with weight 1
        WeightedEdge newConnection;
        newConnection.to = to;
        newConnection.weight = 1;
        m_edges.push_back(newConnection);
    }
}

// Look up the edge weight between two nodes
// If there is no edge, return 0
int Node::getEdgeWeight(int toNode) {
    int edgeWeight = 0;
    for(auto it = m_edges.begin(); it != m_edges.end(); it++) {
        if((*it).to == toNode) {
            edgeWeight = (*it).weight;
            break;
        }
    }
    return edgeWeight;
}

void Solution::Initialize() {
    int solutionLength = m_bitVector.size();
    for (int i = 1; i < solutionLength; i++)
    {
        // Set the first half to 1
        if (i < solutionLength/2 + 1) {
            m_bitVector[i] = 1;
        }
        else {
            m_bitVector[i] = 0;
        }
    }
}

void Solution::InitializeCost(vector<Node> & adjList) {
    m_cost = 0;
    for (int i = 1; i < (int) m_bitVector.size(); i++) {
        // Sum the externality connection of edges in one set (doesn't matter which one)
        if(m_bitVector[i]) {
            Connectivity connectivity = CalculateConnectivity(i, adjList);
            m_cost += connectivity.external;
        }
    }
}

Connectivity Solution::CalculateConnectivity(int from, vector<Node> & adjList) {
    int externalConnectivity = 0;
    int internalConnectivity = 0;
    bool currentSet = m_bitVector[from];
    for(auto it = adjList[from].m_edges.begin(); it != adjList[from].m_edges.end(); it++) {
        if(m_bitVector[(*it).to] == currentSet) {
            // Internal connectivity
            internalConnectivity += (*it).weight;
        }
        else {
            // External connectivity
            externalConnectivity += (*it).weight;
        }
    }
    return {externalConnectivity, internalConnectivity};
}

void Solution::AcceptSwap(int node1, int node2, int deltaCost, int & numAccepted) {
    m_bitVector[node1] = !m_bitVector[node1];
    m_bitVector[node2] = !m_bitVector[node2];
    m_cost += deltaCost;
    numAccepted++;
}

void Solution::Print(std::ostream & outputStream) {
    outputStream << m_cost << endl;
    for (size_t i = 1; i < m_bitVector.size(); i++) {
        if(m_bitVector[i] == true) {
            outputStream << i << " ";
        }
    }
    outputStream << endl;
    for (size_t i = 1; i < m_bitVector.size(); i++) {
        if(m_bitVector[i] == false) {
            outputStream << i << " ";
        }
    }
    outputStream << endl;
}

void Solution::PrintToFile(string fileName) {
    fstream outputStream;
	outputStream.open(fileName, ios::out);
    Print(outputStream);
}

// Construct a weighted bidirectional graph from a file name
Graph::Graph(string fileName) {
    fstream fileStream;
	fileStream.open(fileName, ios::in);

    fileStream >> m_nodes;
    fileStream >> m_edges;
    if(m_nodes % 2) {
        throw "Number of nodes is not even.";
    }

    m_adjList.resize(m_nodes + 1); //index 0 is empty
    m_solution.m_bitVector.resize(m_nodes + 1);

    int from, to;
    for(int i = 0; i < m_edges; i++){
        fileStream >> from;
        fileStream >> to;
        for (int mirror = 0; mirror < 2; mirror++) {
            m_adjList[from].AddEdge(to);
            swap(from, to); // symetric
        }
    }
    fileStream.close();
}

// Run simulated anealing on the graph
void Graph::SimulatedAnealing(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep) {
    m_solution.Initialize();
    m_solution.InitializeCost(m_adjList);
    Solution bestSolution = m_solution;
    int bestCost = m_solution.getCost();

    SA_Random sa_random(m_nodes);

    float temperature = initialTemperature;
    while(temperature > freezingTemperature) {
        float boltzmanLimit = exp(-1/temperature);
        int numAccepted = 0;
        for (int i = 0; i < movesPerStep; i++) {
            int node1 = sa_random.randomNode();
            int node2 = sa_random.randomNode();
            while(m_solution.m_bitVector[node1] == m_solution.m_bitVector[node2]) {
                // Loop until selected nodes are part of opposite sets 
                node2 = sa_random.randomNode();
            }
            int deltaCost = CalculateDeltaCost(node1, node2);

            if (deltaCost < 0) {
                m_solution.AcceptSwap(node1, node2, deltaCost, numAccepted);
            }
            else {
                if (sa_random.randomUnit() < pow(boltzmanLimit, deltaCost)) {
                    // Probablistically accept increases in cost 
                    m_solution.AcceptSwap(node1, node2, deltaCost, numAccepted);
                }
            }

            if (m_solution.getCost() < bestCost) {
                // Save best solution to date
                bestSolution = m_solution;
                bestCost = m_solution.getCost();
            }
        }
        float acceptProportion = (float)numAccepted / (float)movesPerStep;
        m_log.LogStep(temperature, boltzmanLimit, acceptProportion, bestCost);
        temperature *= heatRetention;
    }

    // Revert to best found solution
    m_solution = bestSolution;
}

int Graph::CalculateDeltaCost(int node1, int node2) {
    int gain = CalculateDisparity(node1) + CalculateDisparity(node2) - 2*m_adjList[node1].getEdgeWeight(node2);
    return -gain; // Cost is inverse of gain
}

int Graph::CalculateDisparity(int node) {
    bool currentSet = m_solution.m_bitVector[node];
    int disparity = 0;
    for(auto it = m_adjList[node].m_edges.begin(); it != m_adjList[node].m_edges.end(); it++) {
        if(m_solution.m_bitVector[(*it).to] == currentSet) {
            // Internal connectivity
            disparity -= (*it).weight;
        }
        else {
            // External connectivity
            disparity += (*it).weight;
        }
    }
    return disparity;
}

void Log::LogStep(float temperature, float boltzmanLimit, float acceptProportion, int bestCost) {
    m_temperatures.push_back(temperature);
    m_boltzmanLimits.push_back(boltzmanLimit);
    m_acceptProportions.push_back(acceptProportion);
    m_bestCosts.push_back(bestCost);
    m_points++;
}

void Log::Print(std::ostream & outputStream) {
    outputStream << "Iteration\t" << " Temperature\t" << " Boltzman Limit\t" << " Proportion Accepted\t" << " Best Cost\t" << endl;
    for(int i = 0; i < m_points; i++) {
        outputStream << i << "\t" << m_temperatures[i] << "\t" << m_boltzmanLimits[i] << "\t" << m_acceptProportions[i] << "\t" << m_bestCosts[i] << endl;
    }
}

void Log::PrintToFile(string fileName) {
    fstream outputStream;
	outputStream.open(fileName, ios::out);
    Print(outputStream);
}