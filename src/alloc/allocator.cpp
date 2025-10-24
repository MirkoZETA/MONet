#include "allocator.hpp"

Allocator::Allocator() {
  this->network = nullptr;
  this->name = std::string("No name");
}

Allocator::Allocator(std::shared_ptr<Network> network) {
  this->network = network;
  this->name = std::string("No name");
}

void Allocator::exec(
    Network &network,
    std::vector<std::vector<Demand>>& _demands,
    const std::vector<std::shared_ptr<BitRate>>& _bitRates,
    const std::vector<std::unique_ptr<Connection>>& _connections,
    std::vector<std::unique_ptr<Connection>>& _newConnections) {

    throw std::runtime_error(
        "exec method not implemented. "
        "Create a derived class and implement this method, or use the "
        "incremental allocation macros for simplified development.");
}

void Allocator::alloc(Connection connection) {
  int id = connection.getId();

  // Allocate resources for the connection
  for (int i = 0; i < connection.getLinks().size(); i++) {
    int linkIdx = connection.getLinks()[i];
    int fiberIdx = connection.getFibers()[i];
    int coreIdx = connection.getCores()[i];
    fns::Band band = connection.getBands()[i];
    int modeIdx = connection.getModes()[i];

    this->network->useSlots(
        linkIdx,
        fiberIdx,
        coreIdx,
        band,
        modeIdx,
        connection.getSlots()[i][0],
        connection.getSlots()[i].back() + 1,
        id);
  }
}

std::string Allocator::getName(void) const { return this->name; }

std::shared_ptr<Network> Allocator::getNetwork(void) { 
    return this->network; 
}

void Allocator::setNetwork(std::shared_ptr<Network> network) { this->network = network; }