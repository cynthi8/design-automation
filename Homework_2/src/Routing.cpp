// Routing

#include "Placement.hpp"
#include "Graph.hpp"
#include "Routing.hpp"

Routing::Routing(Placement place)
{
	//This function needs to populate the rows such that we can do channel routing
	int i;
	int j;
	string cell_id;
	int maxCols = 0;

	vector<pair<int, int>> Terminals;
	int placeHeight = (int)place.m_grid.m_grid.size();
	SetRowSize(placeHeight);

	// go through the entire 2D grid that's been placed
	for (i = 0; i < placeHeight; i++)
	{
		Row RowTopTemp;
		Row RowBottomTemp;
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
					RowTopTemp.AddRowVal(terminal, NetID);
				else
					RowBottomTemp.AddRowVal(terminal, NetID);
			}
		}

		// add the temp rows to the array
		// the top most row should be empty
		// the bottom most row should be empty
		this->m_BotRow[i] = RowBottomTemp;
		this->m_TopRow[i + 1] = RowTopTemp;

		if (m_TopRow[i].RowCells.size() > maxCols)
			maxCols = (int)m_TopRow[i].RowCells.size();
		if (m_BotRow[i].RowCells.size() > maxCols)
			maxCols = (int)m_BotRow[i].RowCells.size();
	}

	this->m_colCount = maxCols;

	PadRows(); //pad the rows with zeros

	//for each row, build it up
	m_channels.resize(m_rowCount);
	for (i = 0; i < m_rowCount; i++)
	{

		// Build the range first in case we have to change it for the V graph
		vector<NetAndRanges> NetsAndXRanges;
		BuildRange(i, NetsAndXRanges);

		// Build the V Graph
		vector<vector<int>> V;
		BuildV(i, V);

		// Fix Doglegs by changing range
		FixDogLegs(i, V, NetsAndXRanges);

		// Build the S / H Graph
		vector<SSet> S(m_colCount);
		BuildS(i, S, NetsAndXRanges);

		//TODO route
		RouteNets(i, S, V, NetsAndXRanges);
	}

	return;
}

void Routing::RouteNets(int i, vector<SSet> &S, vector<vector<int>> &V, vector<NetAndRanges> &NetsAndXRanges)
{
	vector<int> &rowT = m_TopRow[i].RowNets;
	vector<int> &rowB = m_BotRow[i].RowNets;

	Track track;

	vector<vector<bool>> netsRangesDone(NetsAndXRanges.size());
	vector<bool> netsDone(NetsAndXRanges.size());
	map<int, int> NetTracks;

	for (int j = 0; j < NetsAndXRanges.size(); j++)
	{
		netsRangesDone[j].insert(netsRangesDone[j].begin(), NetsAndXRanges[j].ranges.size(), false);
		NetTracks.insert({NetsAndXRanges[j].net, -1});
	}
	int j = 0;
	bool Done = false;

	while (Done)
	{
		if (netsDone[j])
		{
			j++;
			continue;
		}
		int netID = NetsAndXRanges[j].net;

		for (int rangeID = 0; rangeID < NetsAndXRanges[j].ranges.size(); rangeID++)
		{
			bool inS = false;
			bool inV = false;
			int removefromV = -1;
			int Vidx = -1;
			pair<int, int> range = NetsAndXRanges[j].ranges[rangeID];

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
				else if (iter == V[k].end() - 1)
				{
					removefromV = (int)(iter - V[k].begin());
					Vidx = k;
				}
				//else its not in it
			}

			if (inV)
				continue;

			//Check if this net and its range is in S
			int maxtrack = 0;
			for (int k = 0; k < S.size(); k++)
			{
				if (S[k].nets.find({netID, rangeID}) != S[k].nets.end() && S[k].nets.size() > 1)
				{
					for (auto l : S[k].nets)
						if (NetTracks[l.first] > maxtrack)
							maxtrack = NetTracks[l.first] + 1;

					break;
				}
			}

			NetTracks[netID] = maxtrack;
			if (maxtrack > m_channels[i].m_tracks.size())
				m_channels[i].m_tracks.resize(maxtrack + 1);

			m_channels[i].m_tracks[maxtrack].AddNet(netID, range);
			if (Vidx >= 0)
				V[Vidx].erase(V[Vidx].begin() + removefromV);

			netsRangesDone[j][rangeID] = true;
		}

		netsDone[j] = true;
		for (auto k : netsRangesDone[j])
		{
			if (k != true)
			{
				netsDone[j] = false;
				break;
			}
		}

		Done = true;
		for (auto k : netsDone)
		{
			if (k != true)
			{
				Done = false;
				break;
			}
		}

		if (++j >= NetsAndXRanges.size())
			j = 0;
	}

	return;
}

