#ifndef NETWORK_ROUTING_SIMULATOR_PATHRESULT_H
#define NETWORK_ROUTING_SIMULATOR_PATHRESULT_H

#include <string>
#include <vector>

/**
 * PathResult
 * ----------
 * Reusable result object produced by the shortest-path algorithm.
 * Rather than having Dijkstra's algorithm print directly to stdout, it
 * returns a PathResult so that the same computation can be reused by:
 *   - the PATH command (printing a single route),
 *   - the TABLE command (building a full routing table),
 *   - the SEND command (simulating packet forwarding).
 */
struct PathResult {
    std::vector<std::string> path; // Ordered list of router ids, source -> destination
    int totalCost = 0;              // Sum of edge costs along the path
    bool reachable = false;         // False if no path exists

    PathResult() = default;
};

#endif // NETWORK_ROUTING_SIMULATOR_PATHRESULT_H
