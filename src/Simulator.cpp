#include "Simulator.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "PathResult.h"

Simulator::Simulator() = default;

// ============================================================================
// Main loop
// ============================================================================

void Simulator::run() {
    std::cout << "=====================================================\n";
    std::cout << " Dynamic Network Routing Simulator\n";
    std::cout << " Type HELP to see the list of available commands.\n";
    std::cout << "=====================================================\n";

    std::string line;
    while (true) {
        std::cout << "\nrouter-sim> ";
        if (!std::getline(std::cin, line)) {
            std::cout << "\nEOF received, exiting.\n";
            break;
        }

        auto tokens = tokenize(line);
        if (tokens.empty()) {
            continue;
        }

        // Normalize the command keyword to uppercase for case-insensitive commands.
        std::string& command = tokens[0];
        std::transform(command.begin(), command.end(), command.begin(), ::toupper);

        if (command == "EXIT" || command == "QUIT") {
            std::cout << "Exiting simulator. Goodbye!\n";
            break;
        }

        handleCommand(tokens);
    }
}

// ============================================================================
// Command dispatch
// ============================================================================

void Simulator::handleCommand(const std::vector<std::string>& tokens) {
    const std::string& command = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    if (command == "ADD") {
        cmdAdd(args);
    } else if (command == "REMOVE") {
        cmdRemove(args);
    } else if (command == "LINK") {
        cmdLink(args);
    } else if (command == "UNLINK") {
        cmdUnlink(args);
    } else if (command == "UPDATE") {
        cmdUpdateLink(args);
    } else if (command == "PRINT") {
        cmdPrint();
    } else if (command == "PATH") {
        cmdPath(args);
    } else if (command == "TABLE") {
        cmdTable(args);
    } else if (command == "SEND") {
        cmdSend(args);
    } else if (command == "FAIL") {
        cmdFail(args);
    } else if (command == "RESTORE") {
        cmdRestore(args);
    } else if (command == "LOAD") {
        cmdLoad(args);
    } else if (command == "SAVE") {
        cmdSave(args);
    } else if (command == "STATS") {
        cmdStats();
    } else if (command == "HELP") {
        cmdHelp();
    } else {
        std::cout << "Unknown command: '" << command << "'. Type HELP for the command list.\n";
    }
}

// ============================================================================
// Individual command handlers
// ============================================================================

void Simulator::cmdAdd(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Usage: ADD <routerId>\n";
        return;
    }
    if (network_.addRouter(args[0])) {
        std::cout << "Router '" << args[0] << "' added.\n";
    } else {
        std::cout << "Error: router '" << args[0] << "' already exists.\n";
    }
}

void Simulator::cmdRemove(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Usage: REMOVE <routerId>\n";
        return;
    }
    if (network_.removeRouter(args[0])) {
        std::cout << "Router '" << args[0] << "' removed.\n";
    } else {
        std::cout << "Error: router '" << args[0] << "' does not exist.\n";
    }
}

void Simulator::cmdLink(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        std::cout << "Usage: LINK <routerA> <routerB> <cost>\n";
        return;
    }
    int cost;
    try {
        cost = std::stoi(args[2]);
    } catch (const std::exception&) {
        std::cout << "Error: cost must be a valid integer.\n";
        return;
    }
    if (cost <= 0) {
        std::cout << "Error: link cost must be a positive integer.\n";
        return;
    }

    if (!network_.hasRouter(args[0]) || !network_.hasRouter(args[1])) {
        std::cout << "Error: both routers must exist before linking. Use ADD first.\n";
        return;
    }
    if (args[0] == args[1]) {
        std::cout << "Error: cannot link a router to itself.\n";
        return;
    }

    if (network_.addLink(args[0], args[1], cost)) {
        std::cout << "Link created: " << args[0] << " <-> " << args[1] << " (cost " << cost << ")\n";
    } else {
        std::cout << "Error: a link between '" << args[0] << "' and '" << args[1]
                   << "' already exists. Use UPDATE to change its cost.\n";
    }
}

void Simulator::cmdUnlink(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "Usage: UNLINK <routerA> <routerB>\n";
        return;
    }
    if (network_.removeLink(args[0], args[1])) {
        std::cout << "Link removed: " << args[0] << " <-> " << args[1] << "\n";
    } else {
        std::cout << "Error: no link exists between '" << args[0] << "' and '" << args[1] << "'.\n";
    }
}

void Simulator::cmdUpdateLink(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        std::cout << "Usage: UPDATE <routerA> <routerB> <newCost>\n";
        return;
    }
    int cost;
    try {
        cost = std::stoi(args[2]);
    } catch (const std::exception&) {
        std::cout << "Error: cost must be a valid integer.\n";
        return;
    }
    if (cost <= 0) {
        std::cout << "Error: link cost must be a positive integer.\n";
        return;
    }

    if (network_.updateLink(args[0], args[1], cost)) {
        std::cout << "Link updated: " << args[0] << " <-> " << args[1] << " (new cost " << cost << ")\n";
    } else {
        std::cout << "Error: no link exists between '" << args[0] << "' and '" << args[1] << "'.\n";
    }
}

void Simulator::cmdPrint() {
    network_.printTopology(std::cout);
}

void Simulator::cmdPath(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "Usage: PATH <source> <destination>\n";
        return;
    }
    const std::string& source = args[0];
    const std::string& destination = args[1];

    if (!network_.hasRouter(source) || !network_.hasRouter(destination)) {
        std::cout << "Error: both routers must exist.\n";
        return;
    }

    PathResult result = network_.computeShortestPath(source, destination);
    if (!result.reachable) {
        std::cout << "No path exists from " << source << " to " << destination << ".\n";
        return;
    }

    std::cout << "Shortest path from " << source << " to " << destination << ": ";
    for (std::size_t i = 0; i < result.path.size(); ++i) {
        std::cout << result.path[i];
        if (i + 1 < result.path.size()) {
            std::cout << " -> ";
        }
    }
    std::cout << " (total cost: " << result.totalCost << ")\n";
}

