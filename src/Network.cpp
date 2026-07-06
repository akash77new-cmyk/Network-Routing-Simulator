#include "Network.h"

#include <algorithm>
#include <climits>
#include <fstream>
#include <iomanip>
#include <queue>
#include <set>
#include <sstream>
#include <utility>

namespace {

/**
 * QueueItem
 * ---------
 * Small, purpose-built structure used as the priority_queue element for
 * Dijkstra's algorithm. Using a named struct here (instead of
 * std::pair<int, std::string>) keeps the comparison logic explicit and the
 * code self-documenting.
 */
struct QueueItem {
    int cost;
    std::string routerId;
};

/**
 * Comparator that turns std::priority_queue into a MIN-heap ordered by cost,
 * which is exactly what Dijkstra's algorithm needs (always expand the
 * closest unvisited router next).
 */
struct QueueItemCompare {
    bool operator()(const QueueItem& lhs, const QueueItem& rhs) const {
        return lhs.cost > rhs.cost;
    }
};

} // namespace

// ============================================================================
// Topology management
// ============================================================================

bool Network::addRouter(const std::string& id) {
    if (routers_.find(id) != routers_.end()) {
        return false; // Router already exists
    }
    routers_.emplace(id, Router(id));
    adjacency_.emplace(id, std::vector<Edge>());
    ++topologyChanges_;
    return true;
}

bool Network::removeRouter(const std::string& id) {
    if (routers_.find(id) == routers_.end()) {
        return false;
    }

    // Remove every edge that points at this router from all neighbor lists.
    for (auto& entry : adjacency_) {
        auto& edges = entry.second;
        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                    [&id](const Edge& e) { return e.destination == id; }),
                    edges.end());
    }

    adjacency_.erase(id);
    routers_.erase(id);
    ++topologyChanges_;
    return true;
}

bool Network::addLink(const std::string& a, const std::string& b, int cost) {
    if (!hasRouter(a) || !hasRouter(b) || a == b) {
        return false;
    }
    if (hasLink(a, b)) {
        return false; // Use UPDATE to change an existing link's cost
    }

    adjacency_[a].push_back(Edge(b, cost));
    adjacency_[b].push_back(Edge(a, cost));
    ++topologyChanges_;
    return true;
}

bool Network::removeLink(const std::string& a, const std::string& b) {
    if (!hasLink(a, b)) {
        return false;
    }

    auto eraseEdge = [](std::vector<Edge>& edges, const std::string& dest) {
        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                    [&dest](const Edge& e) { return e.destination == dest; }),
                    edges.end());
    };

    eraseEdge(adjacency_[a], b);
    eraseEdge(adjacency_[b], a);
    ++topologyChanges_;
    return true;
}

bool Network::updateLink(const std::string& a, const std::string& b, int cost) {
    Edge* edgeAB = findEdge(a, b);
    Edge* edgeBA = findEdge(b, a);
    if (!edgeAB || !edgeBA) {
        return false;
    }
    edgeAB->cost = cost;
    edgeBA->cost = cost;
    ++topologyChanges_;
    return true;
}

// ============================================================================
// Link failure simulation
// ============================================================================

bool Network::failLink(const std::string& a, const std::string& b) {
    Edge* edgeAB = findEdge(a, b);
    Edge* edgeBA = findEdge(b, a);
    if (!edgeAB || !edgeBA) {
        return false;
    }
    if (!edgeAB->active) {
        return false; // Already down
    }
    edgeAB->active = false;
    edgeBA->active = false;
    ++topologyChanges_;
    return true;
}

bool Network::restoreLink(const std::string& a, const std::string& b) {
    Edge* edgeAB = findEdge(a, b);
    Edge* edgeBA = findEdge(b, a);
    if (!edgeAB || !edgeBA) {
        return false;
    }
    if (edgeAB->active) {
        return false; // Already up
    }
    edgeAB->active = true;
    edgeBA->active = true;
    ++topologyChanges_;
    return true;
}

// ============================================================================
// Queries
// ============================================================================

bool Network::hasRouter(const std::string& id) const {
    return routers_.find(id) != routers_.end();
}

bool Network::hasLink(const std::string& a, const std::string& b) const {
    return findEdge(a, b) != nullptr;
}

