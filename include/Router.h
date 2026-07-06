#ifndef NETWORK_ROUTING_SIMULATOR_ROUTER_H
#define NETWORK_ROUTING_SIMULATOR_ROUTER_H

#include <string>

/**
 * Router
 * ------
 * Represents a single node (router) in the network graph. The Router class
 * intentionally stays small: the graph topology (which routers connect to
 * which) is owned and managed by the Network class. This keeps a clean
 * separation of concerns -- Router models "what a router is", Network
 * models "how routers relate to each other".
 */
class Router {
public:
    Router() = default;
    explicit Router(std::string id);

    const std::string& getId() const;

    bool operator==(const Router& other) const;

private:
    std::string id_;
};

#endif // NETWORK_ROUTING_SIMULATOR_ROUTER_H
