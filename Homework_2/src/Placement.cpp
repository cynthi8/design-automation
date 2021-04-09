// Placement

#include "Placement.hpp"
#include "Graph.hpp"

#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

#define FEED_THROUGH_TOP_TERMINAL 1
#define FEED_THROUGH_BOTTOM_TERMINAL 3
#define VERTICAL_COST_WEIGHT 20
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

void Grid::PlaceCell(Location location, string cellId)
{
	(*this)[location].setCellId(cellId);
	(*this)[location].occupied = true;
}

void Grid::PlaceAndLockCell(Location location, string cellId)
{
	PlaceCell(location, cellId);
	(*this)[location].locked = true;
}

Location Grid::FindClosestUnlockedLocation(Location location)
{
	for (int rowMag = 0; rowMag < m_rows; rowMag++)
	{
		int colSearchWidth = min((rowMag + 1) * VERTICAL_COST_WEIGHT + 1, m_cols);
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

Placement::Placement(Graph netlist, int gridWidth) : m_grid(UnsafeRoundUpDivision(netlist.m_cellCount, gridWidth), gridWidth),
													 m_netlist(netlist),
													 m_feedthroughCount(0),
													 m_gridWidth(gridWidth)
{
	// Initialize location list and grid arbitrarily
	int i = 0;
	for (auto &mapEntry : m_netlist.m_cells)
	{
		string cellId = mapEntry.first;
		int cellConnectivity = mapEntry.second.m_connectivity;
		int row = i / m_gridWidth;
		int col = i % m_gridWidth;

		Location newLocation = {row, col};

		PlaceCell(newLocation, cellId);
		m_sortedCells.push_back({cellId, cellConnectivity});
		i++;
	}

	// Sort the cell list by connectivity
	sort(m_sortedCells.begin(), m_sortedCells.end(), [](pair<string, int> entryA, pair<string, int> entryB) {
		return entryA.second > entryB.second;
	});
}

void Placement::UpdateCellLocation(Location newLocation, string cellId)
{
	m_locations[cellId] = newLocation;
}

void Placement::InvalidateLocation(string cellId)
{
	m_locations[cellId].row = INVALID_ROW;
	m_locations[cellId].column = INVALID_COLUMN;
}

Location Placement::PickUpCell(string cellId)
{
	Location location = m_locations[cellId];
	if (location.isValid())
	{
		m_grid[location].occupied = false;
		m_grid[location].locked = false;
	}
	InvalidateLocation(cellId);
	return location;
}

void Placement::InsertCell(Location location, string cellId)
{
	// Create the new GridCell
	GridCell newGridCell(cellId);

	// If needed, add some buffer to the row
	vector<GridCell> &gridRow = m_grid.m_grid[location.row];
	while (gridRow.size() < (size_t)location.column)
	{
		gridRow.push_back(GridCell());
	}

	// Insert the newly created GridCell
	auto it = gridRow.insert(gridRow.begin() + location.column, newGridCell);

	// Update the location of the new cell
	UpdateCellLocation(location, cellId);

	// Update the locations of all the displaced cells (they are shifted a column to the right)
	for (; it != gridRow.end(); it++)
	{
		string displacedCellId = (*it).getCellId();
		Location newLocation = m_locations[displacedCellId] + Location(0, 1);
		UpdateCellLocation(newLocation, displacedCellId);
	}
}

void Placement::PlaceCell(Location newLocation, string cellId)
{
	m_grid.PlaceAndLockCell(newLocation, cellId);
	UpdateCellLocation(newLocation, cellId);
}

int Placement::CalculatePlacementCost()
{
	int cost = 0;
	for (auto &cellEntry : m_netlist.m_cells)
	{
		cost += CalculateCellCost(cellEntry.first);
	}
	return cost / 2; // Each net was counted twice
}

Location Placement::CalculateEquilibriumLocation(string cellId)
{
	Cell &cell = m_netlist.m_cells[cellId];
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

void Placement::ForceDirectedPlace(int iterations)
{
	for (int i = 0; i < iterations; i++)
	{
		m_grid.UnlockAll();
		for (auto cellConnectivityPair : m_sortedCells)
		{
			// Skip cells already locked
			string baseCellId = cellConnectivityPair.first;
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

				// Skip lonely cells
				if (m_netlist.m_cells[baseCellId].m_connectivity == 0)
				{
					continue;
				}

				Location newLoc = CalculateEquilibriumLocation(baseCellId);

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
					string kickedCellId;
					if (m_grid[newLoc].occupied == true)
					{
						kickedCellId = m_grid[newLoc].getCellId();
						PickUpCell(kickedCellId);
						rippleMove = true;
					}

					// Place our base cell
					m_grid.PlaceAndLockCell(newLoc, baseCellId);
					UpdateCellLocation(newLoc, baseCellId);

					// Set up baseCellId for next loop if we are rippling
					if (rippleMove)
					{
						baseCellId = kickedCellId;
					}
				}
			} while (rippleMove == true);
		}

		// Place cells with invalid locations (scan for open spots)
		Location curLoc(0, 0);
		for (auto &mapEntry : m_netlist.m_cells)
		{
			string cellId = mapEntry.first;
			if (m_locations[cellId].isValid() == false)
			{
				Location newLoc = m_grid.FindNextUnoccupiedLocation(curLoc);
				m_grid.PlaceAndLockCell(newLoc, cellId);
				UpdateCellLocation(newLoc, cellId);
				curLoc = newLoc;
			}
		}
	}
}

void Placement::SimulatedAnealingPlace(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep)
{
	std::mt19937 m_generator; //Mersenne_Twister and not seeded for reproducibility
	std::uniform_int_distribution<int> m_cellDistribution(1, m_netlist.m_cellCount);
	std::uniform_real_distribution<float> m_unitDistribution(0, 1);

	float temperature = initialTemperature;
	while (temperature > freezingTemperature)
	{
		float boltzmanLimit = exp(-1 / temperature);
		//int numAccepted = 0;
		for (int i = 0; i < movesPerStep; i++)
		{
			int cell0 = m_cellDistribution(m_generator);
			int cell1 = m_cellDistribution(m_generator);

			if (cell0 == cell1)
			{
				continue;
			}

			string cell0Id = to_string(cell0);
			string cell1Id = to_string(cell1);

			int deltaCost = CalculateDeltaCost(cell0Id, cell1Id);
			if (deltaCost < 0)
			{
				m_cost -= deltaCost;
				//numAccepted++;
			}
			else
			{
				if (m_unitDistribution(m_generator) < pow(boltzmanLimit, deltaCost + 1)) // Add 1 to deltaCost so 0 is not auto-accepted
				{
					// Probablistically accept increases in cost
					m_cost -= deltaCost;
					//numAccepted++;
				}
				else
				{
					// Swap back
					Swap(cell0Id, cell1Id);
				}
			}
		}
		//float acceptProportion = (float)numAccepted / (float)movesPerStep;
		temperature *= heatRetention;
	}
}

int Placement::CalculateDeltaCost(string cellId0, string cellId1)
{
	int costBefore = EstimateCellCost(cellId0) + EstimateCellCost(cellId1);
	Swap(cellId0, cellId1);
	int costAfter = EstimateCellCost(cellId0) + EstimateCellCost(cellId1);
	return costAfter - costBefore;
}
void Placement::Swap(string cellId0, string cellId1)
{
	Location location0 = PickUpCell(cellId0);
	Location location1 = PickUpCell(cellId1);

	PlaceCell(location1, cellId0);
	PlaceCell(location0, cellId1);
}

void Placement::GreedyFlipping(int iterations)
{
	for (int i = 0; i < iterations; i++)
	{
		for (auto cellConnectivityPair : m_sortedCells)
		{
			int currentCost, potentialCost;
			string baseCellId = cellConnectivityPair.first;
			Cell &baseCell = m_netlist.m_cells[baseCellId];

			currentCost = CalculateCellCost(baseCellId);

			baseCell.FlipLeftToRight();
			potentialCost = CalculateCellCost(baseCellId);
			if (potentialCost >= currentCost)
			{
				//unflip
				baseCell.FlipLeftToRight();
			}
			else
			{
				currentCost = potentialCost;
			}

			baseCell.FlipTopToBottom();
			potentialCost = CalculateCellCost(baseCellId);
			if (potentialCost >= currentCost)
			{
				//unflip
				baseCell.FlipTopToBottom();
			}
			else
			{
				currentCost = potentialCost;
			}
		}
	}
}

void Placement::InsertFeedthroughs()
{
	int feedthroughNum = 1;
	for (auto cellEntry : m_netlist.m_cells)
	{
		for (auto net : cellEntry.second.m_nets)
		{
			Terminal term0 = net.m_connections[0];
			Terminal term1 = net.m_connections[1];

			int channelRow0 = CalculateChannelRow(term0);
			int channelRow1 = CalculateChannelRow(term1);

			// Early termination if no feedthroughs are needed
			if (channelRow0 == channelRow1)
			{
				continue;
			}

			// term0 is set to the lower of the two
			if (channelRow0 > channelRow1)
			{
				swap(channelRow0, channelRow1);
				swap(term0, term1);
			}

			string cellId0 = term0.cellId;
			string cellId1 = term1.cellId;
			Cell &cell0 = m_netlist.m_cells[cellId0];
			Cell &cell1 = m_netlist.m_cells[cellId1];

			// Remove the net currently connecting the two cells
			cell0.removeNet(net);
			cell1.removeNet(net);

			// Create Feedthroughs all the way up (like a ladder)
			string lowerCellId = cell0.m_id;
			Terminal lowerTerminal = term0;
			int cellColumn = m_locations[cellId0].column;
			for (int cellRow = channelRow0; cellRow < channelRow1; cellRow++)
			{
				// Create the feedthrough cell and get a reference to it in the netlist
				string feedthroughId = "F" + to_string(feedthroughNum);
				feedthroughNum++;
				m_feedthroughCount++;
				m_netlist.addCell(Cell(feedthroughId));
				InsertCell({cellRow, cellColumn}, feedthroughId);

				// Build the top rung
				Terminal upperTerminal = Terminal(feedthroughId, FEED_THROUGH_BOTTOM_TERMINAL);

				// Link the two rungs
				Net feedthroughNet = Net(net.m_id, lowerTerminal, upperTerminal);
				m_netlist.m_cells[lowerCellId].addNet(feedthroughNet);
				m_netlist.m_cells[feedthroughId].addNet(feedthroughNet);

				// Set the lower rung for next loop
				lowerCellId = feedthroughId;
				lowerTerminal = Terminal(feedthroughId, FEED_THROUGH_TOP_TERMINAL);
			}

			// Final rung links to already existing term1
			Net feedthroughNet = Net(net.m_id, lowerTerminal, term1);
			m_netlist.m_cells[lowerCellId].addNet(feedthroughNet);
			cell1.addNet(feedthroughNet);
		}
	}
}

void Placement::Print()
{
	// Reverse iterator because the bottom row is row 0
	for (auto rowIt = m_grid.m_grid.rbegin(); rowIt != m_grid.m_grid.rend(); rowIt++)
	{
		for (auto gridCell : (*rowIt))
		{
			string cellId = gridCell.getCellId();
			Flips cellOrientation = m_netlist.m_cells[cellId].m_orientation;
			cout << cellId << " " << cellOrientation << " ";
		}
		cout << endl;
	}
}
int Placement::EstimateCellCost(string cellId)
{
	// Estimates do not use coordinates, but vertical distance is weighted higher
	int cost = 0;
	for (auto &net : m_netlist.m_cells[cellId].m_nets)
	{
		Location location0 = m_locations[net.m_connections[0].cellId];
		Location location1 = m_locations[net.m_connections[1].cellId];

		cost += abs(location1.row - location0.row) * VERTICAL_COST_WEIGHT + abs(location1.column - location0.column);
	}
	return cost;
}

int Placement::CalculateCellCost(string cellId)
{
	const Cell &cell = m_netlist.m_cells[cellId];
	int cost = 0;
	for (auto &net : cell.m_nets)
	{
		// We assume there are just two terminals (specified in HW2.pdf)
		Terminal term0 = net.m_connections[0];
		Terminal term1 = net.m_connections[1];
		cost += CalculateTerminalDistance(term0, term1);
	}
	return cost;
}

int Placement::CalculateTerminalDistance(Terminal term0, Terminal term1)
{
	Coordinates Coordinates0 = CalculateCoordinates(term0);
	Coordinates Coordinates1 = CalculateCoordinates(term1);
	return abs(Coordinates1.x - Coordinates0.x) + abs(Coordinates1.y - Coordinates0.y);
}

Coordinates Placement::CalculateCoordinates(Terminal term)
{
	const unordered_map<TerminalLocation, Coordinates> TerminalOffset{{TopLeft, {1, 5}}, {TopRight, {4, 5}}, {BottomLeft, {1, 0}}, {BottomRight, {4, 0}}};

	Cell &cell = m_netlist.m_cells[term.cellId];
	Location &location = m_locations[term.cellId];
	const TerminalLocation termLocation = cell.getTerminalLocation(term.terminalId);

	int x = location.column * CELL_WIDTH;
	int y = location.row * CELL_HEIGHT;

	Coordinates baseCoordinates{x, y};
	return baseCoordinates + TerminalOffset.at(termLocation);
}

int Placement::CalculateChannelRow(Terminal term)
{
	string cellId = term.cellId;
	int terminalId = term.terminalId;
	const Cell &cell = m_netlist.m_cells[cellId];

	// Get Cell Row
	int cellRow = m_locations[cellId].row;

	int channelRow = cellRow;

	// Top half belongs to the channel above
	TerminalLocation terminalLocation = cell.getTerminalLocation(terminalId);
	if (terminalLocation == TopLeft || terminalLocation == TopRight)
	{
		channelRow++;
	}

	return channelRow;
}
