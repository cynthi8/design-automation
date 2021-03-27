//Solution

// I think we can delete this file

#include "HW2.hpp"
#include "Graph.hpp"

void Solution::Initialize() {
    int solutionLength = m_bitVector.size();
    for (int i = 1; i < solutionLength; i++)
    {
        // Set the first half to 1
        if (i < solutionLength / 2 + 1) {
            m_bitVector[i] = 1;
        }
        else {
            m_bitVector[i] = 0;
        }
    }
}

void Solution::InitializeCost(vector<Cell>& adjList) {
    m_cost = 0;
    for (int i = 1; i < (int)m_bitVector.size(); i++) {
        // Sum the externality connection of edges in one set (doesn't matter which one)
        if (m_bitVector[i]) {
            Connectivity connectivity = CalculateConnectivity(i, adjList);
            m_cost += connectivity.external;
        }
    }
}

Connectivity Solution::CalculateConnectivity(int from, vector<Cell>& adjList) {
    int externalConnectivity = 0;
    int internalConnectivity = 0;
    bool currentSet = m_bitVector[from];
    for (auto it = adjList[from].m_edges.begin(); it != adjList[from].m_edges.end(); it++) {
        if (m_bitVector[(*it).to] == currentSet) {
            // Internal connectivity
            internalConnectivity += (*it).weight;
        }
        else {
            // External connectivity
            externalConnectivity += (*it).weight;
        }
    }
    return { externalConnectivity, internalConnectivity };
}


void Solution::AcceptSwap(int node1, int node2, int deltaCost) {
    m_bitVector[node1] = !m_bitVector[node1];
    m_bitVector[node2] = !m_bitVector[node2];
    m_cost += deltaCost;
}

void Solution::AcceptSwap(int node1, int node2, int deltaCost, int& numAccepted) {
    AcceptSwap(node1, node2, deltaCost);
    numAccepted++;
}

void Solution::Print(std::ostream& outputStream) {
    outputStream << m_cost << endl;
    for (size_t i = 1; i < m_bitVector.size(); i++) {
        if (m_bitVector[i] == true) {
            outputStream << i << " ";
        }
    }
    outputStream << endl;
    for (size_t i = 1; i < m_bitVector.size(); i++) {
        if (m_bitVector[i] == false) {
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

//Create something to output a magic file

