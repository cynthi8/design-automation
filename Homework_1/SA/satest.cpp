#include "SA_algorithm.hpp"

int main() {
    Graph myGraph("../development_netlists/bench_2.net");
    myGraph.SimulatedAnealing(4000, 5, .95, 100);

    return 0;
}