void Simulator::cmdTable(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Usage: TABLE <routerId>\n";
        return;
    }
    network_.printRoutingTable(args[0], std::cout);
}

void Simulator::cmdSend(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "Usage: SEND <source> <destination>\n";
        return;
    }
    if (!network_.hasRouter(args[0]) || !network_.hasRouter(args[1])) {
        std::cout << "Error: both routers must exist.\n";
        return;
    }

    Packet packet = sendPacket(args[0], args[1]);

    if (packet.isDelivered()) {
        std::cout << "Packet #" << packet.getId() << " delivered: ";
        const auto& route = packet.getRoute();
        for (std::size_t i = 0; i < route.size(); ++i) {
            std::cout << route[i];
            if (i + 1 < route.size()) {
                std::cout << " -> ";
            }
        }
        std::cout << " (latency: " << packet.getTotalCost() << ")\n";
    } else {
        std::cout << "Packet #" << packet.getId() << " DROPPED: no route from " << args[0] << " to "
                   << args[1] << ".\n";
    }
}

void Simulator::cmdFail(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "Usage: FAIL <routerA> <routerB>\n";
        return;
    }
    if (network_.failLink(args[0], args[1])) {
        std::cout << "Link " << args[0] << " <-> " << args[1]
                   << " has FAILED. Routing tables will now avoid this link.\n";
    } else {
        std::cout << "Error: link between '" << args[0] << "' and '" << args[1]
                   << "' does not exist or is already down.\n";
    }
}

void Simulator::cmdRestore(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "Usage: RESTORE <routerA> <routerB>\n";
        return;
    }
    if (network_.restoreLink(args[0], args[1])) {
        std::cout << "Link " << args[0] << " <-> " << args[1] << " has been RESTORED.\n";
    } else {
        std::cout << "Error: link between '" << args[0] << "' and '" << args[1]
                   << "' does not exist or is already up.\n";
    }
}

void Simulator::cmdLoad(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Usage: LOAD <filename>\n";
        return;
    }
    if (network_.loadFromFile(args[0])) {
        std::cout << "Topology loaded from '" << args[0] << "'. (" << network_.getRouterCount()
                   << " routers, " << network_.getLinkCount() << " links)\n";
    } else {
        std::cout << "Error: could not open file '" << args[0] << "'.\n";
    }
}

void Simulator::cmdSave(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Usage: SAVE <filename>\n";
        return;
    }
    if (network_.saveToFile(args[0])) {
        std::cout << "Topology saved to '" << args[0] << "'.\n";
    } else {
        std::cout << "Error: could not write to file '" << args[0] << "'.\n";
    }
}

void Simulator::cmdStats() const {
    std::cout << "\n=== Network Statistics ===\n";
    std::cout << "Total routers:        " << network_.getRouterCount() << "\n";
    std::cout << "Total links:          " << network_.getLinkCount() << "\n";
    std::cout << "Topology changes:     " << network_.getTopologyChangeCount() << "\n";
    std::cout << "Packets sent:         " << packetsSent_ << "\n";
    std::cout << "Packets delivered:    " << packetsDelivered_ << "\n";
    std::cout << "Packets dropped:      " << (packetsSent_ - packetsDelivered_) << "\n";

    std::cout << std::fixed << std::setprecision(2);
    if (packetsDelivered_ > 0) {
        double avgLatency = totalDeliveredLatency_ / static_cast<double>(packetsDelivered_);
        std::cout << "Average latency:      " << avgLatency << "\n";
    } else {
        std::cout << "Average latency:      N/A (no packets delivered yet)\n";
    }
    std::cout << "===========================\n";
}

void Simulator::cmdHelp() const {
    std::cout << "\nAvailable commands:\n"
               << "  ADD <router>                 Add a new router\n"
               << "  REMOVE <router>               Remove an existing router\n"
               << "  LINK <a> <b> <cost>           Create a link between two routers\n"
               << "  UNLINK <a> <b>                Remove a link between two routers\n"
               << "  UPDATE <a> <b> <cost>         Update the cost of an existing link\n"
               << "  PRINT                         Display the full network topology\n"
               << "  PATH <a> <b>                  Compute the shortest path between two routers\n"
               << "  TABLE <router>                Display the routing table for a router\n"
               << "  SEND <a> <b>                  Simulate sending a packet from a to b\n"
               << "  FAIL <a> <b>                  Simulate a link failure between two routers\n"
               << "  RESTORE <a> <b>               Restore a previously failed link\n"
               << "  LOAD <file>                   Load a topology from a text file\n"
               << "  SAVE <file>                   Save the current topology to a text file\n"
               << "  STATS                         Display network statistics\n"
               << "  HELP                          Display this help message\n"
               << "  EXIT / QUIT                   Exit the simulator\n";
}

// ============================================================================
// Helpers
// ============================================================================

Packet Simulator::sendPacket(const std::string& source, const std::string& destination) {
    Packet packet(source, destination);
    ++packetsSent_;

    PathResult result = network_.computeShortestPath(source, destination);
    if (result.reachable) {
        packet.markDelivered(result.totalCost, result.path);
        ++packetsDelivered_;
        totalDeliveredLatency_ += result.totalCost;
    } else {
        packet.markDropped();
    }
    return packet;
}

std::vector<std::string> Simulator::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}
