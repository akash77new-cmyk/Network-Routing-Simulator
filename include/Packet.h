#ifndef NETWORK_ROUTING_SIMULATOR_PACKET_H
#define NETWORK_ROUTING_SIMULATOR_PACKET_H

#include <string>
#include <vector>

/**
 * Packet
 * ------
 * Models a single simulated packet traveling through the network. A Packet
 * does not know how to route itself -- that logic lives in Network /
 * Simulator (via Dijkstra + routing tables). Instead, Packet simply records
 * the outcome of a send attempt: whether it was delivered, the route it
 * took, and the accumulated cost (latency). This keeps Packet a plain,
 * easily-testable data object, matching good separation of concerns.
 */
class Packet {
public:
    Packet(std::string source, std::string destination);

    const std::string& getSource() const;
    const std::string& getDestination() const;
    int getId() const;

    // Called once the simulator determines the packet reached its destination.
    void markDelivered(int totalCost, std::vector<std::string> route);

    // Called once the simulator determines the packet could not be delivered.
    void markDropped();

    bool isDelivered() const;
    int getTotalCost() const;
    const std::vector<std::string>& getRoute() const;

private:
    static int nextId_;

    int id_;
    std::string source_;
    std::string destination_;
    bool delivered_ = false;
    int totalCost_ = 0;
    std::vector<std::string> route_;
};

#endif // NETWORK_ROUTING_SIMULATOR_PACKET_H
