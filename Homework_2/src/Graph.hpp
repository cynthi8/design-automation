#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>

using namespace std;

#define SPACING_TERMINAL 0
#define UNCONNECTED_TERMINAL -1
#define FEED_THROUGH_TOP_TERMINAL 1
#define FEED_THROUGH_BOTTOM_TERMINAL 3

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

enum TerminalLocation : int
{
    Invalid = -1,
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3
};

struct Terminal
{
    Terminal(string cellId, int terminalId) : cellId(cellId), terminalId(terminalId){};
    string cellId;
    int terminalId;
};

class Net
{
public:
    Net(int id, Terminal terminalA, Terminal terminalB) : m_id(id), m_connections({terminalA, terminalB}){};
    int m_id;
    bool operator==(const Net &rhs) const { return m_id == rhs.m_id; };
    vector<Terminal> m_connections;
};

class Cell
{
public:
    Cell() : m_id("null"), m_connectivity(0), m_orientation(FlipNone){};
    Cell(string id) : m_id(id), m_connectivity(0), m_orientation(FlipNone){};

    // Get the topological location of a terminal by its ID
    const TerminalLocation getTerminalLocation(int term_id) const;

    // Get all the terminals that are being used
    vector<int> GetActiveTerminals();

    vector<Terminal> getSortedTerminals();

    void FlipLeftToRight()
    {
        const unordered_map<Flips, Flips> FlipResult{{FlipNone, FlipLR}, {FlipLR, FlipNone}, {FlipTB, FlipBoth}, {FlipBoth, FlipTB}};
        m_orientation = FlipResult.at(m_orientation);
    }

    void FlipTopToBottom()
    {
        const unordered_map<Flips, Flips> FlipResult{{FlipNone, FlipTB}, {FlipLR, FlipBoth}, {FlipTB, FlipNone}, {FlipBoth, FlipLR}};
        m_orientation = FlipResult.at(m_orientation);
    }

    void addNet(Net netToAdd)
    {
        m_nets.push_back(netToAdd);
        m_connectivity++;
    }
    void removeNet(Net netToRemove)
    {
        m_nets.erase(remove(m_nets.begin(), m_nets.end(), netToRemove), m_nets.end());
        m_connectivity--;
    }

    bool isFeedthrough();
    Terminal getTerminal(int terminalId);
    bool isTerminalTop(Terminal terminal);

    vector<Net> m_nets;
    string m_id;
    int m_connectivity;
    Flips m_orientation;
};

bool isFeedthru(string id);

class Graph
{
public:
    Graph(string fileName);

    string szFileName;

    int m_cellCount;
    int m_netCount;

    map<string, Cell> m_cells;

    void addCell(Cell cell) { m_cells.emplace(cell.m_id, cell); };
    void printTrace(Terminal beginningTerminal);

    // Given a terminal, find the other terminal on the same net
    Terminal getOtherTerminal(Terminal termA);

    // Given a terminal, find the Net ID that contains it
    int GetNetID(Terminal term)
    {
        string CellID = term.cellId;
        int TermID = term.terminalId;

        for (auto net : m_cells[CellID].m_nets)
        {
            for (auto terminal : net.m_connections)
            {
                if (terminal.cellId == CellID && terminal.terminalId == TermID)
                {
                    return net.m_id;
                }
            }
        }
        return UNCONNECTED_TERMINAL;
    }
};

#endif // !GRAPH_HPP