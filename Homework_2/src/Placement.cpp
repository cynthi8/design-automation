// Placement

#include "Placement.hpp"
#include "Graph.hpp"
#include <algorithm>

// One thing to take into consideration is after placement is done
// you need to count the connections between rows
// ex a wire from row 1 to 3, such that we need a dummy cell
// in which case we need at least the # of dummy cells as connections

// what could be done is to weight connections between those in the same row (x direction)
// higher than in the y direction so we don't use as many dummy cells

void Placement::Place(Graph graph, int maxWidth)
{

	// Initialize location list
	m_locations.reserve(graph.m_cells.size());
	int x = 0;
	int y = 0;
	for (long unsigned int i = 1; i < graph.m_cells.size(); i++)
	{
		Location newLocation = {false, x, y};
		m_locations.push_back(newLocation);
		if (x == maxWidth - 1)
		{
			x = 0;
			y++;
		}
		else
		{
			x++;
		}
	}

	// Sort copy of nodes by connectivity
	vector<Cell> cellList(graph.m_cells);
	sort(cellList.begin(), cellList.end(), [](Cell cellA, Cell cellB) { return cellA.m_connectivity > cellB.m_connectivity; });

	for (auto baseCell : cellList)
	{
		// Ignore completely isolated cells
		if (baseCell.m_connectivity == 0)
		{
			continue;
		}

		// Pick up the base cell
		int posX = baseCell.


		// Calculate equilibrium position.
		int equilibriumX = 0, equilibriumY = 0;
		int connections = 0;
		for (auto net : baseCell.m_nets)
		{
			for (auto terminal : net.m_connections)
			{
				if (terminal.cell == baseCell.m_id)
				{
					continue;
				}
				equilibriumX += m_locations[terminal.cell].x;
				equilibriumY += m_locations[terminal.cell].y;
				connections++;
			}
		}
		equilibriumX /= baseCell.m_connectivity;
		equilibriumY /= baseCell.m_connectivity;

		// Move cell to equilibrium position

	}

	return;
}