bool Network::isLinkActive(const std::string& a, const std::string& b) const {
    const Edge* edge = findEdge(a, b);
    return edge != nullptr && edge->active;
}

std::size_t Network::getRouterCount() const {
    return routers_.size();
}

std::size_t Network::getLinkCount() const {
    std::size_t total = 0;
    for (const auto& entry : adjacency_) {
        total += entry.second.size();
    }
    return total / 2; // Each physical link is stored twice (once per direction)
}

int Network::getTopologyChangeCount() const {
    return topologyChanges_;
}

std::vector<std::string> Network::getRouterIds() const {
    std::vector<std::string> ids;
    ids.reserve(routers_.size());
    for (const auto& entry : routers_) {
        ids.push_back(entry.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

// ============================================================================
// Display
// ============================================================================

void Network::printTopology(std::ostream& out) const {
    out << "\n=== Network Topology ===\n";
    if (routers_.empty()) {
        out << "(empty network -- use ADD to create routers)\n";
        return;
    }

    auto ids = getRouterIds();
    for (const auto& id : ids) {
        out << "Router " << id << ":\n";
        auto edges = adjacency_.at(id); // Copy so we can sort for stable display
        std::sort(edges.begin(), edges.end(), [](const Edge& lhs, const Edge& rhs) {
            return lhs.destination < rhs.destination;
        });

        if (edges.empty()) {
            out << "    (no links)\n";
            continue;
        }

        for (const auto& edge : edges) {
            out << "    -> " << edge.destination << " (cost: " << edge.cost << ")"
                << (edge.active ? "" : "  [DOWN]") << "\n";
        }
    }
    out << "========================\n";
}

void Network::printRoutingTable(const std::string& source, std::ostream& out) const {
    if (!hasRouter(source)) {
        out << "Error: router '" << source << "' does not exist.\n";
        return;
    }

    auto results = computeAllShortestPaths(source);
    auto ids = getRouterIds();

    out << "\n=== Routing Table for " << source << " ===\n";
    out << std::left << std::setw(14) << "Destination" << std::setw(14) << "Next Hop"
        << std::setw(10) << "Cost" << "\n";
    out << std::string(38, '-') << "\n";

    for (const auto& dest : ids) {
        if (dest == source) {
            continue;
        }
        const PathResult& result = results.at(dest);
        std::string nextHop = "-";
        std::string costStr = "unreachable";

        if (result.reachable && result.path.size() > 1) {
            nextHop = result.path[1];
            costStr = std::to_string(result.totalCost);
        }

        out << std::left << std::setw(14) << dest << std::setw(14) << nextHop
            << std::setw(10) << costStr << "\n";
    }
    out << "==========================================\n";
}

// ============================================================================
// Path computation (Dijkstra's algorithm)
// ============================================================================

std::unordered_map<std::string, PathResult> Network::computeAllShortestPaths(const std::string& source) const {
    std::unordered_map<std::string, PathResult> results;

    if (!hasRouter(source)) {
        return results; // Empty map signals "no such source router"
    }

    std::unordered_map<std::string, int> distance;
    std::unordered_map<std::string, std::string> previous;
    std::unordered_map<std::string, bool> visited;

    for (const auto& entry : routers_) {
        distance[entry.first] = INT_MAX;
        visited[entry.first] = false;
    }
    distance[source] = 0;

    std::priority_queue<QueueItem, std::vector<QueueItem>, QueueItemCompare> pq;
    pq.push({0, source});

    while (!pq.empty()) {
        QueueItem current = pq.top();
        pq.pop();

        if (visited[current.routerId]) {
            continue;
        }
        visited[current.routerId] = true;

        const auto adjIt = adjacency_.find(current.routerId);
        if (adjIt == adjacency_.end()) {
            continue;
        }

        for (const Edge& edge : adjIt->second) {
            if (!edge.active) {
                continue; // Skip failed links entirely
            }
            if (visited[edge.destination]) {
                continue;
            }
            if (distance[current.routerId] == INT_MAX) {
                continue;
            }

            int candidate = distance[current.routerId] + edge.cost;
            if (candidate < distance[edge.destination]) {
                distance[edge.destination] = candidate;
                previous[edge.destination] = current.routerId;
                pq.push({candidate, edge.destination});
            }
        }
    }

    // Build a PathResult for every router in the network.
    for (const auto& entry : routers_) {
        const std::string& destId = entry.first;
        PathResult result;

        if (distance[destId] == INT_MAX) {
            result.reachable = false;
            result.totalCost = -1;
            results[destId] = result;
            continue;
        }

        // Reconstruct the path by walking backwards through `previous`.
        std::vector<std::string> reversePath;
        std::string cursor = destId;
        reversePath.push_back(cursor);
        while (cursor != source) {
            auto prevIt = previous.find(cursor);
            if (prevIt == previous.end()) {
                break; // Should not happen if distance was reachable
            }
            cursor = prevIt->second;
            reversePath.push_back(cursor);
        }
        std::reverse(reversePath.begin(), reversePath.end());

        result.path = std::move(reversePath);
        result.totalCost = distance[destId];
        result.reachable = true;
        results[destId] = result;
    }

    return results;
}

PathResult Network::computeShortestPath(const std::string& source, const std::string& destination) const {
    if (!hasRouter(source) || !hasRouter(destination)) {
        return PathResult(); // reachable == false by default
    }

    auto allResults = computeAllShortestPaths(source);
    auto it = allResults.find(destination);
    if (it == allResults.end()) {
        return PathResult();
    }
    return it->second;
}

// ============================================================================
// Persistence
// ============================================================================

bool Network::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        return false;
    }

    // Loading replaces the current topology entirely.
    routers_.clear();
    adjacency_.clear();
    topologyChanges_ = 0;

    std::string line;
    while (std::getline(in, line)) {
        // Strip comments and skip blank lines.
        auto hashPos = line.find('#');
        if (hashPos != std::string::npos) {
            line = line.substr(0, hashPos);
        }
        std::istringstream iss(line);
        std::string keyword;
        if (!(iss >> keyword)) {
            continue;
        }

        if (keyword == "ROUTER") {
            std::string id;
            if (iss >> id) {
                addRouter(id);
            }
        } else if (keyword == "LINK") {
            std::string a, b;
            int cost;
            if (iss >> a >> b >> cost) {
                addRouter(a);
                addRouter(b);
                addLink(a, b, cost);
            }
        }
    }

    topologyChanges_ = 0; // A freshly loaded topology counts as the new baseline
    return true;
}

bool Network::saveToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        return false;
    }

    out << "# Network Routing Simulator topology file\n";
    out << "# Format: ROUTER <id>  |  LINK <a> <b> <cost>\n\n";

    auto ids = getRouterIds();
    for (const auto& id : ids) {
        out << "ROUTER " << id << "\n";
    }

    out << "\n";

    // Only emit each undirected link once (a < b lexicographically).
    std::set<std::pair<std::string, std::string>> seen;
    for (const auto& id : ids) {
        auto edges = adjacency_.at(id);
        std::sort(edges.begin(), edges.end(), [](const Edge& lhs, const Edge& rhs) {
            return lhs.destination < rhs.destination;
        });
        for (const auto& edge : edges) {
            std::string a = id;
            std::string b = edge.destination;
            if (a > b) {
                std::swap(a, b);
            }
            auto key = std::make_pair(a, b);
            if (seen.count(key)) {
                continue;
            }
            seen.insert(key);
            out << "LINK " << a << " " << b << " " << edge.cost << "\n";
        }
    }

    return true;
}

// ============================================================================
// Internal helpers
// ============================================================================

Edge* Network::findEdge(const std::string& from, const std::string& to) {
    auto it = adjacency_.find(from);
    if (it == adjacency_.end()) {
        return nullptr;
    }
    for (auto& edge : it->second) {
        if (edge.destination == to) {
            return &edge;
        }
    }
    return nullptr;
}

const Edge* Network::findEdge(const std::string& from, const std::string& to) const {
    auto it = adjacency_.find(from);
    if (it == adjacency_.end()) {
        return nullptr;
    }
    for (const auto& edge : it->second) {
        if (edge.destination == to) {
            return &edge;
        }
    }
    return nullptr;
}
