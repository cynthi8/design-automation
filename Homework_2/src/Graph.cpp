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

    // Initialize cell list
    m_cells.reserve(m_cellCount + 1); //index 0 is empty
    for (int i = 0; i < m_cellCount + 1; i++)
    {
        m_cells.push_back(Cell(i));
    }

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

// Flip the terminal number
int Cell::RelativeTerm(int term_id) {
    switch (this->m_orientation) {
    case FlipNone:
        break;
    case FlipBoth: //pass through
    case FlipLR:
        if (term_id % 2)
            term_id++;
        else
            term_id--;
        if (this->m_orientation != FlipBoth)
            break;
    case FlipTB:
        if (term_id <= 2)
            term_id += 2;
        else
            term_id -= 2;
        break;
    }
    return term_id;
}

// Get all the terminals that are being used
vector<int> Cell::GetActiveTerminals() {
    vector<int> Terminals;
    for (auto k : this->m_nets)
    {
        for (auto l : k.m_connections)
        {
            if (l.cell == this->m_id) {
                Terminals.push_back(l.location);
            }
        }
    }
    return Terminals;
}

// Get the relative terminal positions of each terminal after flipping
// Nathan: I think I need this to return all terminals in their relative pos lol
vector<pair<int, int>> Cell::GetRelativeTerminals() {
    vector<pair<int, int>> RelTerms;
    vector<int> Terms{ 1,2,3,4 };
    for (auto i : Terms) {
        int relTerm = (*this).RelativeTerm(i);
        RelTerms.push_back({ relTerm, i });
    }

    //sort based on relative location
    sort(RelTerms.begin(), RelTerms.end());
    return RelTerms;
}