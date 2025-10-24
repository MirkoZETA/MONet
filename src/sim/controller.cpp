#include "controller.hpp"

#include <memory>

Controller::Controller() {
  this->network = nullptr;
  this->allocator = std::make_unique<Allocator>();
  this->connections = std::vector<std::unique_ptr<Connection>>();
  this->callbackFunction = nullptr;
  this->failureManagementFunction = nullptr;
  this->recompute = false;
};
Controller::Controller(std::shared_ptr<Network> network) {
  this->network = network;
  this->allocator = std::make_unique<Allocator>();
  this->connections = std::vector<std::unique_ptr<Connection>>();
  this->callbackFunction = nullptr;
  this->recompute = false;
};

// Network management
void Controller::setNetwork(std::shared_ptr<Network> network) {
  this->network = network;
}
std::shared_ptr<Network> Controller::getNetwork(void) const {
  return this->network;
}

// Allocator management
void Controller::setAllocator(std::unique_ptr<Allocator> allocator) {
  this->allocator = std::move(allocator);
}
std::unique_ptr<Allocator> &Controller::getAllocator(void) {
  if (this->allocator == nullptr) {
    throw std::runtime_error("Tried to get a nullptr Allocator. "
                             "Please set an Allocator before using it.");
  }
  return this->allocator;
}

// Callbacks
void Controller::setCallbackFunction(
		void (*callbackFunction)(
				Network &network,
				std::vector<std::vector<Demand>> &demands,
				std::vector<std::unique_ptr<Connection>> &connections,
				double time)) {
  this->callbackFunction = callbackFunction;
}
void Controller::setFailureManagementFunction(
    void (*failureManagementFunction)(
        Network& network,
        std::vector<std::vector<Demand>>& demands,
        std::vector<std::unique_ptr<Connection>>& connectionsAffected,
        eventType eventType,
        double time)) {
  this->failureManagementFunction = failureManagementFunction;
}

// Connection management
void Controller::addConnection(std::unique_ptr<Connection>&& connection) {
  if (!connection) {
    throw std::invalid_argument("Cannot add a null Connection");
  }
  this->connections.push_back(std::move(connection));
}
Connection& Controller::getConnection(int idConnection) {
  if (idConnection < 0 || idConnection >= this->connections.size()) {
    throw std::out_of_range("Connection ID out of range");
  }
  if (!this->connections[idConnection]) {
    throw std::runtime_error("Connection object is null");
  }
  return *this->connections[idConnection];
}
std::vector<std::unique_ptr<Connection>>& Controller::getConnections() {
  return this->connections;
}
void Controller::assignConnections(
    std::vector<std::vector<Demand>>& demands,
    const std::vector<std::shared_ptr<BitRate>>& bitRates,
    double time)
{
  auto networkCopy = std::make_shared<Network>(*this->network);

  // demands is by-value matrix now; copying is fine
  auto demandsCopy = demands;

  std::vector<std::unique_ptr<Connection>> newConnections;

  this->allocator->setNetwork(networkCopy);
  this->allocator->exec(*(this->allocator->getNetwork()), demandsCopy, bitRates, this->connections, newConnections);

  // Reserve for unique_ptr slots
  this->connections.reserve(this->connections.size() + newConnections.size());

  for (auto& connectionPtr : newConnections) {
    Connection& conn = *connectionPtr;
    conn.setId(this->connections.size());
    conn.setTime(time);

    const int src = conn.getSrc();
    const int dst = conn.getDst();

    demands[src][dst].addAllocatedCapacity(conn.getBitrate()->getBitRate());

    for (size_t i = 0; i < conn.getLinks().size(); i++) {
      this->network->useSlots(
        conn.getLinks()[i], 
        conn.getFibers()[i], 
        conn.getCores()[i],
        conn.getBands()[i],
        conn.getModes()[i], 
        conn.getSlots()[i][0], 
        conn.getSlots()[i].back() + 1,
        conn.getId());
    }

    // Move each Connection into a unique_ptr owned by Controller
    this->addConnection(std::move(connectionPtr));
  }
  // Call the user-defined callback function after processing connections
  if (this->callbackFunction) {
      this->callbackFunction(
          *this->network,
          demands,
          this->connections,
          time
      );
  }
  // Recompute paths if needed
  // This is done lazily to avoid unnecessary recomputations
  if (this->recompute) {
    if (!this->network) {
      throw std::runtime_error("Network must be set before recomputing paths");
    }
    int k = this->network->getPathK();
    if (k > 0) {
      this->network->clearPaths();
      this->network->setPaths(k);
      this->recompute = false;
    }
  }
}

