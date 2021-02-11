#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <ctime>
#include <random>

#include "SA_algorithm.hpp"

Graph::Graph(string fileName) {
    fstream fileStream;
	fileStream.open(fileName, ios::in);

    fileStream >> m_nodes;
    fileStream >> m_edges;

    m_adjList.resize(m_nodes + 1); //index 0 is empty
    m_solution.resize(m_nodes + 1);

    int from, to;
    for(int i = 0; i < m_edges; i++){
        fileStream >> from;
        fileStream >> to;

        for (int mirror = 0; mirror < 2; mirror++) {
            // Add the connection to the adjacency list
            auto it = m_adjList[from].begin(); 
            for(it; it != m_adjList[from].end(); it++) {
                if((*it).to == to){
                    // If the node was already connected, increment the weight
                    (*it).weight++;
                    break;
                }
            }

            if(it == m_adjList[from].end()) {
                // If the node wasn't already connected, add the node with weight 1
                WeightedEdge newConnection;
                newConnection.to = to;
                newConnection.weight = 1;
                m_adjList[from].push_back(newConnection);
            }
            swap(from, to); // symetric
        }
    }
    fileStream.close();
}

void Graph::SimulatedAnealing(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep) {
    InitializeSolution();
    vector<bool> bestSolution = m_solution;
    int bestCost = m_cost;
    const float k = 5 / (0.69314718056 * initialTemperature); // Assumed a deltaCost of 10 and tried to get the limit to .5 at initialTemp/2

    // Using <random> just to try it out
    // https://www.youtube.com/watch?v=LDPMpc-ENqY this is a interesting talk about it
    std::default_random_engine generator;
    generator.seed(std::time(nullptr));
    std::uniform_int_distribution<int> nodeDistribution(1, m_nodes);
    std::uniform_real_distribution<float> unitDistribution(0, 1);

    auto getRandomNode = std::bind(nodeDistribution, generator);
    auto getRandomNormalized = std::bind(unitDistribution, generator);

    float temperature = initialTemperature;
    while(temperature > freezingTemperature) {
        for (int i = 0; i < movesPerStep; i++) {
            int node1 = getRandomNode();
            int node2 = getRandomNode();
            while(m_solution[node1] == m_solution[node2]) {
                // Loop until selected nodes are part of opposite sets 
                node2 = getRandomNode();
            }
            int deltaCost = CalculateDeltaCost(node1, node2);

            if (deltaCost < 0) {
                AcceptSwap(node1, node2, deltaCost);
            }
            else {
                if (getRandomNormalized() <  0) { // FIXME currently greedy
                    AcceptSwap(node1, node2, deltaCost);
                }
            }

            if (m_cost < bestCost) {
                // Save best solution to date
                bestCost = m_cost;
                bestSolution = m_solution;
            }
        }
        temperature *= heatRetention;
    }

    // Revert to best found solution
    m_cost = bestCost;
    m_solution = bestSolution;
}

void Graph::InitializeSolution() {
    if(m_nodes % 2) {
        throw "Number of nodes is not even.";
    }
    for (int i = 1; i < m_nodes; i++)
    {
        // Set the first half to 1
        if (i < m_nodes/2 + 1) {
            m_solution[i] = 1;
        }
        else {
            m_solution[i] = 0;
        }
    }
    InitializeCost();
}

void Graph::InitializeCost() {
    m_cost = 0;
    for (int i = 1; i < m_nodes; i++) {
        // Sum the weights of edges in one set (doesn't matter which one)
        if(m_solution[i]) {
            for(auto it = m_adjList[i].begin(); it != m_adjList[i].end(); it++) {
                m_cost += (*it).weight;
            }
        }
    }
}

int Graph::CalculateDeltaCost(int node1, int node2) {
    int pairwiseConnection = 0;
    for(auto it = m_adjList[node1].begin(); it != m_adjList[node1].end(); it++) {
        // Check if node1 and node2 are connected
        if((*it).to == node2) {
            pairwiseConnection = (*it).weight;
        }
    }
    int gain = CalculateDisparity(node1) + CalculateDisparity(node2) - 2*pairwiseConnection;
    return -gain; // Cost is inverse of gain
}

int Graph::CalculateDisparity(int node) {
    bool currentSet = m_solution[node];
    int disparity = 0;
    for(auto it = m_adjList[node].begin(); it != m_adjList[node].end(); it++) {
        if(m_solution[(*it).to] == currentSet) {
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

void Graph::AcceptSwap(int node1, int node2, int deltaCost) {
    m_solution[node1] = !m_solution[node1];
    m_solution[node2] = !m_solution[node2];
    m_cost += deltaCost;
}