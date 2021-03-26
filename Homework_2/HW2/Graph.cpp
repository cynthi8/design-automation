#include "Graph.hpp"
#include <cassert>

// Construct a netlist graph from a file name
Graph::Graph(string fileName) {
    fstream fs;
    fs.open(fileName, ios::in);

    fs >> m_cells;
    fs >> m_nets;

    m_cells.resize(m_nodes + 1); //index 0 is empty

    int netID;
    int cellA, terminalA, cellB, terminalB
    for (int i = 1; i < m_nets + 1; i++) {
        fs >> netID;
        assert(netID == i)
        fs >> cellA; fs >> terminalA;
        fs >> cellB; fs >> terminalB;

        for (int mirror = 0; mirror < 2; mirror++) {
            m_adjList[from].AddEdge(to);
            swap(from, to); // symetric
        }
    }
    fs.close();

    m_solution.Initialize();
    m_solution.InitializeCost(m_adjList);
}


/*
void Terminal::AddEdge(int to) {
    auto it = m_edges.begin();

    for (; it != m_edges.end(); it++) {
        if ((*it).to == to) {
            // If the node was already connected, increment the weight
            (*it).weight++;
            break;
        }
    }
    if (it == m_edges.end()) {
        // If the node wasn't already connected, add the node with weight 1
        WeightedEdge newConnection;
        newConnection.to = to;
        newConnection.weight = 1;
        m_edges.push_back(newConnection);
    }
}
*/

// Add an edge to a node, if the edge already exist, increment by 1
void Node::AddEdge(int to) {
    auto it = m_edges.begin();
    
    for (; it != m_edges.end(); it++) {
        if ((*it).to == to) {
            // If the node was already connected, increment the weight
            (*it).weight++;
            break;
        }
    }
    if (it == m_edges.end()) {
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
    for (auto it = m_edges.begin(); it != m_edges.end(); it++) {
        if ((*it).to == toNode) {
            edgeWeight = (*it).weight;
            break;
        }
    }
    return edgeWeight;
}



// Run simulated anealing on the graph
/*
void Graph::SimulatedAnealing(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep) {
    Solution bestSolution = m_solution;
    int bestCost = m_solution.getCost();

    float temperature = initialTemperature;
    while (temperature > freezingTemperature) {
        float boltzmanLimit = exp(-1 / temperature);
        int numAccepted = 0;
        for (int i = 0; i < movesPerStep; i++) {
            int node1, node2;
            FindOpposingNodes(node1, node2);
            int deltaCost = CalculateDeltaCost(node1, node2);

            if (deltaCost < 0) {
                m_solution.AcceptSwap(node1, node2, deltaCost, numAccepted);
            }
            else {
                if (m_graphRandom.randomUnit() < pow(boltzmanLimit, deltaCost)) {
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
        m_log.LogStep(temperature, boltzmanLimit, acceptProportion, m_solution.getCost(), bestCost);
        temperature *= heatRetention;
    }

    // Revert to best found solution
    m_solution = bestSolution;
}
*/

/*
float Graph::CalculateInitialTemperature(float desiredAcceptedProportion) {
    const int c_samples = max(1000, m_nodes);
    vector<float> temperatureSamples;

    for (int i = 0; i < c_samples; i++) {
        // Random walk to generate state transitions
        int node1, node2;
        FindOpposingNodes(node1, node2);
        int deltaCost = CalculateDeltaCost(node1, node2);
        m_solution.AcceptSwap(node1, node2, deltaCost);

        deltaCost = abs(deltaCost); //Only consider the postive cost transition
        float T = (-deltaCost) / log(desiredAcceptedProportion); //Calculate the T that would accept this cost with the desired probablility
        temperatureSamples.push_back(T);
    }

    // Online averaging to avoid overflow
    float average = 0;
    int n = 1;
    for (auto sample : temperatureSamples) {
        float delta = sample - average;
        average += delta / n;
        n++;
    }

    return average;

}
*/

void Graph::FindOpposingNodes(int& node1, int& node2) {
    node1 = m_graphRandom.randomNode();
    node2 = m_graphRandom.randomNode();
    while (m_solution.m_bitVector[node1] == m_solution.m_bitVector[node2]) {
        // Loop until selected nodes are part of opposite sets 
        node2 = m_graphRandom.randomNode();
    }
}

int Graph::CalculateDeltaCost(int node1, int node2) {
    int gain = CalculateDisparity(node1) + CalculateDisparity(node2) - 2 * m_adjList[node1].getEdgeWeight(node2);
    return -gain; // Cost is inverse of gain
}

// Disparity is how strongly a node is pulled to the other set = External - Internal connectivity
int Graph::CalculateDisparity(int node) {
    bool currentSet = m_solution.m_bitVector[node];
    int disparity = 0;
    for (auto edge : m_adjList[node].m_edges) {
        if (m_solution.m_bitVector[edge.to] == currentSet) {
            // Internal connectivity
            disparity -= edge.weight;
        }
        else {
            // External connectivity
            disparity += edge.weight;
        }
    }
    return disparity;
}

GraphRandom::GraphRandom() : m_unitDistribution(0, 1) {}

void GraphRandom::setNodes(int nodes) {
    std::uniform_int_distribution<int>::param_type nodeDist(1, nodes);
    m_nodeDistribution.param(nodeDist);
}