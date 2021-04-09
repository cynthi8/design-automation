// Routing

#include "Placement.hpp"
#include "Graph.hpp"
#include "Routing.hpp"

#include <tuple>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <set>

void Routing::Route(Graph graph, Placement place)
{
	//This function needs to populate the rows such that we can do channel routing
	int i;
	int j;
	string cell_id;
	int maxCols = 0;

	Row RowTopTemp;
	Row RowBottomTemp;
	vector<pair<int, int>> Terminals;
	SetRowSize(place.m_grid.m_rows);

	// go through the entire 2D grid that's been placed
	for (i = 0; i < place.m_gridHeight; i++) {
		for (j = 0; i < place.m_gridWidth; j++) {

			//Get the cell id
			cell_id = place.m_grid[Location::Location(i, j)].m_cellId;
			Cell cell = place.m_netlist.m_cells[cell_id];

			//find all the terminals on this cell
			auto Terminals = cell.getTerminalLocations();

			//add terminals and nets to row in order
			for (auto terms : Terminals) {
				Terminal term(cell_id, terms.first);
				int NetID = place.m_netlist.GetNetID(term);

				if (term.IsTerminalTop())
					RowTopTemp.AddRowVal(term, NetID);
				else
					RowBottomTemp.AddRowVal(term, NetID);
			}
		}

		// add the temp rows to the array
		// the top most row should be empty
		// the bottom most row should be empty
		this->BotRow[i] = RowBottomTemp;
		this->TopRow[i + 1] = RowTopTemp;

		if (TopRow[i].RowCells.size() > maxCols)
			maxCols = TopRow[i].RowCells.size();
		if (BotRow[i].RowCells.size() > maxCols)
			maxCols = BotRow[i].RowCells.size();
	}

	this->m_colCount = maxCols;

	PadRows(); //pad the rows with zeros

	//for each row, build it up
	for (i = 0; i < m_rowCount; i++) {
		
		// Build the range first in case we have to change it for the V graph
		vector<NetAndRanges> NetsAndXRanges;
		BuildRange(i, NetsAndXRanges);

		// Build the V Graph
		vector<vector<int>> V;
		BuildV(i, V);

		// Fix Doglegs by changing range
		FixDogLegs(i, V, NetsAndXRanges);

		// Build the S / H Graph
		vector<set<pair<int, int>>> S(m_colCount);
		BuildS(i, S, NetsAndXRanges);

		//TODO route
		RouteNets(i, S, V, NetsAndXRanges);
	}

	return;
}

void Routing::RouteNets(int i, vector<set<pair<int, int>>>& S, vector<vector<int>>& V, vector<NetAndRanges>& NetsAndXRanges) 
{

	

	return;
}

//how do I want to fix the dog legs
//I suppose I can just change the x range for the second to last element
// 1-2-3-4-1 -> 1-2-3-4a(x1,x2a or something), 4b(x2b,x3)-1
// 1-2-1 -> 1-2b(x1,x2a), 2b(x2b, x3)-1
// 1(range), 2(two ranges)
void Routing::FixDogLegs(int i, vector<vector<int>>& V, vector<NetAndRanges>& NetsAndXRanges) 
{
	vector<int>& rowT = TopRow[i].RowNets;
	vector<int>& rowB = BotRow[i].RowNets;

	//Go though all values in the V graph
	for (auto i = 0; i < V.size(); i++) {

		//if the first and last element are the same, then we have a dogleg
		if (V[i][0] == V[i].back()) {
			//split the second to last net's x range into two
			int netIDProb = V[i].back() - 1;
			int netIDEnd = V[i].back();

			auto iter = find_if(NetsAndXRanges.begin(), NetsAndXRanges.end(),
				[netIDProb](NetAndRanges const& item) {return item.net == netIDProb; });
			int idx = iter - NetsAndXRanges.begin();
			pair<int, int> ORange = NetsAndXRanges[idx].ranges[0];

			//Need to find an empty place to split it
			int j;
			for (j = ORange.first; j <= ORange.second; j++) {
				if (rowT[j] <= 0 || rowB[j] <= 0)
					break;
			}
			pair<int, int> NewRange1 = { ORange.first, j };
			pair<int, int> NewRange2 = { j, ORange.second };
			NetsAndXRanges[idx].ranges[0] = NewRange1;
			NetsAndXRanges[idx].ranges.push_back(NewRange2);

			//remove the last two elements causing the dogleg problem
			V[i].pop_back();
			V[i].pop_back();
			V.push_back({ netIDProb , netIDEnd });
		}
	}

	return;
}