//how do I want to fix the dog legs
//I suppose I can just change the x range for the second to last element
// 1-2-3-4-1 -> 1-2-3-4a(x1,x2a or something), 4b(x2b,x3)-1
// 1-2-1 -> 1-2b(x1,x2a), 2b(x2b, x3)-1
// 1(range), 2(two ranges)
void Routing::FixDogLegs(int i, vector<vector<int>> &V, vector<NetAndRanges> &NetsAndXRanges)
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

			auto iter = find_if(NetsAndXRanges.begin(), NetsAndXRanges.end(),
								[netIDProb](NetAndRanges const &item) { return item.net == netIDProb; });
			int idx = (int)(iter - NetsAndXRanges.begin());
			pair<int, int> ORange = NetsAndXRanges[idx].ranges[0];

			//Need to find an empty place to split it
			int j;
			for (j = ORange.first; j <= ORange.second; j++)
			{
				if (rowT[j] <= 0 || rowB[j] <= 0)
					break;
			}
			pair<int, int> NewRange1 = {ORange.first, j};
			pair<int, int> NewRange2 = {j, ORange.second};
			NetsAndXRanges[idx].ranges[0] = NewRange1;
			NetsAndXRanges[idx].ranges.push_back(NewRange2);

			//remove the last two elements causing the dogleg problem
			V[i].pop_back();
			V[i].pop_back();
			V.push_back({netIDProb, netIDEnd});
		}
	}

	return;
}

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

void Routing::BuildRange(int i, vector<NetAndRanges> &NetsAndXRanges)
{
	vector<int> &rowT = m_TopRow[i].RowNets;
	vector<int> &rowB = m_BotRow[i].RowNets;

	//find the range of every net
	for (int j = 0; j < m_colCount; j++)
	{
		int netID = rowT[j];
		if (netID > 0)
		{
			auto iter = find_if(NetsAndXRanges.begin(), NetsAndXRanges.end(),
								[netID](NetAndRanges const &item) { return item.net == netID; });

			//if the net doesn't exist in the list, add it
			if (iter == NetsAndXRanges.end())
			{
				NetsAndXRanges.push_back(ColumnsCrossed(i, j, netID, true));
			}
		}

		netID = rowB[j];
		if (netID > 0)
		{
			auto iter = find_if(NetsAndXRanges.begin(), NetsAndXRanges.end(),
								[netID](NetAndRanges const &item) { return item.net == netID; });

			//if the net doesn't exist in the list, add it
			if (iter == NetsAndXRanges.end())
			{
				NetsAndXRanges.push_back(ColumnsCrossed(i, j, netID, false));
			}
		}
	}
}

NetAndRanges Routing::ColumnsCrossed(int i, int j, int netID, bool isTop)
{
	//Get the X columns for each net
	vector<int> &rowT = m_TopRow[i].RowNets;
	vector<int> &rowB = m_BotRow[i].RowNets;

	//in case the other terminal for the net is actually in the same column
	int TopAdj = 0, BotAdj = 0;
	if (isTop)
		TopAdj++;
	else
		BotAdj++;

	vector<int>::iterator iter1 = rowT.begin() + j + TopAdj;
	vector<int>::iterator iter2 = rowB.begin() + j + BotAdj;

	int x2 = -1;

	//look through top and bottom row for the next instance of the id
	iter1 = find(iter1, rowT.end(), netID);
	iter2 = find(iter2, rowB.end(), netID);

	//net is only in top
	if (iter1 != rowT.end())
	{
		x2 = (int)(find(iter1, rowT.end(), netID) - rowB.begin());
	}
	//net is only in bottom
	else if (iter2 != rowB.end())
	{
		x2 = (int)(find(iter2, rowB.end(), netID) - rowB.begin());
	}

	if (x2 == -1)
		throw invalid_argument("Placement is borked; Missing Nets in Row.");

	return NetAndRanges(netID, {{j, x2}});
}

void Routing::BuildS(int i, vector<SSet> &S, vector<NetAndRanges> &NetsAndXVals)
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

	vector<int> DeleteS;

	//check each set to see if its contained in another set
	for (int j = 0; j < S.size(); j++)
	{
		/*
		if (S[j].nets.size() < 2) {
			DeleteS.push_back(j);
		}
		*/

		for (int k = 0; k < S.size(); k++)
		{
			if (includes(S[j].nets.begin(), S[j].nets.end(),
						 S[k].nets.begin(), S[k].nets.end()))
			{

				DeleteS.push_back(j);
			}
		}
	}

	//if a set is contained within another, delete it
	for (int j = (int)(DeleteS.size() - 1); j >= 0; j--)
		S.erase(S.begin() + DeleteS[j]);
	//remove(S.begin(), S.end(), DeleteS[j]);

	return;
}

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