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
    m_cells.reserve(m_cellCount);
    for (int i = 1; i < m_cellCount + 1; i++)
    {
        string cellId = to_string(i);
        m_cells[cellId] = Cell(cellId);
    }

    for (int i = 1; i < m_netCount + 1; i++)
    {
        int netID;
        string cellAId, cellBId;
        int termA, termB;

        // Parse line
        fs >> netID;
        assert(netID == i);
        fs >> cellAId;
        fs >> termA;
        fs >> cellBId;
        fs >> termB;

        // Create Terminals and Nets
        Terminal terminalA{cellAId, termA};
        Terminal terminalB{cellBId, termB};
        Net net(netID, terminalA, terminalB);

        // Add Nets to Cells
        m_cells[cellAId].addNet(net);
        m_cells[cellBId].addNet(net);
    }
    fs.close();
}

void Cell::addNet(Net net)
{
    m_nets.push_back(net);
    m_connectivity++;
}

void Cell::FlipLeftToRight()
{
    const unordered_map<Flips, Flips> FlipResult{{FlipNone, FlipLR}, {FlipLR, FlipNone}, {FlipTB, FlipBoth}, {FlipBoth, FlipTB}};
    m_orientation = FlipResult.at(m_orientation);
}

void Cell::FlipTopToBottom()
{
    const unordered_map<Flips, Flips> FlipResult{{FlipNone, FlipTB}, {FlipLR, FlipBoth}, {FlipTB, FlipNone}, {FlipBoth, FlipLR}};
    m_orientation = FlipResult.at(m_orientation);
}

// Flip the terminal number
TerminalLocation Cell::getTerminalLocation(int term_id)
{
    // Map terminal ids to topological locations
    const unordered_map<int, TerminalLocation> FlipNone_Map{{1, TopLeft}, {2, TopRight}, {3, BottomLeft}, {4, BottomRight}};
    const unordered_map<int, TerminalLocation> FlipLR_Map{{2, TopLeft}, {1, TopRight}, {4, BottomLeft}, {3, BottomRight}};
    const unordered_map<int, TerminalLocation> FlipTB_Map{{3, TopLeft}, {4, TopRight}, {1, BottomLeft}, {2, BottomRight}};
    const unordered_map<int, TerminalLocation> FlipBoth_Map{{4, TopLeft}, {3, TopRight}, {2, BottomLeft}, {1, BottomRight}};
    switch (this->m_orientation)
    {
    case FlipLR:
        return FlipLR_Map.at(term_id);
        break;

    case FlipTB:
        return FlipTB_Map.at(term_id);
        break;

    case FlipBoth:
        return FlipBoth_Map.at(term_id);
        break;

    default:
        return FlipNone_Map.at(term_id);
        break;
    }
}

// Get all the terminals that are being used
vector<int> Cell::GetActiveTerminals()
{
    vector<int> Terminals;
    for (auto k : this->m_nets)
    {
        for (auto l : k.m_connections)
        {
            if (l.cellId == this->m_id)
            {
                Terminals.push_back(l.terminalId);
            }
        }
    }
    return Terminals;
}

vector<pair<TerminalLocation, int>> Cell::getTerminalLocations()
{
    vector<pair<TerminalLocation, int>> terminalLocations;
    vector<int> Terms{1, 2, 3, 4};
    for (auto i : Terms)
    {
        TerminalLocation termLoc = getTerminalLocation(i);
        terminalLocations.push_back({termLoc, i});
    }

    //sort based on terminal location
    sort(terminalLocations.begin(), terminalLocations.end());
    return terminalLocations;
}