#include "node.hpp"

Node::Node(void) {
  this->id = -1;
  this->dcs = std::nullopt;
  this->ixps = std::nullopt;
  this->longitude = std::nullopt;
  this->latitude = std::nullopt;
  this->population = std::nullopt;
  this->param1 = std::nullopt;
  this->param2 = std::nullopt;
  this->label = std::nullopt;
}

Node::Node(const Node &node) {
  this->id = node.id;
  this->dcs = node.dcs;
  this->ixps = node.ixps;
  this->longitude = node.longitude;
  this->latitude = node.latitude;
  this->population = node.population;
  this->param1 = node.param1;
  this->param2 = node.param2;
  this->label = node.label;
}

Node::Node(int id, std::optional<std::string> label) {
  this->id = id;
  this->dcs = std::nullopt;
  this->ixps = std::nullopt;
  this->longitude = std::nullopt;
  this->latitude = std::nullopt;
  this->population = std::nullopt;
  this->param1 = std::nullopt;
  this->param2 = std::nullopt;
  this->label = label;
}

Node::Node(int id, std::optional<int> dcs, std::optional<int> ixps, std::string label) {
  this->id = id;
  this->dcs = dcs;
  this->ixps = ixps;
  this->longitude = std::nullopt;
  this->latitude = std::nullopt;
  this->population = std::nullopt;
  this->param1 = std::nullopt;
  this->param2 = std::nullopt;
  this->label = label;
}

Node::Node(int id, 
           std::optional<int> dcs,
           std::optional<int> ixps,
           std::optional<double> population,
           std::optional<std::string> label,
           std::optional<double> longitude,
           std::optional<double> latitude,
           std::optional<double> param1,
           std::optional<double> param2) {
  this->id = id;
  this->dcs = dcs;
  this->ixps = ixps;
  this->population = population;
  this->label = label;
  this->longitude = longitude;
  this->latitude = latitude;
  this->param1 = param1;
  this->param2 = param2;
}

void Node::setId(int id) {
  if (this->id != -1)
    throw std::runtime_error(
        "Cannot set Id to a Node with Id different than -1.");
  this->id = id;
}

int Node::getId(void) const { return this->id; }

void Node::setDCs(std::optional<int> dcs) {
  if (dcs.has_value() && dcs.value() < 0) {
    throw std::invalid_argument("Number of DCs cannot be negative.");
  }
  this->dcs = dcs;
}
std::optional<int> Node::getDCs(void) const {
  if (!this->dcs.has_value()) {
    throw NodeAttributeNotSetException("Number of DCs");
  }
  return this->dcs;
}

void Node::setIXPs(std::optional<int> ixps) {
  if (ixps.has_value() && ixps.value() < 0) {
    throw std::invalid_argument("Number of IXPs cannot be negative.");
  }
  this->ixps = ixps;
}
std::optional<int> Node::getIXPs(void) const {
  if (!this->ixps.has_value()) {
    throw NodeAttributeNotSetException("Number of IXPs");
  }
  return this->ixps;
}

void Node::setLongitude(std::optional<double> longitude) {
  if (longitude.has_value() && (longitude.value() < -180.0 || longitude.value() > 180.0)) {
    throw std::invalid_argument("Longitude must be between -180 and 180 degrees.");
  }
  this->longitude = longitude;
}
std::optional<double> Node::getLongitude(void) const {
  if (!this->longitude.has_value()) {
    throw NodeAttributeNotSetException("Longitude");
  }
  return this->longitude;
}

void Node::setLatitude(std::optional<double> latitude) {
  if (latitude.has_value() && (latitude.value() < -90.0 || latitude.value() > 90.0)) {
    throw std::invalid_argument("Latitude must be between -90 and 90 degrees.");
  }
  this->latitude = latitude;
}
std::optional<double> Node::getLatitude(void) const {
  if (!this->latitude.has_value()) {
    throw NodeAttributeNotSetException("Latitude");
  }
  return this->latitude;
}

void Node::setPopulation(std::optional<double> population) {
  this->population = population;
}
std::optional<double> Node::getPopulation(void) const {
  if (!this->population.has_value()) {
    throw NodeAttributeNotSetException("Population");
  }
  return this->population;
}

void Node::setParam1(std::optional<double> param1) {
  this->param1 = param1;
}
std::optional<double> Node::getParam1(void) const {
  if (!this->param1.has_value()) {
    throw NodeAttributeNotSetException("Parameter 1");
  }
  return this->param1;
}

void Node::setParam2(std::optional<double> param2) {
  this->param2 = param2;
}
std::optional<double> Node::getParam2(void) const {
  if (!this->param2.has_value()) {
    throw NodeAttributeNotSetException("Parameter 2");
  }
  return this->param2;
}

void Node::setLabel(std::optional<std::string> label) {
  this->label = label;
}
std::optional<std::string> Node::getLabel(void) const {
  if (!this->label.has_value()) {
    throw NodeAttributeNotSetException("Label");
  }
  return this->label;
}

void Node::setDegree(int degree) {
  if (degree < 0) {
    throw std::invalid_argument("Node degree cannot be negative.");
  }
  this->degree = degree;
}
int Node::getDegree(void) const {
  if (this->degree < 0) {
    throw NodeAttributeNotSetException("Node degree");
  }
  return this->degree;
}