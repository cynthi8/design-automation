#ifndef PLACEMENT_HPP
#define PLACEMENT_HPP

#include "Graph.hpp"
#include <vector>

#define INVALID_ROW -1
#define INVALID_COLUMN -1

using namespace std;

class Location
{
public:
    Location() : row(INVALID_ROW), column(INVALID_COLUMN){};
    Location(int row, int column) : row(row), column(column){};
    int row;
    int column;
    bool isValid() { return row != INVALID_ROW && column != INVALID_COLUMN; };
};

class GridCell
{
public:
    GridCell() : occupied(false), locked(false), cellId(-1){};
    bool occupied;
    bool locked;
    int cellId;
};

class Grid
{
public:
    Grid(int rows, int cols);
    const int m_rows;
    const int m_cols;
    GridCell operator[](Location location) const { return m_grid[location.row][location.column]; }
    GridCell &operator[](Location location) { return m_grid[location.row][location.column]; }
    void UnlockAll();
    void PlaceCell(Location location, int cellId);
    void PlaceAndLockCell(Location location, int cellId);
    Location FindClosestUnlockedLocation(Location location);
    Location FindNextUnoccupiedLocation(Location location);

private:
    vector<vector<GridCell>> m_grid;
};

class Placement
{
public:
    const Graph m_netlist;
    const int m_gridWidth;
    const int m_gridHeight;
    Grid m_grid;
    vector<Location> m_locations;

    Placement(Graph netlist, int gridWidth);
    void UpdateCellLocation(Location newLocation, int cellId);
    void ForceDirected();
    Location CalculateEquilibriumLocation(Cell cell);
    void InvalidateLocation(int cellId);
    void PickUpCell(int cellId);
};

#endif // !PLACEMENT_HPP
