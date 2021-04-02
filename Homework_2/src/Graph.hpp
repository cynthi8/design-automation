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
};

#endif // !GRAPH_HPP