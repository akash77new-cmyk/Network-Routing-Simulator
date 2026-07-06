#include "Packet.h"

#include <utility>

int Packet::nextId_ = 1;

Packet::Packet(std::string source, std::string destination)
    : id_(nextId_++), source_(std::move(source)), destination_(std::move(destination)) {}

const std::string& Packet::getSource() const {
    return source_;
}

const std::string& Packet::getDestination() const {
    return destination_;
}

int Packet::getId() const {
    return id_;
}

void Packet::markDelivered(int totalCost, std::vector<std::string> route) {
    delivered_ = true;
    totalCost_ = totalCost;
    route_ = std::move(route);
}

void Packet::markDropped() {
    delivered_ = false;
    totalCost_ = 0;
}

bool Packet::isDelivered() const {
    return delivered_;
}

int Packet::getTotalCost() const {
    return totalCost_;
}

const std::vector<std::string>& Packet::getRoute() const {
    return route_;
}
