#ifndef NETWORK_ROUTING_SIMULATOR_EDGE_H
#define NETWORK_ROUTING_SIMULATOR_EDGE_H

#include <string>

/**
 * Edge
 * ----
 * Represents a directed connection from one router to another inside the
 * adjacency list. Every physical link in the simulator is stored as a pair
 * of Edge objects (one in each direction) so that the underlying graph
 * behaves like an undirected network while still allowing per-direction
 * bookkeeping (useful for link failure simulation).
 */
struct Edge {
    std::string destination;   // Id of the router this edge points to
    int cost = 0;               // Latency / weight of the link
    bool active = true;         // False when the link has been "FAILed"

    Edge() = default;
    Edge(std::string destination, int cost, bool active = true)
        : destination(std::move(destination)), cost(cost), active(active) {}
};

#endif // NETWORK_ROUTING_SIMULATOR_EDGE_H
