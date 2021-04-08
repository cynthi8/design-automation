#pragma once

#include "Graph.hpp"
#include "Placement.hpp"
#include <algorithm> 
#include <vector>
#include <utility>

using namespace std;

// need a row struct that includes the terminals of cells
// and a vector of tracks
// each track needs what nets will go thru it at different x values
// After all of the tracks are done, I can send the total rows and columns back to placement
// in case there are some more optimizations that can occur.

/*
vector of Spans

Channel is vector of Tracks
Track is vector of Nets
And then there also the top and bottom TerminalRows
*/

// Each track will have a vector of locations that are being used by a net
class Track
{
public:
	vector<Net> m_nets;
	vector<pair<int, int>> m_locs;

	bool TrackOverlap(int Left, int Right)
	{
		for (auto i : this->m_locs)
		{
			//iterate over pairs and see if a or b of the pair is inbetween any other pair
			if ((Left <= i.second || Left >= i.first) ||
				(Right <= i.second || Right >= i.second))
				return true;
			return false;
		}
	}

	bool AddNet(Net net, int left, int right)
	{
		if (!TrackOverlap(left, right))
		{
			this->m_nets.push_back(net);
			this->m_locs.push_back({ left, right });
			return true;
		}
		return false;
	}
};

// Channel has some number of tracks that will be used for routing wires
class Channel
{
	vector<Track> m_tracks;
	void addTrack(Track track)
	{
		m_tracks.push_back(track);
		num_tracks++;
		return;
	}
	int num_tracks;
};

// This is essentially a cell in each row for Routing
class RowCell
{
public:
	RowCell(Terminal term = Terminal(0,0), int NetID = -1) 
	: Term(term), NetID(NetID), AboveCell("",-1), Above(false) {}

	Terminal Term;
	Terminal AboveCell;
	int NetID;
	bool Above;
};

class Row
{
public:
	vector<RowCell> RowCells;
	vector<int> RowNets;
	vector<int> Order;

	// Add a terminal and associated Net ID to the Row Cell vector
	void AddRowVal(Terminal Term, int NetID)
	{
		RowCell cell(Term, NetID);
		RowCells.push_back(cell);
		RowNets.push_back(NetID);
		return;
	}

	// if a net has been ordered, return that value
	int GetUsedNetID(int NetID, int j) {
		for (int k = 0; k < j; k++)
			if (RowCells[k].NetID == NetID)
				return Order[k];
		return -1;
	}

	//Pad Row to colCount
	void PadRow(int colCount) {
		int sizeToGrow = colCount - RowCells.size();
		if (sizeToGrow <= 0) return;

		RowCell rowCell(Terminal("", Invalid), -1);
		RowCells.insert(RowCells.end(), sizeToGrow, rowCell);
		RowNets.insert(RowNets.end(), sizeToGrow, -1);
	}

};

// Top class, to be called by main
class Routing
{
public:
	void Route(Graph graph, Placement place);

	vector<Row> TopRow;
	vector<Row> BotRow;
	Channel Channel;
	//vector<vector<pair<int, int>>> test;

	vector<vector<int>> BuildS(int i, vector<tuple<int, int, int>>& NetsAndXVals);
	tuple<int, int, int> ColumnsCrossed(int i, int j, int netID, bool isTop);
	

	// Set the number of rows, should be +1 than the number given
	void SetRowSize(int rows) {
		this->m_rowCount = rows + 1;
		this->TopRow.resize(this->m_rowCount);
		this->BotRow.resize(this->m_rowCount);
	}

	// Pad the end of the rows with zeros so they all 
	// have the same number of zeros
	void PadRows() {
		for (int i = 0; i < m_colCount; i++) {
			TopRow[i].PadRow(m_colCount);
			BotRow[i].PadRow(m_colCount);
		}
	}

	int m_rowCount;
	int m_colCount;
};