// Routing

#include "Placement.hpp"
#include "Graph.hpp"
#include "Routing.hpp"

Routing::Routing(Placement place)
{
	// Build the terminal rows
	BuildRows(place);

	// Pad the rows with zeros
	PadRows();

	//for each row, build it up
	m_channels.resize(m_channelCount);
	m_Spans.resize(m_channelCount);
	for (int i = 0; i < m_channelCount; i++)
	{
		// Build the range first in case we have to change it for the V graph
		vector<Span> Spans;
		BuildSpans(i, Spans);

		// Build the V Graph
		vector<vector<int>> V;
		BuildV(i, V);

		// Fix Doglegs by changing range
		FixDogLegs(i, V, Spans);

		// Build the S / H Graph
		vector<SSet> S(m_colCount);
		BuildS(i, S, Spans);

		// Finally Route the nets
		RouteNets(i, S, V, Spans);

		m_Spans[i] = Spans;
	}

	return;
}

// Build the Rows of Cells and Terminals
void Routing::BuildRows(Placement &place)
{
	//This function needs to populate the rows such that we can do channel routing
	int i, j;
	int maxCols = 0;

	vector<pair<int, int>> Terminals;
	string cell_id;

	int placeHeight = (int)place.m_grid.m_grid.size();
	SetRowSize(placeHeight);

	// go through the entire 2D grid that's been placed
	for (i = 0; i < placeHeight; i++)
	{
		Row topTerminalRowTemp;
		Row bottomTerminalRowTemp;
		for (j = 0; j < place.m_grid.m_grid[i].size(); j++)
		{
			// Get the cell id
			cell_id = place.m_grid[Location(i, j)].m_cellId;
			Cell cell = place.m_netlist.m_cells[cell_id];

			// Add terminals and nets to row in order
			for (auto terminal : cell.getSortedTerminals())
			{
				// Even if the terminal is not connected, add it to the terminal row
				int NetID = place.m_netlist.GetNetID(terminal);
				if (cell.isTerminalTop(terminal))
					// If the terminal is at the top of the cell, it will be at the bottom of the channel
					bottomTerminalRowTemp.AddRowVal(terminal, NetID);
				else
					topTerminalRowTemp.AddRowVal(terminal, NetID);
			}
		}

		// add the temp rows to the array
		// m_BotRow[0] should be empty
		// m_TopRow[m_channelCount] should be empty
		this->m_BotRow[i + 1] = bottomTerminalRowTemp;
		this->m_TopRow[i] = topTerminalRowTemp;

		if (m_TopRow[i].RowCells.size() > maxCols)
			maxCols = (int)m_TopRow[i].RowCells.size();
		if (m_BotRow[i].RowCells.size() > maxCols)
			maxCols = (int)m_BotRow[i].RowCells.size();
	}

	this->m_colCount = maxCols;
}

