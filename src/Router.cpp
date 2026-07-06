#include "Router.h"

Router::Router(std::string id) : id_(std::move(id)) {}

const std::string& Router::getId() const {
    return id_;
}

bool Router::operator==(const Router& other) const {
    return id_ == other.id_;
}
