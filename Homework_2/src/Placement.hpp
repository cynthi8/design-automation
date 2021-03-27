#ifndef PLACEMENT_HPP
#define PLACEMENT_HPP

#include "Graph.hpp"
#include <vector>

using namespace std;

struct Location
{
    bool locked;
    int x;
    int y;
};

class Placement
{
public:
    Placement();
    void Place(Graph graph, int maxWidth);
    int m_rowCount;
    vector<vector<int>> m_grid; // Assumes 0 height channels
    vector<Location> m_locations;
};

#endif // !PLACEMENT_HPP