// Route the nets through the channel
void Routing::RouteNets(int i, vector<SSet> &S, vector<vector<int>> V, vector<Span> &Spans)
{
	vector<int> &rowT = m_TopRow[i].RowNets;
	vector<int> &rowB = m_BotRow[i].RowNets;

	Track track;

	vector<vector<bool>> netsRangesDone(Spans.size());
	vector<bool> netsDone(Spans.size());
	map<int, int> NetTracks;

	for (int j = 0; j < Spans.size(); j++)
	{
		int k = Spans[j].ranges.size();
		Spans[j].n_tracks.resize(k);
		netsRangesDone[j].insert(netsRangesDone[j].begin(), Spans[j].ranges.size(), false);
		NetTracks.insert({Spans[j].net, -1});
	}

	int j = 0;
	bool Done = false;

	if (Spans.size() == 0)
	{
		Done = true;
	}

	//While all nets haven't been placed
	while (!Done)
	{
		//If this net has been completely placed, go to next net
		if (netsDone[j])
		{
			j++;
			continue;
		}

		int netID = Spans[j].net;

		//Go through all spans on this net
		for (int rangeID = 0; rangeID < Spans[j].ranges.size(); rangeID++)
		{
			bool inS = false;
			bool inV = false;
			int removefromV = -1;
			int Vidx = -1;
			pair<int, int> range = Spans[j].ranges[rangeID];

			//Check if this net is in a VCG
			for (int k = 0; k < V.size(); k++)
			{
				//if the value is in the VCG and isn't the very last one, then skip it
				auto iter = find(V[k].begin(), V[k].end(), netID);
				if (iter < V[k].end() - 1)
				{
					inV = true;
					break;
				}
				//else if it is the last one, then we are good to remove it from V
				else if (iter == V[k].end() - 1)
				{
					removefromV = (int)(iter - V[k].begin());
					Vidx = k;
				}
				//else its not in it
			}

			//value is in a VCG
			if (inV)
				continue;

			//Check if this net and its range is in S
			int maxtrack = 0;
			for (int k = 0; k < S.size(); k++)
			{
				//if we find the net in S, then it must be on a different track than the rest
				if (S[k].nets.find({netID, rangeID}) != S[k].nets.end() && S[k].nets.size() > 1)
				{
					//go through all nets
					for (auto l : S[k].nets)
						//find the largest track a net in this S has already been assigned to
						if (NetTracks[l.first] > maxtrack)
							maxtrack = NetTracks[l.first] + 1;

					break;
				}
			}

			//Set this net and track to this available track
			NetTracks[netID] = maxtrack;

			//resize the channel if this track is larger than the number of elements in it
			if (maxtrack >= m_channels[i].m_tracks.size())
				m_channels[i].m_tracks.resize(maxtrack + 1);

			//add this net and its range to the track
			m_channels[i].m_tracks[maxtrack].AddNet(netID, range);

			//If we need to, remove this net from the VCG
			if (Vidx >= 0)
				V[Vidx].erase(V[Vidx].begin() + removefromV);

			//This specific range is done
			netsRangesDone[j][rangeID] = true;

			//keep the assigned track of this span for Magic
			Spans[j].n_tracks[rangeID] = maxtrack;
		}

		//Go thru all spans on this net, if any aren't done, this set netsDone to false
		netsDone[j] = true;
		for (auto k : netsRangesDone[j])
		{
			if (k != true)
			{
				netsDone[j] = false;
				break;
			}
		}

		//Go thru all netsDone, if any aren't done, this set Done to false
		Done = true;
		for (auto k : netsDone)
		{
			if (k != true)
			{
				Done = false;
				break;
			}
		}

		//Increment j, and see if its larger than the number of nets, if so wrap back around
		if (++j >= Spans.size())
			j = 0;
	}

	return;
}

// Fix any doglegs that appear by splitting the range of the nets
void Routing::FixDogLegs(int i, vector<vector<int>> &V, vector<Span> &Spans)
{
	vector<int> &rowT = m_TopRow[i].RowNets;
	vector<int> &rowB = m_BotRow[i].RowNets;

	//Go though all values in the V graph
	for (auto i = 0; i < V.size(); i++)
	{

		//if the first and last element are the same, then we have a dogleg
		if (V[i][0] == V[i].back())
		{
			//split the second to last net's x range into two
			int netIDProb = *(V[i].rbegin() + 1);
			int netIDEnd = *(V[i].rbegin());

			auto iter = find_if(Spans.begin(), Spans.end(),
								[netIDProb](Span const &item) { return item.net == netIDProb; });
			int idx = (int)(iter - Spans.begin());
			pair<int, int> ORange = Spans[idx].ranges[0];

			//Need to find an empty place to split it
			int j;
			for (j = ORange.first; j <= ORange.second; j++)
			{
				if (rowT[j] <= 0 || rowB[j] <= 0)
					break;
			}
			pair<int, int> NewRange1 = {ORange.first, j};
			pair<int, int> NewRange2 = {j, ORange.second};
			Spans[idx].ranges[0] = NewRange1;
			Spans[idx].ranges.push_back(NewRange2);

			//remove the last two elements causing the dogleg problem
			V[i].pop_back();
			V[i].pop_back();
			V.push_back({netIDProb, netIDEnd});
		}
	}

	return;
}

// Build the VCG Graph
void Routing::BuildV(int i, vector<vector<int>> &V)
{
	//vector<vector<int>> V;
	vector<int> &rowT = m_TopRow[i].RowNets;
	vector<int> &rowB = m_BotRow[i].RowNets;

	//find the range of every net
	for (int j = 0; j < m_colCount; j++)
	{
		int netIDT = rowT[j];
		int netIDB = rowB[j];

		if (netIDT > 0 && netIDB > 0)
		{
			for (int k = 0; k < V.size(); k++)
			{
				if (netIDB == V[k][0])
				{
					V[k].insert(V[k].begin(), netIDT);
					goto EndOuterForLoop;
				}
				else
				{
					for (int l = 0; l < V[k].size(); l++)
					{
						if (V[k][l] == netIDT)
						{
							V[k].push_back(netIDB);
							goto EndOuterForLoop;
						}
					}
				}
			}
			V.push_back({netIDT, netIDB});
		}

	EndOuterForLoop:
		continue;
	}

	return;
}

