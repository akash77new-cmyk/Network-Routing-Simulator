#ifndef NETWORK_ROUTING_SIMULATOR_NETWORK_H
#define NETWORK_ROUTING_SIMULATOR_NETWORK_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Edge.h"
#include "PathResult.h"
#include "Router.h"

/**
 * Network
 * -------
 * Owns the router graph and every graph algorithm used by the simulator:
 *   - Topology management (add/remove routers, add/remove/update links)
 *   - Persistence (load/save topology text files)
 *   - Shortest path computation via Dijkstra's algorithm
 *   - Routing table generation
 *   - Link failure / restoration
 *
 * The graph is represented as an adjacency list:
 *      unordered_map<router_id, vector<Edge>>
 * Every physical link is stored as two Edge entries (one per direction),
 * which models an undirected network while still letting each direction be
 * failed/restored independently if ever needed.
 */
class Network {
public:
    Network() = default;

    // ---- Topology management -------------------------------------------------
    bool addRouter(const std::string& id);
    bool removeRouter(const std::string& id);

    bool addLink(const std::string& a, const std::string& b, int cost);
    bool removeLink(const std::string& a, const std::string& b);
    bool updateLink(const std::string& a, const std::string& b, int cost);

    // ---- Link failure simulation ----------------------------------------------
    bool failLink(const std::string& a, const std::string& b);
    bool restoreLink(const std::string& a, const std::string& b);

    // ---- Queries ----------------------------------------------------------------
    bool hasRouter(const std::string& id) const;
    bool hasLink(const std::string& a, const std::string& b) const;
    bool isLinkActive(const std::string& a, const std::string& b) const;

    std::size_t getRouterCount() const;
    std::size_t getLinkCount() const;
    int getTopologyChangeCount() const;
    std::vector<std::string> getRouterIds() const;

    // ---- Display ------------------------------------------------------------------
    void printTopology(std::ostream& out = std::cout) const;
    void printRoutingTable(const std::string& source, std::ostream& out = std::cout) const;

    // ---- Path computation (Dijkstra) --------------------------------------------
    // Returns the shortest path from source to destination as a reusable PathResult.
    PathResult computeShortestPath(const std::string& source, const std::string& destination) const;

    // Returns shortest paths from source to every reachable router in the network.
    // Reused by both TABLE (routing tables) and SEND (packet forwarding).
    std::unordered_map<std::string, PathResult> computeAllShortestPaths(const std::string& source) const;

    // ---- Persistence --------------------------------------------------------------
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;

private:
    std::unordered_map<std::string, Router> routers_;
    std::unordered_map<std::string, std::vector<Edge>> adjacency_;
    int topologyChanges_ = 0; // Incremented on every structural/weight/failure change

    Edge* findEdge(const std::string& from, const std::string& to);
    const Edge* findEdge(const std::string& from, const std::string& to) const;
};

#endif // NETWORK_ROUTING_SIMULATOR_NETWORK_H
