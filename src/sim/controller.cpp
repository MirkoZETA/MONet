#include "controller.hpp"

#include <fstream>
#include <memory>
#include <queue>
#include <algorithm>
#include <limits>
#include <unordered_set>

Controller::Controller() {
  this->network = nullptr;
  this->allocator = std::make_unique<Allocator>();
  this->connections = std::vector<std::unique_ptr<Connection>>();
  this->path = std::make_shared<Paths>();
  this->callbackFunction = nullptr;
  this->failureManagementFunction = nullptr;
  this->recompute = false;
};
Controller::Controller(std::shared_ptr<Network> network) {
  this->network = network;
  this->allocator = std::make_unique<Allocator>();
  this->connections = std::vector<std::unique_ptr<Connection>>();
  this->path = std::make_shared<Paths>();
  this->callbackFunction = nullptr;
  this->recompute = false;
  if (this->network) {
    buildAdjacencyList();
  }
};

// Network management
void Controller::setNetwork(std::shared_ptr<Network> network) {
  this->network = network;
  if (this->network) {
    buildAdjacencyList();
  }
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
  if (this->recompute && this->k > 0) {
    this->clearPaths();
    this->buildAdjacencyList();
    this->setPaths(this->k);
    this->recompute = false;
  }
}

// Paths management
void Controller::setPaths(std::string filename) {
  // open JSON file
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }
  
  nlohmann::json filePaths;
  file >> filePaths;

  // Check if network is set
  if (!this->network) {
    throw std::runtime_error("Network must be set before setting paths");
  }
  // Check if routes field exists
  if (!filePaths.contains("routes")) {
    throw std::runtime_error("Error in file: " + filename + 
                             ". 'routes' field is missing.");
  }

  int numberOfNodes = this->network->getNumberOfNodes();

  // allocate space for path[src]
  this->path->resize(numberOfNodes);

  // allocate space for path[src][dst]
  for (int t = 0; t < numberOfNodes; t++) {
    (*this->path)[t].resize(numberOfNodes);
  }

  int routesNumber = filePaths["routes"].size();
  int maxK = 0;

  for (int i = 0; i < routesNumber; i++) {
    const auto& route = filePaths["routes"][i];
    
    // Check required fields exist
    if (!route.contains("src") || !route.contains("dst") || !route.contains("paths")) {
      throw std::runtime_error("Error in file: " + filename + 
                               ". Each route must contain 'src', 'dst', and 'paths' fields.");
    }

    int src = route["src"];
    int dst = route["dst"];

    // Validate node indices
    if (src < 0 || src >= numberOfNodes || dst < 0 || dst >= numberOfNodes) {
      throw std::runtime_error("Invalid node index in route: src=" + std::to_string(src) + 
                               ", dst=" + std::to_string(dst));
    }
    int pathsNumber = route["paths"].size();
    if (pathsNumber == 0) {
      continue; // Skip empty paths
    }
    if (pathsNumber > maxK) {
      maxK = pathsNumber;
    }

    // allocate path[src][dst][pathsNumber]
    (*this->path)[src][dst].resize(pathsNumber);

    // go through available routes
    for (int b = 0; b < pathsNumber; b++) {
      const auto& pathNodes = route["paths"][b];
      int nodesPathNumber = pathNodes.size();
      if (nodesPathNumber < 2) {
        throw std::runtime_error("Error in file: " + filename + 
                                 ". Each path must contain at least two nodes.");
      }
      int lastNode = nodesPathNumber - 1;
      for (int c = 0; c < lastNode; c++) {
        int actNode = pathNodes[c];
        int nextNode = pathNodes[c + 1];

        // Validate node indices
        if (actNode < 0 || actNode >= numberOfNodes || 
            nextNode < 0 || nextNode >= numberOfNodes) {
          throw std::runtime_error("Invalid node index in path: " + 
                                   std::to_string(actNode) + " -> " + std::to_string(nextNode));
        }

        int idLink = this->network->isConnected(actNode, nextNode);
        if (idLink == -1) {
          throw std::runtime_error("No link found between nodes " + 
                                   std::to_string(actNode) + " and " + std::to_string(nextNode));
        }

        (*this->path)[src][dst][b].push_back(this->network->getLink(idLink));
      }
    }
  }
  this->k = maxK;
}
void Controller::setPaths(int k) {
  // Check if network is set
  if (!this->network) {
    throw std::runtime_error("Network must be set before computing paths");
  }
  this->k = k;

  int numberOfNodes = this->network->getNumberOfNodes();
  
  // Initialize paths structure
  this->path->resize(numberOfNodes);
  for (int i = 0; i < numberOfNodes; i++) {
    (*this->path)[i].resize(numberOfNodes);
  }

  // Compute k shortest paths for all pairs where src < dst
  for (int src = 0; src < numberOfNodes; src++) {
    for (int dst = src + 1; dst < numberOfNodes; dst++) {

      if (src == dst) continue;

      // Compute k shortest paths from src to dst using Yen's algorithm
      std::vector<std::vector<int>> kPaths = yenKShortestPaths(src, dst, k);
      
      if (kPaths.empty()) continue;

      // Store paths for src -> dst
      (*this->path)[src][dst].reserve(kPaths.size());
      for (const auto& nodePath : kPaths) {
        Route route;
        route.reserve(nodePath.size() - 1);
        for (size_t i = 0; i < nodePath.size() - 1; i++) {
          int linkId = this->network->isConnected(nodePath[i], nodePath[i + 1]);
          if (linkId != -1) {
            route.push_back(this->network->getLink(linkId));
          }
        }
        (*this->path)[src][dst].push_back(route);
      }
      // Store paths for dst -> src (reverse paths)
      (*this->path)[dst][src].reserve(kPaths.size());
      for (const auto& nodePath : kPaths) {
        Route reverseRoute;
        reverseRoute.reserve(nodePath.size() - 1);
        
        // Traverse the node path in reverse order
        for (int i = nodePath.size() - 1; i > 0; i--) {
          int linkId = this->network->isConnected(nodePath[i], nodePath[i - 1]);
          if (linkId != -1) {
            reverseRoute.push_back(this->network->getLink(linkId));
          }
        }
        (*this->path)[dst][src].push_back(reverseRoute);
      }
    }
  }
}
std::shared_ptr<Paths> Controller::getPaths(void) const {
  return this->path;
}
void Controller::clearPaths() {
  if (this->path) {
    // Clear out all stored routes
    for (auto &row : *this->path) {
      for (auto &col : row) {
        for (auto &routes : col) {
          routes.clear(); // clear each Route (vector<shared_ptr<Link>>)
        }
        col.clear(); // clear vector<Route>
      }
      row.clear(); // clear vector<vector<Route>>
    }
    this->path->clear(); // clear top-level Paths
  }

  // Reset adjacency list
  this->adj.clear();
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
  if (this->path.get()->empty()) {
    throw std::runtime_error("Paths must be computed before adding P2P connections");
  }
  if (pathIdx < 0 || pathIdx >= static_cast<int>((*this->path)[src][dst].size())) {
    throw std::out_of_range("Invalid path index for the given source and destination");
  }

  const auto& pathLinks = (*this->path)[src][dst][pathIdx];
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
  if (this->path.get()->empty()) {
    throw std::runtime_error("Paths must be computed before adding P2P connections");
  }
  if (pathIdx < 0 || pathIdx >= static_cast<int>((*this->path)[src][dst].size())) {
    throw std::out_of_range("Invalid path index for the given source and destination");
  }

  auto newP2P = std::make_unique<P2P>(this->p2ps.size(), src, dst);

  for (const auto& link : (*this->path)[src][dst][pathIdx]) {
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

// Network modification methods
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


// Path helpers
void Controller::buildAdjacencyList() {
    if (!network) return;
    int numberOfNodes = network->getNumberOfNodes();
    adj.assign(numberOfNodes, std::vector<Edge>());
    
    // Initialize node degrees to 0
    std::vector<int> nodeDegrees(numberOfNodes, 0);
    
    // Build adjacency list and count out-degrees
    for (const auto& link : network->getLinks()) {
        int srcNode = link->getSrc();
        adj[srcNode].push_back({link->getDst(), link->getId(), link->getLength()});
        
        // Count out-degree for each node
        if (srcNode >= 0 && srcNode < numberOfNodes) {
            nodeDegrees[srcNode]++;
        }
    }
    
    // Set the degree attribute for each node
    for (int i = 0; i < numberOfNodes; i++) {
        network->getNode(i)->setDegree(nodeDegrees[i]);
    }
}
std::vector<int> Controller::dijkstra(int src, int dst, const std::unordered_set<int>& excludedLinks, const std::unordered_set<int>& excludedNodes) {
  int numberOfNodes = this->network->getNumberOfNodes();
  
  if (excludedNodes.count(src) || excludedNodes.count(dst)) {
      return {};
  }

  std::vector<double> distances(numberOfNodes, std::numeric_limits<double>::infinity());
  std::vector<int> previous(numberOfNodes, -1);
  std::vector<char> visited(numberOfNodes, 0);
  
  distances[src] = 0.0;
  
  using PQElement = std::pair<double, int>;
  std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;
  pq.push({0.0, src});
  
  while (!pq.empty()) {
    auto [currentDist, currentNode] = pq.top();
    pq.pop();
    
    if (visited[currentNode] || excludedNodes.count(currentNode)) continue;
    visited[currentNode] = 1;
    
    if (currentNode == dst) break;
    
    if (currentNode >= adj.size()) continue;

    for (const auto& edge : adj[currentNode]) {
      if (excludedLinks.count(edge.linkId)) continue;
      
      double newDist = currentDist + edge.w;
      
      if (newDist < distances[edge.to]) {
        distances[edge.to] = newDist;
        previous[edge.to] = currentNode;
        pq.push({newDist, edge.to});
      }
    }
  }
  if (distances[dst] == std::numeric_limits<double>::infinity()) {
    return {}; // No path found
  }
  std::vector<int> path;
  for (int current = dst; current != -1; current = previous[current]) {
    path.push_back(current);
  }
  std::reverse(path.begin(), path.end());
  return path;
}
std::vector<std::vector<int>> Controller::yenKShortestPaths(int src, int dst, int k) {
  std::vector<std::vector<int>> kPaths;
  
  std::vector<int> shortestPath = dijkstra(src, dst);
  if (shortestPath.empty()) {
    return kPaths;
  }
  
  kPaths.push_back(shortestPath);
  
  using Candidate = std::pair<double, std::vector<int>>;
  std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> candidates;
  std::unordered_set<std::vector<int>, VecHash> candidateSet;

  for (int pathCount = 1; pathCount < k; pathCount++) {
    const std::vector<int>& previousPath = kPaths.back();
    
    for (size_t i = 0; i < previousPath.size() - 1; i++) {
      int spurNode = previousPath[i];
      std::vector<int> rootPath(previousPath.begin(), previousPath.begin() + i + 1);
      
      std::unordered_set<int> removedLinks;
      for (const auto& path : kPaths) {
        if (path.size() > i && std::equal(rootPath.begin(), rootPath.end(), path.begin())) {
          if (i + 1 < path.size()) {
            int linkId = this->network->isConnected(path[i], path[i + 1]);
            if (linkId != -1) {
              removedLinks.insert(linkId);
            }
          }
        }
      }
      
      std::unordered_set<int> excludedNodes;
      for(size_t j = 0; j < i; ++j) {
          excludedNodes.insert(previousPath[j]);
      }
      
      std::vector<int> spurPath = dijkstra(spurNode, dst, removedLinks, excludedNodes);
      
      if (!spurPath.empty() && spurPath.size() > 1) {
        std::vector<int> totalPath = rootPath;
        totalPath.insert(totalPath.end(), spurPath.begin() + 1, spurPath.end());
        
        if (candidateSet.find(totalPath) == candidateSet.end()) {
          double pathLength = 0.0;
          for (size_t j = 0; j < totalPath.size() - 1; j++) {
            int linkId = this->network->isConnected(totalPath[j], totalPath[j + 1]);
            if (linkId != -1) {
              pathLength += this->network->getLink(linkId)->getLength();
            } else {
              pathLength = std::numeric_limits<double>::infinity();
              break;
            }
          }
          
          if(pathLength != std::numeric_limits<double>::infinity()){
            candidates.push({pathLength, totalPath});
            candidateSet.insert(totalPath);
          }
        }
      }
    }
    
    if (candidates.empty()) {
      break;
    }
    
    kPaths.push_back(candidates.top().second);
    candidateSet.erase(candidates.top().second);
    candidates.pop();
  }
  
  return kPaths;
}