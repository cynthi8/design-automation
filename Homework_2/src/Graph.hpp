#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <string>
#include <algorithm>

using namespace std;

/*
Graph has Cells
Each Cell has Nets 
Each Net has Terminals
Each Terminal is {Cell_ID, Location}
*/

enum Flips : unsigned int
{
    FlipNone = 0,
    FlipLR = 1,
    FlipTB = 2,
    FlipBoth = 3
};

struct Terminal
{
    //Terminal(Terminal term) : cellId(term.cellId), location(term.location) {};
    Terminal(int cellId, int location) : cellId(cellId), location(location) {};
    int cellId;
    int location;
};

class Net
{
public:
    Net(int id, Terminal terminalA, Terminal terminalB) : m_id(id), m_connections({terminalA, terminalB}){};
    int m_id;
    vector<Terminal> m_connections;
};

class Cell
{
public:
    Cell(int id) : m_id(id), m_connectivity(0), m_orientation(FlipNone){};
    
    // Flip the terminal number
    int RelativeTerm(int term_id);

    // Get all the terminals that are being used
    vector<int> GetActiveTerminals(); 

    // Get the relative terminal positions of each terminal after flipping
    vector<pair<int, int>> GetRelativeTerminals();

    void addNet(Net net);
    vector<Net> m_nets;
    int m_id;
    int m_connectivity;
    Flips m_orientation;
};

class Graph
{
public:
    Graph(string fileName);
    vector<Cell> m_cells;
    int m_cellCount;
    int m_netCount;

    //See if the terminal given is actually being used for a net
    /*
    bool IsTerminalInUse(Terminal TermA) {
        int CellID = TermA.cellId;
        int TermID = TermA.cellId;

        vector<int> terms = m_cells[CellID].GetActiveTerminals();
        for (auto i : terms)
            if (TermID == i)
                return true;

        return false;
    }
    */

    // Given a terminal, find the Net ID that contains it
    int GetNetID(Terminal term)
    {
        int CellID = term.cellId;
        int TermID = term.cellId;
        for (auto i : m_cells[CellID].m_nets) {
            for (int j = 0; j < i.m_connections.size(); j++) {
                if (i.m_connections[j].cellId == CellID)
                    return i.m_id;
            }
        }
        return -1;
    }
    
    // Given a terminal, find the other terminal on the same net
    Terminal GetOtherTerminal(Terminal TermA) {
        int CellID = TermA.cellId;

        //find the net with this terminal
        //return the other terminal on the same net
        for (auto i : m_cells[CellID].m_nets) {
            for (int j = 0; j < i.m_connections.size(); j++) {
                if (i.m_connections[j].cellId == CellID)
                    if (j == 0)
                        return i.m_connections[1];
                    else
                        return i.m_connections[0];
            }
        }

        return Terminal(0, 0); //This should really not happen
    }

};

#endif // !GRAPH_HPP