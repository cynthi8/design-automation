// Placement

#include "Placement.hpp"
#include "Graph.hpp"

#include <unordered_map>
#include <algorithm>

#define FEED_THROUGH_ID 0
#define COL_PREFERENCE 10
#define CELL_WIDTH 6
#define CELL_HEIGHT 6

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

Grid::~Grid()
{
	for (int row = 0; row < m_rows; row++)
	{
		for (int col = 0; col < m_cols; col++)
		{
			m_grid[row][col].~GridCell();
		}
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

void Grid::PlaceCell(Location location, Cell *const cellPtr)
{
	(*this)[location].setCell(cellPtr);
	(*this)[location].occupied = true;
}

void Grid::PlaceAndLockCell(Location location, Cell *const cellPtr)
{
	PlaceCell(location, cellPtr);
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

Placement::Placement(Graph netlist, int gridWidth) : m_gridWidth(gridWidth),
													 m_gridHeight(UnsafeRoundUpDivision(netlist.m_cellCount, gridWidth)),
													 m_grid(m_gridHeight, gridWidth),
													 m_netlist(netlist)
{
	// Initialize location list and grid arbitrarily
	m_locations.resize(m_netlist.m_cellCount + 1);

	int row = 0, col = 0;

	for (int id : m_netlist.m_validIds)
	{
		Location newLocation = {row, col};
		Cell *const newCellPtr = &m_netlist.m_cells[id];

		m_grid.PlaceCell(newLocation, newCellPtr);
		UpdateCellLocation(newLocation, id);
		if (col < gridWidth)
		{
			col++;
		}
		else
		{
			col = 0;
			row++;
		}
		m_sortedCells.push_back(newCellPtr);
	}

	// Sort the cell list by connectivity
	sort(m_sortedCells.begin(), m_sortedCells.end(), [](Cell *cellA, Cell *cellB) { return (*cellA).m_connectivity > (*cellB).m_connectivity; });
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

Location Placement::CalculateEquilibriumLocation(const Cell &cell)
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
	for (auto baseCellPtr : m_sortedCells)
	{
		// Skip cells already locked
		int baseCellId = baseCellPtr->m_id;
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
			int baseCellId = baseCellPtr->m_id;

			// Skip lonely cells
			if (baseCellPtr->m_connectivity == 0)
			{
				continue;
			}

			Location newLoc = CalculateEquilibriumLocation((*baseCellPtr));

			// Move cell to equilibrium position
			if (m_grid[newLoc].occupied == false)
			{
				// Easy case where the new location is vacant
				m_grid.PlaceAndLockCell(newLoc, baseCellPtr);
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
					Cell *kickedCellPtr = m_grid[newLoc].getCell();
					PickUpCell(kickedCellPtr->m_id);
					baseCellPtr = kickedCellPtr;
					rippleMove = true;
				}

				m_grid.PlaceAndLockCell(newLoc, baseCellPtr);
				UpdateCellLocation(newLoc, baseCellId);
			}
		} while (rippleMove == true);
	}

	// Place cells with invalid locations
	Location curLoc(0, 0);
	for (auto id : m_netlist.m_validIds)
	{
		if (m_locations[id].isValid() == false)
		{
			Cell *cellPtr = &m_netlist.m_cells[id];
			Location newLoc = m_grid.FindNextUnoccupiedLocation(curLoc);
			m_grid.PlaceAndLockCell(newLoc, cellPtr);
			UpdateCellLocation(newLoc, id);
			curLoc = newLoc;
		}
	}
	return;
}

void Placement::ForceDirectedFlip()
{
	for (auto baseCellPtr : m_sortedCells)
	{
		int currentCost, potentialCost;
		currentCost = CalculateFineCost((*baseCellPtr));

		baseCellPtr->FlipLeftToRight();
		potentialCost = CalculateFineCost((*baseCellPtr));
		if (potentialCost >= currentCost)
		{
			//unflip
			baseCellPtr->FlipLeftToRight();
		}
		else
		{
			currentCost = potentialCost;
		}

		baseCellPtr->FlipTopToBottom();
		potentialCost = CalculateFineCost((*baseCellPtr));
		if (potentialCost >= currentCost)
		{
			//unflip
			baseCellPtr->FlipTopToBottom();
		}
		else
		{
			currentCost = potentialCost;
		}
	}
}

int Placement::CalculateFineCost(const Cell &cell)
{
	int cost = 0;
	for (auto net : cell.m_nets)
	{
		// We assume there are just two terminals (specified in HW2.pdf)
		Terminal term0 = net.m_connections[0];
		Terminal term1 = net.m_connections[1];
		cost += CalculateFineDistance(term0, term1);
	}
	return cost;
}

int Placement::CalculateFineDistance(Terminal term0, Terminal term1)
{
	FineLocation fineLocation0 = CalculateFineLocation(term0);
	FineLocation fineLocation1 = CalculateFineLocation(term1);
	return abs(fineLocation1.x - fineLocation0.x) + abs(fineLocation1.y - fineLocation0.y);
}

FineLocation Placement::CalculateFineLocation(Terminal term)
{
	const unordered_map<TerminalLocation, FineLocation> TerminalOffset{{TopLeft, {1, 5}}, {TopRight, {4, 5}}, {BottomLeft, {1, 0}}, {BottomRight, {4, 0}}};

	Cell cell = m_netlist.m_cells[term.cellId];
	Location location = m_locations[term.cellId];
	const TerminalLocation termLocation = cell.getTerminalLocation(term.terminalId);

	int x = location.column * CELL_WIDTH;
	int y = location.row * CELL_HEIGHT;

	FineLocation baseFineLocation{x, y};
	return baseFineLocation + TerminalOffset.at(termLocation);
}