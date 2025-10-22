#include "dummy_allocator.hpp"

DummyAllocator::DummyAllocator(std::shared_ptr<Network> network) : Allocator(network) {
  this->name = std::string("Dummy Allocator");
}

void DummyAllocator::exec(
  Network &network,
  std::vector<std::vector<Demand>>& _demands,
  const std::vector<std::shared_ptr<BitRate>>& _bitRates,
  const std::vector<std::unique_ptr<Connection>>& _connections,
  std::vector<std::unique_ptr<Connection>>& _newConnections) {
  for (auto &demand_row : _demands) {
    for (auto &demand : demand_row) {
      if (!demand.isProvisioned()) {
        int src = demand.getSrc();
        int dst = demand.getDst();
        int link = this->network->isConnected(src, dst);
        if (link != -1) {
          if (!this->network->isSlotUsed(link, 0, 0, fns::Band::C, 0, 2)) {
            auto new_connection = std::make_unique<Connection>();
            new_connection->addLink(link, 0, 0, fns::Band::C, 0, 0, 2);
            _newConnections.push_back(std::move(new_connection));
          }
        }
      }
    }
  }
}

DummyAllocator::~DummyAllocator() {}
DummyAllocator::DummyAllocator() : Allocator() {}
