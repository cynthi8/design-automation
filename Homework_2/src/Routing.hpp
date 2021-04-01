#pragma once

#include "Graph.hpp"
#include "Placement.hpp"

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
class Track
{
public:
	Track(int id, int left, int right) : m_nets(id), m_locs.push_back({left, right}){};
	//Net (int id, Terminal terminalA, Terminal terminalB) : m_id(id), m_connections({ terminalA, terminalB }) {};
	vector<Net> m_nets;
	vector<pair<int, int>> m_locs;

	bool TrackOverlap(pair<int, int> PairAB)
	{
		for (auto i : m_locs)
		{
			//iterate over pairs and see if a or b of the pair is inbetween any other pair
			if ((PairAB.first <= i.second || PairAB.first >= i.first) ||
				(PairAB.second <= i.second || PairAB.first >= i.second))
				return true;
			return false;
		}
	}
};

class Channel
{
	vector<Track> m_tracks;
	void addTrack(Track track)
	{
		m_tracks.push_back(track);
		num_tracks++;
	}
	int num_tracks;
};

class Row
{
public:
	vector<Terminal> m_Top;
	vector<Terminal> m_Bot;
	Channel m_channel;
};

class Routing
{
public:
	void Route(Graph graph, Placement place);
	vector<Row> Array;

	int m_rowCount;
	int m_colCount;
};

// Create a graph for VCG
class VNet
{
public:
	VNet(VCell cellA, VCell cellB) : Owner(cellA.m_id), Object(cellB.m_id){};
	int Owner;
	int Object;
};

class VCell
{
public:
	VCell(int id) : m_id(id), m_connectivity(0){};

	void addCellConnection(VCell cell)
	{
		this->m_nets.push_back(*this, cell);
		this->m_connectivity++;
	};

	vector<VNet> m_nets;

	int m_id;
	int m_connectivity;
};

class VGraph
{
public:
	vector<VCell> m_cells;
	int m_cellCount;
};