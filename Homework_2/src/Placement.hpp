#ifndef PLACEMENT_HPP
#define PLACEMENT_HPP

#include "Graph.hpp"
#include <vector>
#include <unordered_map>

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

struct FineLocation
{
    FineLocation operator+(const FineLocation &a)
    {
        return {x + a.x, y + a.y};
    }
    int x;
    int y;
};

class GridCell
{
public:
    GridCell() : occupied(false), locked(false), m_cellId("null"){};
    bool occupied;
    bool locked;
    void setCellId(string cellId) { m_cellId = cellId; };
    string getCellId() { return m_cellId; };

private:
    string m_cellId;
};

class Grid
{
public:
    Grid(int rows, int cols);
    ~Grid();
    const int m_rows;
    const int m_cols;
    GridCell &operator[](const Location &location) { return m_grid[location.row][location.column]; };
    const GridCell &operator[](const Location &location) const { return m_grid[location.row][location.column]; }
    void UnlockAll();
    void PlaceCell(Location location, string cellId);
    void PlaceAndLockCell(Location location, string cellId);
    Location FindClosestUnlockedLocation(Location location);
    Location FindNextUnoccupiedLocation(Location location);

private:
    vector<vector<GridCell>> m_grid;
};

class Placement
{
public:
    Placement(Graph netlist, int gridWidth);

    const int m_gridWidth;
    const int m_gridHeight;
    Grid m_grid;
    Graph m_netlist;

    unordered_map<string, Location> m_locations;

    void ForceDirectedPlace();
    void ForceDirectedFlip();
    void Export(string fileName);

    void UpdateCellLocation(Location newLocation, string cellId);
    Location CalculateEquilibriumLocation(string cellId);
    void PickUpCell(string cellId);
    void InvalidateLocation(string cellId);

    int CalculateFineCost(string cellId);
    int CalculateFineDistance(Terminal term0, Terminal term1);
    FineLocation CalculateFineLocation(Terminal term);

private:
    vector<pair<string, int>> m_sortedCells;
};

#endif // !PLACEMENT_HPP
