#include "Graph.hpp"
#include <fstream>
#include <cassert>
#include <algorithm>
#include <iostream>

#define FEED_THROUGH_TOP_TERMINAL 1
#define FEED_THROUGH_BOTTOM_TERMINAL 3

using namespace std;

// Construct a netlist graph from a file name
Graph::Graph(string fileName)
{
    ifstream fs;
    //string test = "test.txt";
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
    Terminal otherTerminal = GetOtherTerminal(currentTerminal);
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

vector<pair<TerminalLocation, int>> Cell::getTerminalLocations()
{
    vector<pair<TerminalLocation, int>> terminalLocations;
    vector<int> Terms;

    if(isFeedthrough())
        Terms = { 1, 3 };
    else
        Terms = { 1, 2, 3, 4 };

    for (auto i : Terms)
    {
        TerminalLocation termLoc = getTerminalLocation(i);
        terminalLocations.push_back({termLoc, i});
    }

    //sort based on terminal location
    sort(terminalLocations.begin(), terminalLocations.end());
    return terminalLocations;
}

bool Cell::isFeedthrough()
{
    if (m_id[0] == 'F')
    {
        return true;
    }
    return false;
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