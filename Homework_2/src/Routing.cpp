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
	int maxSize = 0;

	Row rowTop;
	Row rowBottom;
	vector<pair<int, int>> Terminals;
	this->Rows.resize(place.m_grid.m_rows);

	// go through the entire 2D grid that's been placed
	for (i = 0; i < place.m_grid.m_rows; i++)
	{
		for (j = 0; i < place.m_grid[i].size(); j++)
		{
			cell_id = place.m_grid[i][j].cell;

			Cell cell = graph.m_cells[cell_id];

			//find all the terminals on this cell
			//auto Terminals = cell.GetActiveTerminals();
			auto Terminals = cell.GetRelativeTerminals();

			for (auto k : Terminals) //add terminals to row in order
			{
				Terminal term(k.second, cell_id);
				switch (k.first)
				{
				case 1:
				case 2:
					rowTop.m_Top.push_back(term);
					break;
				case 3:
				case 4:
					rowBottom.m_Bot.push_back(term);
					break;
				}
			}
		}

		this->Rows[i] = rowBottom;
		this->Rows[i + 1] = rowTop;

		if (Rows[i].m_Top.size() > maxSize)
			maxSize = Rows[i].m_Top.size();
		if (Rows[i].m_Bot.size() > maxSize)
			maxSize = Rows[i].m_Top.size();
	}

	// Pad the end of the rows with zeros
	Terminal empty(0, 0);
	for (i = 0; i < this->Rows.size(); i++)
	{
		for (j = 0; j < this->Rows[i].m_Top.size() - maxSize; j++)
			this->Rows[i].m_Top.push_back(empty);
		for (j = 0; j < this->Rows[i].m_Bot.size() - maxSize; j++)
			this->Rows[i].m_Bot.push_back(empty);
	}

	// Now I need to go and make the VCG graph
	VGraph vgraph;

	// Then just actually route the thing

	return;
}