// Paths management (this goes to Network.cpp) -> setPaths(std::string filename) { this->network->setPaths(filename); }
void Controller::setPaths(std::string filename) {
  if (!this->network) {
    throw std::runtime_error("Network must be set before setting paths");
  }
  this->network->setPaths(filename);
}
// (this goes to Network.cpp) and has to be implemented as setPaths(int k) { this->network->setPaths(k); }
void Controller::setPaths(int k) {
  // Check if network is set
  if (!this->network) {
    throw std::runtime_error("Network must be set before computing paths");
  }
  this->network->setPaths(k);
}
// (this goes to Network.cpp) -> getPaths(void) const { return this->network->getPaths(); }
Paths* Controller::getPaths(void) const {
  if (!this->network) {
    throw std::runtime_error("Network must be set before getting paths");
  }
  return this->network->getPaths();
}
// (this goes to Network.cpp) -> clearPaths(void) { this->network->clearPaths(); }
void Controller::clearPaths() {
  if (!this->network) {
    throw std::runtime_error("Network must be set before clearing paths");
  }
  this->network->clearPaths();
}

// P2P management
void Controller::addP2P(int src, int dst, int pathIdx, std::vector<int> fiberIdxs) {
  if (!this->network) {
    throw std::runtime_error("Network must be set before adding P2P connections");
  }
  if (src < 0 || src >= this->network->getNumberOfNodes() || 
    dst < 0 || dst >= this->network->getNumberOfNodes()) {
    throw std::invalid_argument("Invalid source or destination node index");
  }
  auto paths = this->network->getPaths();
  if (!paths || paths->empty()) {
    throw std::runtime_error("Paths must be computed before adding P2P connections");
  }
  if (pathIdx < 0 || pathIdx >= static_cast<int>((*paths)[src][dst].size())) {
    throw std::out_of_range("Invalid path index for the given source and destination");
  }

  const auto& pathLinks = (*paths)[src][dst][pathIdx];
  if (static_cast<int>(fiberIdxs.size()) != static_cast<int>(pathLinks.size())) {
    throw std::invalid_argument("Size of fiberIdxs must match number of links in the path");
  }
  auto newP2P = std::make_unique<P2P>(this->p2ps.size(), src, dst);

  for (size_t i = 0; i < pathLinks.size(); ++i) {
    const auto& link = pathLinks[i];
    int fiberIdx = fiberIdxs[i];

    // Validate fiber index
    if (fiberIdx < 0 || fiberIdx >= link.get()->getNumberOfFibers()) {
      throw std::out_of_range("Invalid fiber index for link " + std::to_string(link.get()->getId()));
    }

    auto existingFiber = link.get()->getFiber(fiberIdx);
    if (existingFiber->isActive() || existingFiber->isDedicatedToP2P()) {
      throw std::invalid_argument("Fiber " + std::to_string(fiberIdx) + " on link " + 
                                  std::to_string(link.get()->getId()) + " is already active or assigned to another P2P");
    }
    // Mark existing fiber as dedicated to P2P
    // (this is done by addFiber method of P2P)
    newP2P->addFiber(link.get()->getId(), fiberIdx, existingFiber);
  }
  this->p2ps.push_back(std::move(newP2P));
}
P2P& Controller::getP2P(int id) {
  if (id < 0 || id >= static_cast<int>(this->p2ps.size())) {
    throw std::out_of_range("P2P ID out of range");
  }
  if (!this->p2ps[id]) {
    throw std::runtime_error("P2P object is null");
  }
  return *this->p2ps[id];
}
std::vector<std::unique_ptr<P2P>>& Controller::getP2Ps() {
  return this->p2ps;
}
void Controller::addP2P(int src, int dst, int pathIdx, const std::map<fns::Band, std::vector<std::vector<int>>>& bandSlotMatrix) {
  if (!this->network) {
    throw std::runtime_error("Network must be set before adding P2P connections");
  }
  if (src < 0 || src >= this->network->getNumberOfNodes() || 
    dst < 0 || dst >= this->network->getNumberOfNodes()) {
    throw std::invalid_argument("Invalid source or destination node index");
  }
  auto paths = this->network->getPaths();
  if (!paths || paths->empty()) {
    throw std::runtime_error("Paths must be computed before adding P2P connections");
  }
  if (pathIdx < 0 || pathIdx >= static_cast<int>((*paths)[src][dst].size())) {
    throw std::out_of_range("Invalid path index for the given source and destination");
  }

  auto newP2P = std::make_unique<P2P>(this->p2ps.size(), src, dst);

  for (const auto& link : (*paths)[src][dst][pathIdx]) {
    auto newFiber = std::make_shared<Fiber>(bandSlotMatrix);

    // Add fiber to the actual Link in the Network
    link.get()->addFiber(newFiber);

    // Add fiber to the P2P object
    // This also marks the fiber as dedicated to P2P
    newP2P->addFiber(link.get()->getId(), link.get()->getFibers().size(), newFiber);
    
  }
  this->p2ps.push_back(std::move(newP2P));
}
void Controller::migrateConnectionToP2P(int p2pId, int core, fns::Band band, int mode, int slotFrom, int slotTo, int idConnection, std::shared_ptr<const BitRate> bitRate) {
  auto& conn = this->getConnection(idConnection);
  if (conn.isAllocatedInP2P()) {
    throw std::runtime_error("Connection is already allocated in P2P");
  }
  this->getP2P(p2pId).useSlots(core, band, mode, slotFrom, slotTo, idConnection);
  conn.setAllocatedInP2P(true);
  conn.changeBitRate(bitRate);

  // Free up the previous resources in the network
  for (size_t i = 0; i < conn.getLinks().size(); i++) {
    this->network->unuseSlots(
      conn.getLinks()[i], 
      conn.getFibers()[i], 
      conn.getCores()[i], conn.getBands()[i], conn.getModes()[i], 
      conn.getSlots()[i][0], 
      conn.getSlots()[i].back() + 1);
  }

  // Clear
  conn.links.clear();
  conn.fibers.clear();
  conn.cores.clear();
  conn.bands.clear();
  conn.modes.clear();
  conn.slots.clear();

  // Update connection to reflect P2P allocation
  for (size_t i = 0; i < this->getP2P(p2pId).getLinkIds().size(); i++) {
    // Get the values from P2P
    int linkId = this->getP2P(p2pId).getLinkIds()[i];
    auto fiberIdx = this->getP2P(p2pId).getFiberIdxs()[i];

    // Update connection object
    conn.addLink(linkId, fiberIdx, core, band, mode, slotFrom, slotTo);
  }
}

