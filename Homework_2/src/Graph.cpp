#include "Graph.hpp"
#include <fstream>
#include <cassert>
#include <algorithm>
#include <iostream>

using namespace std;

// Construct a netlist graph from a file name
Graph::Graph(string fileName)
{
    ifstream fs;
    this->szFileName = fileName;
    fs.open(fileName.c_str(), ios::in);

    if (!fs.is_open())
        throw invalid_argument("Error: File could not be opened.");

    fs >> m_cellCount;
    fs >> m_netCount;

    // Initialize cell list
    for (int i = 1; i < m_cellCount + 1; i++)
    {
        string cellId = to_string(i);
        addCell(Cell(cellId));
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
        //addNet(net);
    }
    fs.close();
}

void Graph::printTrace(Terminal beginningTerminal)
{
    // Start with a terminal
    Terminal currentTerminal = beginningTerminal;
    cout << "Current Cell: " << currentTerminal.cellId << " Current Terminal: " << currentTerminal.terminalId << endl;
    Terminal otherTerminal = getOtherTerminal(currentTerminal);
    cout << "is connected to Cell: " << otherTerminal.cellId << " Terminal: " << otherTerminal.terminalId << endl;

    // If the terminal connects to a feedthrough cell, follow the chain
    currentTerminal = otherTerminal;

    if (m_cells[currentTerminal.cellId].isFeedthrough())
    {
        if (currentTerminal.terminalId == FEED_THROUGH_TOP_TERMINAL)
        {
            currentTerminal = m_cells[currentTerminal.cellId].getTerminal(FEED_THROUGH_BOTTOM_TERMINAL);
        }
        else
        {
            currentTerminal = m_cells[currentTerminal.cellId].getTerminal(FEED_THROUGH_TOP_TERMINAL);
        }
        printTrace(currentTerminal);
    }
}

// Get the topological location of a terminal
const TerminalLocation Cell::getTerminalLocation(int term_id) const
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

vector<Terminal> Cell::getSortedTerminals()
{
    // Returns all terminals on a cell (even those that don't exist on a net)
    // sorted w.r.t TerminalLocation. The order goes [TopLeft, TopRight, BottomLeft, BottomRight]
    vector<Terminal> sortedTerminals;
    vector<int> terminalIds;

    if (isFeedthrough())
        terminalIds = {1, 3};
    else
        terminalIds = {1, 2, 3, 4};

    for (auto terminalId : terminalIds)
    {
        Terminal term = Terminal(m_id, terminalId);
        sortedTerminals.push_back(term);
    }

    // Sort based on terminal location
    sort(sortedTerminals.begin(), sortedTerminals.end(), [this](const Terminal &lhs, const Terminal &rhs) {
        return getTerminalLocation(lhs.terminalId) < getTerminalLocation(rhs.terminalId);
    });
    return sortedTerminals;
}

bool Cell::isFeedthrough()
{
    if (m_id[0] == 'F')
    {
        return true;
    }
    return false;
}

bool Cell::isTerminalTop(Terminal terminal)
{
    TerminalLocation termLoc = getTerminalLocation(terminal.terminalId);
    if (termLoc == TopLeft || termLoc == TopRight)
    {
        return true;
    }
    else
    {
        return false;
    }
}

Terminal Cell::getTerminal(int terminalId)
{
    for (auto &net : m_nets)
    {
        for (auto &terminal : net.m_connections)
        {
            if (terminal.cellId == m_id && terminal.terminalId == terminalId)
            {
                return terminal;
            }
        }
    }
    throw("terminalId is not connected on cell");
}

Terminal Graph::getOtherTerminal(Terminal termA)
{
    string cellId = termA.cellId;
    int terminalId = termA.terminalId;

    // Search the nets on this cell to find termA, and then return the other terminal
    for (auto net : m_cells[cellId].m_nets)
    {
        if (net.m_connections[0].cellId == cellId && net.m_connections[0].terminalId == terminalId)
        {
            return net.m_connections[1];
        }
        else if (net.m_connections[1].cellId == cellId && net.m_connections[1].terminalId == terminalId)
        {
            return net.m_connections[0];
        }
    }

    throw("This terminal is not part of a net");
}