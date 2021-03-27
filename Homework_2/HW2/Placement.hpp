#ifndef PLACEMENT_HPP
#define PLACEMENT_HPP

#include "Graph.hpp"
#include <vector>

using namespace std;

class Placement
{
public:
    Placement();
    void Place(Graph graph, int maxWidth);
    int m_rowCount;
    vector<vector<int>> m_grid;
};

#endif // !PLACEMENT_HPP