// Network modification methods (dont touch yet)
void Controller::addLink(
    int src, int dst, float length,
    const std::map<fns::Band, std::vector<std::vector<int>>>& bandSlotMatrix) {

  if (!this->network) {
    throw std::runtime_error("Network must be set before adding links");
  }

  auto addOne = [&](int s, int d) {
    auto fiber = std::make_shared<Fiber>(bandSlotMatrix);

    const int new_id = this->network->getNumberOfLinks();
    auto link = std::make_shared<Link>(new_id, length, fiber);
    link->setSrc(s);
    link->setDst(d);

    this->network->addLink(link);
    this->network->connect(s, new_id, d);
  };

  // Add forward (and reverse) without rebuilding in-between
  addOne(src, dst);
  addOne(dst, src);

  this->recompute = true;
}
void Controller::addNode(int id,
                         std::optional<int> dcs,
                         std::optional<int> ixps,
                         std::optional<double> population,
                         std::optional<std::string> label,
                         std::optional<double> longitude,
                         std::optional<double> latitude,
                         std::optional<double> param1,
                         std::optional<double> param2) {
  if (!this->network) {
    throw std::runtime_error("Network must be set before adding nodes");
  }
  int new_id = this->network->getNumberOfNodes();
  auto node = std::make_unique<Node>(new_id, dcs, ixps, population, label, longitude, latitude, param1, param2);
  this->network->addNode(std::move(node));

  this->recompute = true;
}
