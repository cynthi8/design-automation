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
    Location operator+(const Location &a)
    {
        return {row + a.row, column + a.column};
    }
    int row;
    int column;
    bool isValid() { return row != INVALID_ROW && column != INVALID_COLUMN; };
};

struct Coordinates
{
    Coordinates operator+(const Coordinates &a)
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
    GridCell(string cellId) : occupied(false), locked(false), m_cellId(cellId){};
    bool occupied;
    bool locked;
    void setCellId(string cellId) { m_cellId = cellId; };
    string getCellId() { return m_cellId; };

    string m_cellId;
};

class Grid
{
public:
    Grid(int rows, int cols);

    GridCell &operator[](const Location &location) { return m_grid[location.row][location.column]; };
    const GridCell &operator[](const Location &location) const { return m_grid[location.row][location.column]; }
    void UnlockAll();
    void PlaceCell(Location location, string cellId);
    void PlaceAndLockCell(Location location, string cellId);
    Location FindClosestUnlockedLocation(Location location);
    Location FindNextUnoccupiedLocation(Location location);

    vector<vector<GridCell>> m_grid;

private:
    const int m_rows;
    const int m_cols;
};

class Placement
{
public:
    Placement(Graph netlist, int gridWidth);

    const int m_gridWidth;
    const int m_gridHeight;
    Grid m_grid;
    Graph m_netlist;
    int m_feedthroughCount;

    unordered_map<string, Location> m_locations;

    void ForceDirectedPlace(int iterations);
    void SimulatedAnealingPlace(float initialTemperature, float freezingTemperature, float heatRetention, int movesPerStep);
    void GreedyFlipping(int iterations);
    void InsertFeedthroughs();
    void Print();

    void PlaceCell(Location newLocation, string cellId);
    Location PickUpCell(string cellId);

    void UpdateCellLocation(Location newLocation, string cellId);
    void InvalidateLocation(string cellId);
    void InsertCell(Location location, string cellId);

    int CalculatePlacementCost();
    Location CalculateEquilibriumLocation(string cellId);
    int EstimateCellCost(string cellId);
    int CalculateCellCost(string cellId);
    int CalculateTerminalDistance(Terminal term0, Terminal term1);
    Coordinates CalculateCoordinates(Terminal term);
    int CalculateChannelRow(Terminal term);

private:
    int CalculateDeltaCost(string cellId0, string cellId1);
    void AcceptSwap(string cellId0, string cellId1, int deltaCost);
    void Swap(string cellId0, string cellId1);

    int m_cost;
    vector<pair<string, int>> m_sortedCells;
};

#endif // !PLACEMENT_HPP
