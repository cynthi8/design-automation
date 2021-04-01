#ifndef PLACEMENT_HPP
#define PLACEMENT_HPP

#include "Graph.hpp"
#include <vector>

using namespace std;

struct Location
{
    int x;
    int y;
};

struct GridCell
{
    bool placed;
    bool locked;
    int cell;
};

class Placement
{
public:
    Placement();
    void Place(Graph graph, int maxWidth);
    int m_rowCount;
    vector<vector<GridCell>> m_grid; // Assumes 0 height channels
    vector<Location> m_locations;
};

#endif // !PLACEMENT_HPP
