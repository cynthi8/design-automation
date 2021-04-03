// Routing

#include "Placement.hpp"
#include "Graph.hpp"
#include "Routing.hpp"

void Routing::Route(Graph graph, Placement place)
{
	//This function needs to populate the rows such that we can do channel routing
	int i;
	int j;
	int cell_id;
	int maxCols = 0;

	Row RowTopTemp;
	Row RowBottomTemp;
	vector<pair<int, int>> Terminals;
	SetRowSize(place.m_grid.m_rows);

	// go through the entire 2D grid that's been placed
	for (i = 0; i < place.m_gridHeight; i++) {
		for (j = 0; i < place.m_gridWidth; j++) {

			//Get the cell id
			cell_id = place.m_grid[Location::Location(i, j)].cellId;
			Cell cell = graph.m_cells[cell_id];

			//find all the terminals on this cell
			auto Terminals = cell.getTerminalLocations();

			//add terminals to row in order
			for (auto k : Terminals) {
				Terminal term(k.second, cell_id);
				int NetID = graph.GetNetID(term);
				if (k.first == TopLeft || k.first == TopRight) {
					RowTopTemp.AddTerm(term, NetID);
				}
				else if (k.first == BottomLeft || k.first == BottomRight) {
					RowBottomTemp.AddTerm(term, NetID);
				}
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
	
	// Now I need to go and make the VCG and HCG graph
	// This doesn't actually make a graph but I think maybe I should've
	// This is too hard, never again using objects
	for (i = 0; i < m_rowCount; i++) {
		int order = 0;
		vector<int> NetsUsed;

		for (j = 0; j < m_colCount; j++) {
			Terminal TopTerm = TopRow[i].RowCells[j].Term;
			Terminal BotTerm = BotRow[i].RowCells[j].Term;

			int NetIDTop = TopRow[i].RowCells[j].NetID;
			int NetIDBot = BotRow[i].RowCells[j].NetID;

			int TOrder = -1;
			int BOrder = -1;

			//Need to order the terminals
			//and see if any terminals are on top of another
			bool TopNetIsUsed = NetsUsed.end() != find(NetsUsed.begin(), NetsUsed.end(), NetIDTop);
			bool BotNetIsUsed = NetsUsed.end() != find(NetsUsed.begin(), NetsUsed.end(), NetIDBot);

			//increase the order
			if ((NetIDTop != -1 || NetIDTop != -1) &&
				(!TopNetIsUsed || !BotNetIsUsed))
				order++;

			//add this net to the bucket of used nets
			if (!TopNetIsUsed)
				NetsUsed.push_back(NetIDTop);
			if (!BotNetIsUsed)
				NetsUsed.push_back(NetIDBot);

			//Then the top cell is above the bottom cell, add it to the list
			if (NetIDTop != -1 && NetIDBot != -1) {
				TopRow[i].RowCells[j].AboveCell = BotTerm;
				TopRow[i].RowCells[j].Above = true;
			}
			if (NetIDTop != -1) {
				if(!TopNetIsUsed)
					TOrder = order;
				else
					TOrder = TopRow[i].GetUsedNetID(NetIDTop, j);
			}
			if (NetIDBot != -1) {
				if (!BotNetIsUsed)
					BOrder = order;
				else
					BOrder = BotRow[i].GetUsedNetID(NetIDTop, j);
			}

			//each terminal now has some order number for the H Graph
			TopRow[i].Order[j] = TOrder;
			BotRow[i].Order[j] = BOrder;
		} 
	}

	// TODO: Route wires
	// Go through all the rows
	// 
	// I need to find the smallest order value where it's 
	// corresponding terminal isn't above another terminal being used
	for (i = 0; i < m_rowCount; i++) {
		int order = 1;
		int firstorder = 0;
		bool UsingTop = false;

		//find the first occurance of order
		auto Topl = find(TopRow[i].Order.begin(), TopRow[i].Order.end(), order);
		auto Botl = find(BotRow[i].Order.begin(), BotRow[i].Order.end(), order);

		if (Topl != TopRow[i].Order.end()) {
			firstorder = Topl - TopRow[i].Order.begin();
			UsingTop = true;
		}

		if (Botl != TopRow[i].Order.end()) {
			if (Botl - BotRow[i].Order.begin() < firstorder) {
				firstorder = Botl - BotRow[i].Order.begin();
				UsingTop = false;
			}
		}

		//go thru cols from firstorder to col count
		for (j = firstorder; j < m_colCount; j++) {

			for (int k = j; k < m_colCount; k++) {
				if (1)
					continue;

			}
		}
	}

	return;
}