void Routing::BuildV(int i, vector<vector<int>>& V)
{
	//vector<vector<int>> V;
	vector<int>& rowT = TopRow[i].RowNets;
	vector<int>& rowB = BotRow[i].RowNets;

	//find the range of every net
	for (int j = 0; j < m_colCount; j++) {
		int netIDT = rowT[j];
		int netIDB = rowB[j];

		if (netIDT > 0 && netIDB > 0) {
			for (int k = 0; k < V.size(); k++) {
				if (netIDB == V[k][0]) {
					V[k].insert(V[k].begin(), netIDT);
					goto EndOuterForLoop;
				}
				else {
					for (int l = 0; l < V[k].size(); l++) {
						if (V[k][l] == netIDT) {
							V[k].push_back(netIDB);
							goto EndOuterForLoop;
						}
					}
				}
			}
		}

		V.push_back({ netIDT, netIDB });

	EndOuterForLoop:
	}

	return;
}


void Routing::BuildRange(int i, vector<NetAndRanges>& NetsAndXRanges)
{
	vector<int>& rowT = TopRow[i].RowNets;
	vector<int>& rowB = BotRow[i].RowNets;

	//find the range of every net
	for (int j = 0; j < m_colCount; j++) {
		int netID = rowT[j];
		if (netID > 0) {
			auto iter = find_if(NetsAndXRanges.begin(), NetsAndXRanges.end(),
				[netID](NetAndRanges const& item) {return item.net == netID; });

			//if the net doesn't exist in the list, add it
			if (iter == NetsAndXRanges.end()) {
				NetsAndXRanges.push_back(ColumnsCrossed(i, j, netID, true));
			}
		}

		netID = rowB[j];
		if (netID > 0) {
			auto iter = find_if(NetsAndXRanges.begin(), NetsAndXRanges.end(),
				[netID](NetAndRanges const& item) {return item.net == netID; });

			//if the net doesn't exist in the list, add it
			if (iter == NetsAndXRanges.end()) {
				NetsAndXRanges.push_back(ColumnsCrossed(i, j, netID, false));
			}
		}
	}
}


NetAndRanges Routing::ColumnsCrossed(int i, int j, int netID, bool isTop)
{
	//Get the X columns for each net
	vector<int>& rowT = TopRow[i].RowNets;
	vector<int>& rowB = BotRow[i].RowNets;

	//in case the other terminal for the net is actually in the same column
	int TopAdj = 0, BotAdj = 0;
	if (isTop) TopAdj++;
	else BotAdj++;

	vector<int>::iterator iter1 = rowT.begin() + j + TopAdj;
	vector<int>::iterator iter2 = rowB.begin() + j + BotAdj;

	int x2 = -1;

	//look through top and bottom row for the next instance of the id
	iter1 = find(iter1, rowT.end(), netID);
	iter2 = find(iter2, rowB.end(), netID);

	//net is only in top
	if (iter1 != rowT.end()) {
		x2 = find(iter1, rowT.end(), netID) - rowB.begin();
	}
	//net is only in bottom
	else if (iter2 != rowB.end()) {
		x2 = find(iter2, rowB.end(), netID) - rowB.begin();
	}

	return NetAndRanges(netID, { { j, x2 } });
}


void Routing::BuildS(int i, vector<set<pair<int, int>>>& S, vector<NetAndRanges>& NetsAndXVals)
{
	//if the range of the net overlaps a column, add it to the HGraph
	for (int j = 0; j < m_colCount; j++){				//all cols
		for (auto& k : NetsAndXVals) {					//all nets and x ranges
			int iter = 0;
			for (auto& l : k.ranges) {					//all ranges for each net
				if (l.first >= j && l.second <= j) {	//check if net's x ranges overlap this col
					S[j].insert({ k.net,iter });		//push net
				}
				iter++;
			}
		}
	}

	vector<int> DeleteS;
	//check each vector to see if its contained in another vector
	for (int j = 0; j < S.size(); j++){
		for (int k = 0; k < S.size(); k++) {
			for (int l = 0; l < S.size(); l++) {
				if (includes(S[k].begin(), S[k].end(), S[l].begin(), S[l].end())) {
					DeleteS.push_back(k);
				}
			}
		}
	}

	for (int j = DeleteS.size()-1; j >= 0; j--)
		remove(S.begin(), S.end(), DeleteS[j]);

	return;
}


template <typename T>
void swapNum(T& n1, T& n2)
{
	T temp = n1;
	n1 = n2;
	n2 = temp;
}