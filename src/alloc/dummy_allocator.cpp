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
        auto linkIds = this->network->isConnected(src, dst);
        for (int linkId : linkIds) {
          if (!this->network->isSlotUsed(linkId, 0, 0, fns::Band::C, 0, 2)) {
            auto new_connection = std::make_unique<Connection>();
            new_connection->addLink(linkId, 0, 0, fns::Band::C, 0, 0, 2);
            _newConnections.push_back(std::move(new_connection));
            break;
          }
        }
      }
    }
  }
}

DummyAllocator::~DummyAllocator() {}
DummyAllocator::DummyAllocator() : Allocator() {}
