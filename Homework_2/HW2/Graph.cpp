#include "Graph.hpp"
#include <fstream>
#include <cassert>

using namespace std;

// Construct a netlist graph from a file name
Graph::Graph(string fileName)
{
    fstream fs;
    fs.open(fileName, ios::in);

    fs >> m_cellCount;
    fs >> m_netCount;

    m_cells.resize(m_cellCount + 1); //index 0 is empty

    int netID;
    int cellA, locA, cellB, locB;
    for (int i = 1; i < m_netCount + 1; i++)
    {
        fs >> netID;
        assert(netID == i);

        fs >> cellA;
        fs >> locA;
        fs >> cellB;
        fs >> locB;
        Terminal terminalA{cellA, locA};
        Terminal terminalB{cellB, locB};
        Net net(netID, terminalA, terminalB);

        m_cells[cellA].addNet(net);
        m_cells[cellB].addNet(net);
    }
    fs.close();
}

void Cell::addNet(Net net)
{
    m_nets.push_back(net);
    m_connectivity++;
}