// Build the spans for a channel
void Routing::BuildSpans(int channelIndex, vector<Span> &Spans)
{
	vector<int> &rowT = m_TopRow[channelIndex].RowNets;
	vector<int> &rowB = m_BotRow[channelIndex].RowNets;

	//find the range of every net
	for (int col = 0; col < m_colCount; col++)
	{
		int netID = rowT[col];
		if (netID != UNCONNECTED_TERMINAL)
		{
			// Find if there is a span with this netID in Spans already
			auto iter = find_if(Spans.begin(), Spans.end(), [netID](Span const &span) {
				return span.net == netID;
			});

			// If the netID isn't in spans, add it
			if (iter == Spans.end())
			{
				Spans.push_back(CalculateSpan(channelIndex, col, netID, true));
			}
		}

		netID = rowB[col];
		if (netID != UNCONNECTED_TERMINAL)
		{
			auto iter = find_if(Spans.begin(), Spans.end(),
								[netID](Span const &item) { return item.net == netID; });

			//if the net doesn't exist in the list, add it
			if (iter == Spans.end())
			{
				Spans.push_back(CalculateSpan(channelIndex, col, netID, false));
			}
		}
	}
}

// Find the columns a particular net crosses
Span Routing::CalculateSpan(int channelIndex, int col, int netID, bool isTop)
{
	//Get the X columns for each net
	vector<int> &rowT = m_TopRow[channelIndex].RowNets;
	vector<int> &rowB = m_BotRow[channelIndex].RowNets;

	//in case the other terminal for the net is actually in the same column
	int TopAdj = 0, BotAdj = 0;
	if (isTop)
		TopAdj++;
	else
		BotAdj++;

	vector<int>::iterator topIter = rowT.begin() + col + TopAdj;
	vector<int>::iterator bottomIter = rowB.begin() + col + BotAdj;

	int x2 = -1;

	//look through top and bottom row for the next instance of the id
	topIter = find(topIter, rowT.end(), netID);
	bottomIter = find(bottomIter, rowB.end(), netID);

	//net is in top
	if (topIter != rowT.end())
	{
		x2 = topIter - rowT.begin();
	}
	//net is in bottom
	else if (bottomIter != rowB.end())
	{
		x2 = bottomIter - rowB.begin();
	}

	if (x2 == -1)
		throw invalid_argument("Placement is borked; Missing Nets in Row.");

	return Span(netID, {{col, x2}});
}

// Build the HCG Graph, ie what nets cannot be on the same track as one another
void Routing::BuildS(int i, vector<SSet> &S, const vector<Span> &NetsAndXVals)
{
	//if the range of the net overlaps a column, add it to the HGraph
	for (int j = 0; j < m_colCount; j++)
	{ //all cols
		for (auto &k : NetsAndXVals)
		{ //all nets and x ranges
			int iter = 0;
			for (auto &l : k.ranges)
			{ //all ranges for each net
				if (l.first >= j && l.second <= j)
				{ //check if net's x ranges overlap this col
					//S[j].insert({ k.net,iter });		//push net
					S[j].addSet(j, {k.net, iter}); //push net
				}
				iter++;
			}
		}
	}

	// Mark any non-maximial sets to be removed
	set<int> columnsToRemove;
	for (int j = 0; j < S.size(); j++)
	{
		for (int k = 0; k < S.size(); k++)
		{
			//check each set to see if its contained in another set
			if (includes(S[j].nets.begin(), S[j].nets.end(),
						 S[k].nets.begin(), S[k].nets.end()))
			{
				columnsToRemove.insert(j);
			}
		}
	}

	// Remove columns from the back of the vector
	for (auto it = columnsToRemove.rbegin(); it != columnsToRemove.rend(); it++)
	{
		S.erase(S.begin() + (*it));
	}
}

// Debug Printing I suppose
void Routing::Print()
{
	for (auto i : m_channels)
	{
		for (auto j : i.m_tracks)
		{
			for (auto k = 0; k < j.m_nets.size(); k++)
			{
				int netID = j.m_nets[k];
				pair<int, int> locs = j.m_locs[k];

				cout << netID << ": " << locs.first << ", " << locs.second << " ";
			}
		}
		cout << endl;
	}
	return;
}