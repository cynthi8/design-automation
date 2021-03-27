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
Placement::Placement(Graph graph) {


	// Sort copy of nodes by connectvity
	L = graph.m_cells.copy();
	L.sort([] )

	return;
}