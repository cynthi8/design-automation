// Routing

#include "Placement.hpp"
#include "Graph.hpp"
#include "Routing.hpp"

#include <tuple>
#include <vector>
#include <cstdlib>
#include <algorithm>

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
			Cell cell = graph.m_cells[cell_id];

			//find all the terminals on this cell
			auto Terminals = cell.getTerminalLocations();

			//add terminals and nets to row in order
			for (auto terms : Terminals) {
				Terminal term(cell_id, terms.first);
				int NetID = graph.GetNetID(term);

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

	for (i = 0; i < m_rowCount; i++) {
		vector<tuple<int, int, int>> NetsAndXVals;
		vector<vector<int>> S = BuildS(i, NetsAndXVals);

		//TODO build V and then route
		vector<vector<int>> V;
	}

	return;
}


vector<vector<int>> Routing::BuildS(int i, vector<tuple<int, int, int>>& NetsAndXVals)
{
	vector<vector<int>> S(m_colCount);

	vector<int>& rowT = TopRow[i].RowNets;
	vector<int>& rowB = BotRow[i].RowNets;

	//find the range of every net
	for (int j = 0; j < m_colCount; j++) {
		int netID = rowT[j];
		if (netID > 0) {
			auto iter = find_if(NetsAndXVals.begin(), NetsAndXVals.end(),
				[=](auto item) {return get<0>(item) == netID; });

			//if the net doesn't exist in the list, add it
			if (iter == NetsAndXVals.end()) {
				NetsAndXVals.push_back(ColumnsCrossed(i, j, netID, true));
			}
		}

		netID = rowB[j];
		if (netID > 0) {
			auto iter = find_if(NetsAndXVals.begin(), NetsAndXVals.end(),
				[=](auto item) {return get<0>(item) == netID; });

			//if the net doesn't exist in the list, add it
			if (iter == NetsAndXVals.end()) {
				NetsAndXVals.push_back(ColumnsCrossed(i, j, netID, false));
			}
		}
	}

	//if the range of the net overlaps a column, add it to the HGraph
	for (int j = 0; j < m_colCount; j++) {
		for (int k = 0; k < NetsAndXVals.size(); k++) {
			if (get<1>(NetsAndXVals[k]) >= j && get<2>(NetsAndXVals[k]) <= j)
				S[j].push_back(get<0>(NetsAndXVals[k]));
		}
	}

	//Might want to remove redundant columns as from textbook pg 331
	return S;
}

tuple<int, int, int> Routing::ColumnsCrossed(int i, int j, int netID, bool isTop)
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


	return { netID, j, x2};
}

template <typename T>
void swapNum(T& n1, T& n2)
{
	T temp = n1;
	n1 = n2;
	n2 = temp;
}