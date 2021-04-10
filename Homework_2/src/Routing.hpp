#pragma once

#include "Graph.hpp"
#include "Placement.hpp"
#include <algorithm>
#include <vector>
#include <utility>
#include <cstdlib>
#include <algorithm>
#include <set>
#include <iostream>

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
	vector<int> m_nets;
	vector<pair<int, int>> m_locs;

	bool TrackOverlap(pair<int, int> locs)
	{
		int Left = locs.first;
		int Right = locs.second;
		for (auto i : this->m_locs)
		{
			//iterate over pairs and see if a or b of the pair is inbetween any other pair
			if ((Left <= i.second || Left >= i.first) ||
				(Right <= i.second || Right >= i.second))
				return true;
			return false;
		}
		return false;
	}

	bool AddNet(int net, pair<int, int> locs)
	{
		if (!TrackOverlap(locs))
		{
			this->m_nets.push_back(net);
			this->m_locs.push_back(locs);
			return true;
		}
		return false;
	}
};

// Channel has some number of tracks that will be used for routing wires
class Channel
{
public:
	vector<Track> m_tracks;
};

// This is essentially a cell in each row for Routing
class RowCell
{
public:
	RowCell(Terminal term = Terminal(0, 0), int NetID = -1)
		: Term(term), NetID(NetID), AboveCell("", -1), Above(false) {}

	Terminal Term;
	int NetID;
	Terminal AboveCell;
	bool Above;
};

class Row
{
public:
	vector<RowCell> RowCells;
	vector<int> RowNets;
	//vector<int> Order;

	// Add a terminal and associated Net ID to the Row Cell vector
	void AddRowVal(Terminal Term, int NetID)
	{
		RowCell cell(Term, NetID);
		RowCells.push_back(cell);
		RowNets.push_back(NetID);
		return;
	}

	// if a net has been ordered, return that value
	//int GetUsedNetID(int NetID, int j)
	//{
	//	for (int k = 0; k < j; k++)
	//		if (RowCells[k].NetID == NetID)
	//			return Order[k];
	//	return -1;
	//}

	//Pad Row to colCount
	void PadRow(int colCount)
	{
		int sizeToGrow = colCount - (int)RowCells.size();
		if (sizeToGrow <= 0)
			return;

		RowCell rowCell(Terminal("", Invalid), -1);
		RowCells.insert(RowCells.end(), sizeToGrow, rowCell);
		RowNets.insert(RowNets.end(), sizeToGrow, -1);
	}
};

class NetAndRanges
{
public:
	NetAndRanges(int net, vector<pair<int, int>> Range)
		: net(net), ranges(Range) {}

	int net;
	vector<pair<int, int>> ranges;
	vector<int> n_tracks;
};

class SSet
{
public:
	SSet() : colID(-1) {}
	SSet(int colID) : colID(colID) {}

	int colID;				  //need to keep track of the column this originated from
	set<pair<int, int>> nets; //all of the nets that cross this column

	void addSet(int colID, pair<int, int> sets)
	{
		this->colID = colID;
		nets.insert(sets);
	}
};

// Top class, to be called by main
class Routing
{
public:
	Routing(Placement place);

	vector<Row> m_TopRow;
	vector<Row> m_BotRow;
	vector<Channel> m_channels;
	vector<vector<NetAndRanges>> m_netRanges;

	void BuildRows(Placement &place);

	NetAndRanges ColumnsCrossed(int i, int j, int netID, bool isTop);
	void BuildRange(int i, vector<NetAndRanges> &NetsAndXVals);
	void BuildS(int i, vector<SSet> &S, const vector<NetAndRanges> &NetsAndXVals);
	void BuildV(int i, vector<vector<int>> &V);
	void FixDogLegs(int i, vector<vector<int>> &V, vector<NetAndRanges> &NetsAndXRanges);
	void RouteNets(int i, vector<SSet> &S, vector<vector<int>> &V, vector<NetAndRanges> &NetsAndXRanges);

	void Print();

	// Set the number of rows, should be +1 than the number given
	void SetRowSize(int rows)
	{
		this->m_rowCount = rows + 1;
		this->m_TopRow.resize(this->m_rowCount);
		this->m_BotRow.resize(this->m_rowCount);
	}

	// Pad the end of the rows with zeros so they all
	// have the same number of zeros
	void PadRows()
	{
		for (int i = 0; i < m_rowCount; i++)
		{
			m_TopRow[i].PadRow(m_colCount);
			m_BotRow[i].PadRow(m_colCount);
		}
	}

	int m_rowCount;
	int m_colCount;
};