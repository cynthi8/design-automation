#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <map>
#include <unordered_map>
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
    bool IsTerminalTop()
    {
        if (terminalId == TopLeft || terminalId == TopRight)
            return true;
        return false;
    }
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

    // Get the topological location of all the terminals
    vector<pair<TerminalLocation, int>> getTerminalLocations();

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

    vector<Net> m_nets;
    string m_id;
    int m_connectivity;
    Flips m_orientation;
};

class Graph
{
public:
    Graph(string fileName);
    int m_cellCount;
    int m_netCount;

    map<string, Cell> m_cells;
    //vector<Net> m_nets;

    void addCell(Cell cell) { m_cells.emplace(cell.m_id, cell); };
    void printTrace(Terminal beginningTerminal);

    /*
    void addNet(Net netToAdd) { 
        m_nets.push_back(netToAdd); 
    }
    void removeNet(Net netToRemove) { 
        m_nets.erase(remove(m_nets.begin(), m_nets.end(), netToRemove), m_nets.end()); 
    }
    */

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

        return -1;
        //throw;
    }

    // Given a terminal, find the other terminal on the same net
    Terminal GetOtherTerminal(Terminal TermA)
    {
        string cellID = TermA.cellId;
        int correctNetId = GetNetID(TermA);
        for (auto net : m_cells[cellID].m_nets)
        {
            if (net.m_id != correctNetId)
            {
                continue;
            }

            Terminal firstTerminal = net.m_connections[0];
            if (firstTerminal.cellId == TermA.cellId && firstTerminal.terminalId == TermA.terminalId)
            {
                return net.m_connections[1];
            }
            else
            {
                return net.m_connections[0];
            }
        }

        throw; //This should really not happen
    }
};

#endif // !GRAPH_HPP