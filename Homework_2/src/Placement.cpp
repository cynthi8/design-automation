// Placement

#include "Placement.hpp"
#include "Graph.hpp"
#include <algorithm>

#define FEED_THROUGH_ID 0
#define COL_PREFERENCE 10

// One thing to take into consideration is after placement is done
// you need to count the connections between rows
// ex a wire from row 1 to 3, such that we need a dummy cell
// in which case we need at least the # of dummy cells as connections

static int UnsafeRoundUpDivision(int dividend, int divisor)
{
	return (dividend + divisor - 1) / divisor;
}

Grid::Grid(int rows, int cols) : m_rows(rows), m_cols(cols)
{
	m_grid.resize(rows);
	for (int row = 0; row < rows; row++)
	{
		m_grid[row].resize(cols);
	}
}

void Grid::UnlockAll()
{
	for (int row = 0; row < m_rows; row++)
	{
		for (int col = 0; col < m_cols; col++)
		{
			m_grid[row][col].locked = false;
		}
	}
}

void Grid::PlaceCell(Location location, int cellId)
{
	(*this)[location].cellId = cellId;
	(*this)[location].occupied = true;
}

void Grid::PlaceAndLockCell(Location location, int cellId)
{
	PlaceCell(location, cellId);
	(*this)[location].locked = true;
}

Location Grid::FindClosestUnlockedLocation(Location location)
{
	for (int rowMag = 0; rowMag < m_rows; rowMag++)
	{
		int colSearchWidth = min((rowMag + 1) * COL_PREFERENCE + 1, m_cols);
		for (int rowSign = 1; rowSign > -2; rowSign -= 2)
		{
			int row = rowSign * rowMag;
			if (row < 0 || row >= m_rows)
			{
				continue;
			}

			for (int colMag = 0; colMag < colSearchWidth; colMag++)
			{
				for (int colSign = 1; colSign > -2; colSign -= 2)
				{
					int col = colSign * colMag;
					if (col < 0 || col >= m_cols)
					{
						continue;
					}
					Location newLocation(row, col);

					if ((*this)[newLocation].locked == false)
					{
						return newLocation;
					}
				}
			}
		}
	}
	throw;
}

Location Grid::FindNextUnoccupiedLocation(Location location)
{
	int row = location.row;
	int column = location.column;
	for (; row < m_rows; row++)
	{
		for (; column < m_cols; column++)
		{
			Location nextLocation(row, column);
			if ((*this)[nextLocation].occupied == false)
			{
				return nextLocation;
			}
		}
		column = 0;
	}
	throw;
}

void Placement::UpdateCellLocation(Location newLocation, int cellId)
{
	m_locations[cellId] = newLocation;
}

void Placement::InvalidateLocation(int cellId)
{
	m_locations[cellId].row = INVALID_ROW;
	m_locations[cellId].column = INVALID_COLUMN;
}

void Placement::PickUpCell(int cellId)
{
	Location location = m_locations[cellId];
	if (location.isValid())
	{
		m_grid[location].occupied = false;
		m_grid[location].locked = false;
	}
	InvalidateLocation(cellId);
}

Placement::Placement(Graph netlist, int gridWidth) : m_netlist(netlist),
													 m_gridWidth(gridWidth),
													 m_gridHeight(UnsafeRoundUpDivision(netlist.m_cellCount, gridWidth)),
													 m_grid(m_gridHeight, gridWidth),
													 m_sortedCells(m_netlist.m_cells)
{
	// Initialize location list and grid arbitrarily
	m_locations.resize(m_netlist.m_cellCount);

	int row = 0, col = 0;
	for (int cellId = 1; cellId < netlist.m_cellCount + 1; cellId++)
	{
		Location newLocation = {row, col};
		m_grid.PlaceCell(newLocation, cellId);
		UpdateCellLocation(newLocation, cellId);
		if (col < gridWidth)
		{
			col++;
		}
		else
		{
			col = 0;
			row++;
		}
	}

	// Sort the cell list by connectivity
	sort(m_sortedCells.begin(), m_sortedCells.end(), [](Cell cellA, Cell cellB) { return cellA.m_connectivity > cellB.m_connectivity; });
}

Location Placement::CalculateEquilibriumLocation(Cell cell)
{
	int eqRow = 0, eqCol = 0;
	for (auto net : cell.m_nets)
	{
		for (auto terminal : net.m_connections)
		{
			if (terminal.cellId == cell.m_id)
			{
				continue;
			}
			eqRow += m_locations[terminal.cellId].row;
			eqCol += m_locations[terminal.cellId].column;
		}
	}
	eqRow /= cell.m_connectivity;
	eqCol /= cell.m_connectivity;

	Location eqLoc(eqRow, eqCol);
	return eqLoc;
}

void Placement::ForceDirectedPlace()
{
	for (auto baseCell : m_sortedCells)
	{
		// Skip cells already locked
		int baseCellId = baseCell.m_id;
		Location curLoc = m_locations[baseCellId];
		if (curLoc.isValid() && m_grid[curLoc].locked)
		{
			continue;
		}

		// Pick up the base cell
		PickUpCell(baseCellId);

		bool rippleMove;
		do
		{
			rippleMove = false;
			int baseCellId = baseCell.m_id;

			// Skip lonely cells
			if (baseCell.m_connectivity == 0)
			{
				continue;
			}

			Location newLoc = CalculateEquilibriumLocation(baseCell);

			// Move cell to equilibrium position
			if (m_grid[newLoc].occupied == false)
			{
				// Easy case where the new location is vacant
				m_grid.PlaceAndLockCell(newLoc, baseCellId);
				UpdateCellLocation(newLoc, baseCellId);
			}
			else
			{
				// Find an unlocked location
				if (m_grid[newLoc].locked == true)
				{
					newLoc = m_grid.FindClosestUnlockedLocation(newLoc);
				}

				// Kick the current occupant if needed
				if (m_grid[newLoc].occupied == true)
				{
					int kickedCellId = m_grid[newLoc].cellId;
					PickUpCell(kickedCellId);
					baseCell = m_netlist.m_cells[kickedCellId];
					rippleMove = true;
				}

				m_grid.PlaceAndLockCell(newLoc, baseCellId);
				UpdateCellLocation(newLoc, baseCellId);
			}
		} while (rippleMove == true);
	}

	// Place cells with invalid locations
	Location curLoc(0, 0);
	for (size_t cellId = 1; cellId < m_locations.size(); cellId++)
	{
		if (m_locations[cellId].isValid() == false)
		{
			Location newLoc = m_grid.FindNextUnoccupiedLocation(curLoc);
			m_grid.PlaceAndLockCell(newLoc, cellId);
			UpdateCellLocation(newLoc, cellId);
			curLoc = newLoc;
		}
	}

	return;
}

void Placement::ForceDirectedFlip()
{
	for (auto baseCell : m_sortedCells)
	{
		for (auto net : baseCell.m_nets)
		{
			for (auto terminal : net.m_connections)
			{
				if(terminal.cellId == baseCell.m_id){
					continue;
				}
			}
		}
	}

	// If the cell is being pulled

	// Flip it in direction of pull
}