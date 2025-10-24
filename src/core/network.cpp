#include "network.hpp"

#include <cmath>   // For pow() function used in nodalVariance() metric calculation
#include <queue>   // For std::priority_queue used in Dijkstra and Yen's k-shortest paths algorithms

Network::Network(void) {
  this->linkCounter = 0;
  this->nodeCounter = 0;
  this->name = "Unnamed Network";

  this->nodes = std::vector<std::unique_ptr<Node>>();
  this->links = std::vector<std::shared_ptr<Link>>();
  this->linksIn = std::vector<std::shared_ptr<Link>>();
  this->linksOut = std::vector<std::shared_ptr<Link>>();

  this->nodesIn  = std::vector<int>{0};
  this->nodesOut = std::vector<int>{0};
  this->paths = std::make_unique<Paths>();
  this->pathK = 0;
}

Network::Network(std::string filename)
  : Network() {
  // open JSON file
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::invalid_argument("Could not open file: " + filename);
  }
  nlohmann::json network;
  file >> network;

  // Read optional name field, default to "Unnamed Network" if not present
  this->name = network.value("name", "Unnamed Network");

  // number of nodes
  int numberOfNodes = network["nodes"].size();
  // number of links
  int numberOfLinks = network["links"].size();

  // adding nodes to the network
  for (int i = 0; i < numberOfNodes; i++) {
    int id = network["nodes"][i]["id"];

    // Optional fields
    std::optional<int> dcs = network["nodes"][i].contains("DC") 
        ? std::optional<int>(network["nodes"][i]["DC"].get<int>()) 
        : std::nullopt;
    std::optional<int> ixps = network["nodes"][i].contains("IXP") 
        ? std::optional<int>(network["nodes"][i]["IXP"].get<int>()) 
        : std::nullopt;
    std::optional<double> longitude = network["nodes"][i].contains("longitude") 
        ? std::optional<double>(network["nodes"][i]["longitude"].get<double>()) 
        : std::nullopt;
    std::optional<double> latitude = network["nodes"][i].contains("latitude") 
        ? std::optional<double>(network["nodes"][i]["latitude"].get<double>()) 
        : std::nullopt;
    std::optional<double> population = network["nodes"][i].contains("pop") 
        ? std::optional<double>(network["nodes"][i]["pop"].get<double>()) 
        : std::nullopt;

    // Read optional label
    std::optional<std::string> label = network["nodes"][i].contains("label") 
        ? std::optional<std::string>(network["nodes"][i]["label"].get<std::string>()) 
        : std::nullopt;

    // Customizable parameters
    std::optional<double> param1 = network["nodes"][i].contains("param1") 
        ? std::optional<double>(network["nodes"][i]["param1"].get<double>()) 
        : std::nullopt;
    std::optional<double> param2 = network["nodes"][i].contains("param2") 
        ? std::optional<double>(network["nodes"][i]["param2"].get<double>()) 
        : std::nullopt;

    std::unique_ptr<Node> node = std::make_unique<Node>(id, dcs, ixps, population, label, longitude, latitude, param1, param2);
    this->addNode(std::move(node));
  }

  // Track link directions for bidirectional validation
  std::map<std::pair<int, int>, int> linkPairs; // (src,dst) -> linkId

  // adding links to the network
  for (int i = 0; i < numberOfLinks; i++) {
    int id = network["links"][i]["id"];
    float length = network["links"][i]["length"];
    int src = network["links"][i]["src"];
    int dst = network["links"][i]["dst"];

    // Store this link direction
    linkPairs[{src, dst}] = id;

    std::shared_ptr<Link> link;

    // Check if "fibers" field exists (multiple fibers)
    if (network["links"][i].contains("fibers")) {
      // Warn if link-level type is specified but will be ignored
      if (network["links"][i].contains("type")) {
        std::cerr << "Warning: Link " << id << " has link-level 'type' field that will be ignored. "
                  << "When using 'fibers' array, type must be set individually for each fiber." << std::endl;
      }
      std::vector<std::shared_ptr<Fiber>> multi_fiber = readMultiFiber(network["links"][i]["fibers"]);
      link = std::make_shared<Link>(id, length, multi_fiber);
    } else {
      std::shared_ptr<Fiber> single_fiber = readSingleFiber(network["links"][i]);
      link = std::make_shared<Link>(id, length, single_fiber);
    }
    
    // Set source and destination nodes for the link
    link.get()->setSrc(src);
    link.get()->setDst(dst);

    // Add the link to the network
    this->addLink(link);

    // Connect the link to the nodes
    this->connect(src, id, dst);
  }
  // Validate bidirectional links
  for (const auto& linkPair : linkPairs) {
    int src = linkPair.first.first;
    int dst = linkPair.first.second;
    int linkId = linkPair.second;
    
    // Check if reverse direction exists
    if (linkPairs.find({dst, src}) == linkPairs.end()) {
      throw std::runtime_error("Network is not bidirectional: Link " + std::to_string(linkId) + 
                               " from node " + std::to_string(src) + " to node " + std::to_string(dst) + 
                               " exists, but reverse link from " + std::to_string(dst) + 
                               " to " + std::to_string(src) + " is missing.");
    }
  }
}

Network::Network(const Network &net) {
  this->linkCounter = net.linkCounter;
  this->nodeCounter = net.nodeCounter;
  this->name = net.name;
  
  // Deep copy nodes
  this->nodes = std::vector<std::unique_ptr<Node>>(net.nodes.size());
  for (unsigned int i = 0; i < this->nodes.size(); i++) {
    this->nodes[i] = std::make_unique<Node>(*net.nodes[i]);
  }
  
  // Deep copy links
  this->links = std::vector<std::shared_ptr<Link>>(net.links.size());
  for (unsigned int i = 0; i < this->links.size(); i++) {
    this->links[i] = std::make_shared<Link>(*net.links[i]);  // Deep copy
  }
  
  // Update linksIn and linksOut to point to the new Link objects
  this->linksIn = std::vector<std::shared_ptr<Link>>(net.linksIn.size());
  for (unsigned int i = 0; i < this->links.size(); i++) {
    for (unsigned int j = 0; j < net.linksIn.size(); j++) {
      if (this->links[i]->getId() == net.linksIn[j]->getId()) {
        this->linksIn[j] = this->links[i];
        break;
      }
    }
  }

  this->linksOut = std::vector<std::shared_ptr<Link>>(net.linksOut.size());
  for (unsigned int i = 0; i < this->links.size(); i++) {
    for (unsigned int j = 0; j < net.linksOut.size(); j++) {
      if (this->links[i]->getId() == net.linksOut[j]->getId()) {
        this->linksOut[j] = this->links[i];
        break;
      }
    }
  }
  this->nodesIn = net.nodesIn;
  this->nodesOut = net.nodesOut;
  if (net.paths) {
    this->paths = std::make_unique<Paths>(*net.paths);
  } else {
    this->paths = std::make_unique<Paths>();
  }
  this->pathK = net.pathK;
}

std::string Network::getName() const {
    return this->name;
}

std::unique_ptr<Node> &Network::getNode(int nodePos) {
  if (nodePos < 0 || nodePos >= static_cast<int>(this->nodes.size()))
    throw std::invalid_argument("Cannot get Node from a position out of bounds.");

  return this->nodes.at(nodePos);
}

void Network::addNode(std::unique_ptr<Node> node) {
  if (node->getId() != this->nodeCounter) {
    throw std::runtime_error(
        "Cannot add a Node to this network with Id mismatching node counter.");
  }
  this->nodeCounter++;
  this->nodes.push_back(std::move(node));
  this->nodesIn.push_back(this->nodesIn.back());     // keep prefix sum
  this->nodesOut.push_back(this->nodesOut.back());   // keep prefix sum
}
// Add a Node to Nodes vector, increases nodesIn/Out size.

void Network::addLink(std::shared_ptr<Link> link) {
  if (link->getId() != Network::linkCounter) {
    throw std::runtime_error(
        "Cannot add a Link to this network with Id mismatching link counter.");
  }
  this->linkCounter++;
  this->links.push_back(link);
}
// Add a Link to Links vector.

void Network::connect(int src, int linkPos,
                      int dst)  // Using Ids and Link from Nodes/Links vectors
{
  if (src < 0 || src >= this->nodeCounter) {
    throw std::runtime_error(
        "Cannot connect src " + std::to_string(src) +
        " because its ID is not in the network. Number of nodes in network: " +
        std::to_string(this->nodeCounter));
  }
  if (dst < 0 || dst >= this->nodeCounter) {
    throw std::runtime_error(
        "Cannot connect dst " + std::to_string(dst) +
        " because its ID is not in the network. Number of nodes in network: " +
        std::to_string(this->nodeCounter));
  }
  if (linkPos < 0 || linkPos >= this->linkCounter) {
    throw std::runtime_error(
        "Cannot use link " + std::to_string(linkPos) +
        " because its ID is not in the network. Number of links in network: " +
        std::to_string(this->linkCounter));
  }
  this->linksOut.insert(this->linksOut.begin() + this->nodesOut.at(src),
                        this->links.at(linkPos));
  std::for_each(this->nodesOut.begin() + src + 1, this->nodesOut.end(),
                [](int &n) { n += 1; });

  this->linksIn.insert(this->linksIn.begin() + this->nodesIn.at(dst),
                       this->links.at(linkPos));
  std::for_each(this->nodesIn.begin() + dst + 1, this->nodesIn.end(),
                [](int &n) { n += 1; });
  this->links.at(linkPos)->src = src;
  this->links.at(linkPos)->dst = dst;
  // Note: Node degrees are set in setPaths() methods, not here
}
// Connects two Nodes through one Link (order is important: src != dst):
//
//       (Source Node) ---Link---> (Destination Node)

std::vector<int> Network::isConnected(int src, int dst) const {
  std::vector<int> linkIds;
  if (src < 0 || src >= this->nodeCounter || dst < 0 || dst >= this->nodeCounter) {
    return linkIds;
  }
  if (src + 1 >= static_cast<int>(this->nodesOut.size())) {
    return linkIds;
  }
  for (int i = this->nodesOut[src]; i < this->nodesOut[src + 1]; i++) {
    const auto& link = this->linksOut[i];
    if (link->getDst() == dst) {
      linkIds.push_back(link->getId());
    }
  }
  return linkIds;
}

int Network::getNumberOfLinks() const { return this->linkCounter; }

std::vector<std::shared_ptr<Link>> Network::getLinks() const {
  return this->links;
}

std::shared_ptr<Link> Network::getLink(int linkPos) {
  if (linkPos < 0 || linkPos >= static_cast<int>(this->links.size()))
    throw std::runtime_error("Cannot get Link from a position out of bounds.");

  return this->links.at(linkPos);
}
// Returns the Link pointer at a "linkPos" index inside Links vector.

std::shared_ptr<Link> Network::getLink(int src, int dst) const {
  for (int i = this->nodesOut[src]; i < this->nodesOut[src + 1]; i++) {
    for (int j = nodesIn[dst]; j < nodesIn[dst + 1]; j++) {
      if (linksOut[i]->getId() == linksIn[j]->getId()) {
        return linksOut[i];
      }
    }
  }
  return nullptr;
}

int Network::getNumberOfNodes() const {
  return this->nodeCounter;
}

std::vector<const Node*> Network::getNodes() const {
  std::vector<const Node*> nodePointers;
  nodePointers.reserve(this->nodes.size());
  for (const auto& nodePtr : this->nodes) {
    nodePointers.push_back(nodePtr.get());
  }
  return nodePointers;
}



std::unique_ptr<Node>& Network::getNode(std::string label) {
  for (auto& nodePtr : this->nodes) {
    if (nodePtr->getLabel().has_value() && nodePtr->getLabel().value() == label) {
      return nodePtr;
    }
  }
  throw std::invalid_argument("No Node with label '" + label + "' found in the network.");
}

void Network::useSlots(int idLink, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo, int connectionId) {
  validateAux(idLink, fiber, core, mode, slotFrom, slotTo);
  for (int i = slotFrom; i < slotTo; i++)
    this->getLink(idLink)->getFiber(fiber).get()->setSlot(core, band, mode, i, connectionId);
}

void Network::unuseSlots(int idLink, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo) {
  validateAux(idLink, fiber, core, mode, slotFrom, slotTo);
  for (int i = slotFrom; i < slotTo; i++)
    this->getLink(idLink)->getFiber(fiber).get()->setSlot(core, band, mode, i, false);
}

int Network::isSlotUsed(int idLink, int fiber, int core, fns::Band band, int mode, int slotPos){
  validateAux(idLink, fiber, core, mode, slotPos);
  return this->getLink(idLink)->getFiber(fiber).get()->getSlot(core, band, mode, slotPos);
}

void Network::validateAux(int idLink, int fiber, int core, int mode, int slotPos) {
  if (idLink < 0)
    throw std::invalid_argument("Link ID must be non-negative.");
  if (idLink >= this->getNumberOfLinks())
    throw std::out_of_range("Link ID exceeds number of links in the network.");
  if (fiber < 0)
    throw std::invalid_argument("Fiber index must be non-negative.");
  if (core < 0)
    throw std::invalid_argument("Core index must be non-negative.");
  if (mode < 0)
    throw std::invalid_argument("Mode index must be non-negative.");
  if (slotPos < 0)
    throw std::invalid_argument("Slot position must be non-negative.");
}

void Network::validateAux(int idLink, int fiber, int core, int mode, int slotFrom, int slotTo) {
  validateAux(idLink, fiber, core, mode, slotFrom);
  if (slotTo <= slotFrom)
    throw std::invalid_argument("Invalid slot range.");
}

// TODO:
float Network::averageNeighborhood() {
  if (this->getNumberOfNodes() == 0)
    throw std::runtime_error("The network must be have at least one node.");
  float result = 0;
  result = this->getNumberOfLinks() / this->getNumberOfNodes();
  return result;
}
float Network::normalAverageNeighborhood() {
  if (this->getNumberOfNodes() == 0)
    throw std::runtime_error("The network must be have at least one node.");
  float result = 0;
  result = (float)this->getNumberOfLinks() /
           (this->getNumberOfNodes() * (this->getNumberOfNodes() - 1));
  return result;
}
float Network::nodalVariance() {
  if (this->getNumberOfNodes() == 0)
    throw std::runtime_error("The network must be have at least one node.");
  float result = 0;
  float average = this->averageNeighborhood();
  for (int i = 0; i < this->getNumberOfNodes(); i++) {
    result += pow((this->nodesOut[i + 1] - this->nodesOut[i]) - average, 2);
  }
  result /= this->getNumberOfNodes();
  return result;
}

void Network::setPaths(int k) {
  if (k <= 0) {
    this->clearPaths();
    this->pathK = 0;
    return;
  }

  if (!this->paths) {
    this->paths = std::make_unique<Paths>();
  }

  const int numberOfNodes = this->getNumberOfNodes();
  // Initialize paths matrix: paths[src][dst] will contain up to k routes
  this->paths->assign(numberOfNodes, std::vector<std::vector<Route>>(numberOfNodes));

  // Compute k-shortest paths for each node pair
  for (int src = 0; src < numberOfNodes; ++src) {
    for (int dst = 0; dst < numberOfNodes; ++dst) {
      // Skip self-loops (src == dst)
      if (src == dst) {
        continue;
      }

      // Use Yen's algorithm to find k shortest paths from src to dst
      auto shortestPaths = this->yenKShortestPaths(src, dst, k);
      if (shortestPaths.empty()) {
        continue;  // No path exists between src and dst
      }

      // Convert ShortestPathResult (with link IDs) to Route (with Link pointers)
      auto& cell = (*this->paths)[src][dst];
      cell.reserve(shortestPaths.size());

      for (const auto& pathResult : shortestPaths) {
        Route route;
        route.reserve(pathResult.linkPath.size());
        // Convert each link ID to a shared_ptr<Link>
        for (int linkId : pathResult.linkPath) {
          route.push_back(this->getLink(linkId));
        }
        cell.push_back(std::move(route));
      }
    }
  }

  // Store the k value for future reference
  this->pathK = k;

  // Update node degrees based on outgoing links (out-degree only)
  for (int node = 0; node < numberOfNodes; ++node) {
    this->nodes.at(node)->setDegree(this->nodesOut[node + 1] - this->nodesOut[node]);
  }
}

void Network::setPaths(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }

  nlohmann::json filePaths;
  file >> filePaths;

  if (!filePaths.contains("routes")) {
    throw std::runtime_error("Error in file: " + filename + ". 'routes' field is missing.");
  }

  if (!this->paths) {
    this->paths = std::make_unique<Paths>();
  }

  const int numberOfNodes = this->getNumberOfNodes();
  this->paths->assign(numberOfNodes, std::vector<std::vector<Route>>(numberOfNodes));

  int maxK = 0;

  // Read each route entry explicitly (no automatic reverse computation)
  for (const auto& route : filePaths["routes"]) {
    if (!route.contains("src") || !route.contains("dst") || !route.contains("paths")) {
      throw std::runtime_error("Error in file: " + filename +
                               ". Each route must contain 'src', 'dst', and 'paths' fields.");
    }

    int src = route["src"];
    int dst = route["dst"];
    if (src < 0 || src >= numberOfNodes || dst < 0 || dst >= numberOfNodes) {
      throw std::runtime_error("Invalid node index in route: src=" + std::to_string(src) +
                               ", dst=" + std::to_string(dst));
    }

    const auto& pathsArray = route["paths"];
    if (!pathsArray.is_array()) {
      throw std::runtime_error("Invalid 'paths' field format in file: " + filename);
    }

    int pathsNumber = pathsArray.size();
    if (pathsNumber == 0) {
      continue;
    }

    // Store paths for the specified direction only (no reverse computation)
    auto& pathsForThisDirection = (*this->paths)[src][dst];
    pathsForThisDirection.resize(pathsNumber);

    for (int pathIdx = 0; pathIdx < pathsNumber; ++pathIdx) {
      const auto& pathLinkIds = pathsArray[pathIdx];
      if (!pathLinkIds.is_array()) {
        throw std::runtime_error("Each path must be an array of link IDs in file: " + filename);
      }

      int linksInPath = pathLinkIds.size();
      if (linksInPath < 1) {
        throw std::runtime_error("Each path must contain at least one link ID in file: " + filename);
      }

      Route route;
      route.reserve(linksInPath);

      // Build the route by converting link IDs to Link pointers
      for (int c = 0; c < linksInPath; ++c) {
        int linkId = pathLinkIds[c];
        if (linkId < 0 || linkId >= this->getNumberOfLinks()) {
          throw std::runtime_error("Invalid link ID in path: " + std::to_string(linkId));
        }
        auto link = this->getLink(linkId);

        // Validate path continuity
        if (c == 0) {
          if (link->getSrc() != src) {
            throw std::runtime_error("First link " + std::to_string(linkId) +
                                     " does not start at source node " + std::to_string(src));
          }
        } else {
          auto prevLink = route.back();
          if (prevLink->getDst() != link->getSrc()) {
            throw std::runtime_error("Path is not continuous: link " +
                                     std::to_string(prevLink->getId()) + " to link " +
                                     std::to_string(linkId));
          }
        }

        route.push_back(link);
      }

      // Validate that path ends at destination
      if (route.back()->getDst() != dst) {
        throw std::runtime_error("Last link " + std::to_string(route.back()->getId()) +
                                 " does not end at destination node " + std::to_string(dst));
      }

      pathsForThisDirection[pathIdx] = std::move(route);
    }

    maxK = std::max(maxK, pathsNumber);
  }

  this->pathK = maxK;

  // Update node degrees based on outgoing links (out-degree only)
  for (int node = 0; node < numberOfNodes; ++node) {
    this->nodes.at(node)->setDegree(this->nodesOut[node + 1] - this->nodesOut[node]);
  }
}

Paths* Network::getPaths() const {
  return this->paths.get();
}

void Network::clearPaths() {
  if (this->paths) {
    this->paths->clear();
  }
  this->pathK = 0;
}

int Network::getPathK() const {
  return this->pathK;
}

Network::ShortestPathResult Network::dijkstra(
    int src,
    int dst,
    const std::unordered_set<int>& excludedLinks,
    const std::unordered_set<int>& excludedNodes) const {
  ShortestPathResult result;

  const int numberOfNodes = this->nodeCounter;
  if (src < 0 || src >= numberOfNodes || dst < 0 || dst >= numberOfNodes) {
    return result;
  }
  if (excludedNodes.count(src) || excludedNodes.count(dst)) {
    return result;
  }

  std::vector<double> distances(numberOfNodes, std::numeric_limits<double>::infinity());
  std::vector<int> previousNode(numberOfNodes, -1);
  std::vector<int> previousLink(numberOfNodes, -1);
  std::vector<char> visited(numberOfNodes, 0);

  distances[src] = 0.0;

  using QueueEntry = std::pair<double, int>;
  struct QueueCompare {
    bool operator()(const QueueEntry& lhs, const QueueEntry& rhs) const {
      return lhs.first > rhs.first;
    }
  };
  std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueCompare> pq;
  pq.push({0.0, src});

  while (!pq.empty()) {
    auto [currentDistance, currentNode] = pq.top();
    pq.pop();

    if (visited[currentNode]) {
      continue;
    }
    if (excludedNodes.count(currentNode)) {
      visited[currentNode] = 1;
      continue;
    }
    visited[currentNode] = 1;

    if (currentNode == dst) {
      break;
    }

    if (currentNode + 1 >= static_cast<int>(this->nodesOut.size())) {
      continue;
    }

    for (int edgeIndex = this->nodesOut[currentNode];
         edgeIndex < this->nodesOut[currentNode + 1];
         ++edgeIndex) {
      const auto& link = this->linksOut[edgeIndex];
      int linkId = link->getId();
      if (excludedLinks.count(linkId)) {
        continue;
      }
      int neighbor = link->getDst();
      if (excludedNodes.count(neighbor) && neighbor != dst) {
        continue;
      }
      double newDistance = currentDistance + link->getLength();
      if (newDistance < distances[neighbor]) {
        distances[neighbor] = newDistance;
        previousNode[neighbor] = currentNode;
        previousLink[neighbor] = linkId;
        pq.push({newDistance, neighbor});
      }
    }
  }

  if (distances[dst] == std::numeric_limits<double>::infinity()) {
    return result;
  }

  std::vector<int> nodePath;
  std::vector<int> linkPath;
  for (int current = dst; current != -1; current = previousNode[current]) {
    nodePath.push_back(current);
    if (previousLink[current] != -1) {
      linkPath.push_back(previousLink[current]);
    }
  }
  std::reverse(nodePath.begin(), nodePath.end());
  std::reverse(linkPath.begin(), linkPath.end());

  result.nodePath = std::move(nodePath);
  result.linkPath = std::move(linkPath);
  result.totalLength = distances[dst];
  return result;
}

std::vector<Network::ShortestPathResult> Network::yenKShortestPaths(int src, int dst, int k) const {
  std::vector<ShortestPathResult> kPaths;
  if (k <= 0 || src == dst) {
    return kPaths;
  }

  auto firstPath = this->dijkstra(src, dst);
  if (firstPath.empty()) {
    return kPaths;
  }

  kPaths.push_back(firstPath);

  using Candidate = std::pair<double, ShortestPathResult>;
  struct CandidateCompare {
    bool operator()(const Candidate& lhs, const Candidate& rhs) const {
      return lhs.first > rhs.first;
    }
  };
  std::priority_queue<Candidate, std::vector<Candidate>, CandidateCompare> candidates;
  std::unordered_set<std::vector<int>, VecHash> bestPathSet;
  std::unordered_set<std::vector<int>, VecHash> candidateSet;

  bestPathSet.insert(firstPath.linkPath);

  for (int pathCount = 1; pathCount < k; ++pathCount) {
    const auto& previousPath = kPaths[pathCount - 1];

    for (size_t i = 0; i < previousPath.nodePath.size() - 1; ++i) {
      int spurNode = previousPath.nodePath[i];
      std::vector<int> rootNodes(previousPath.nodePath.begin(), previousPath.nodePath.begin() + i + 1);
      std::vector<int> rootLinks(previousPath.linkPath.begin(), previousPath.linkPath.begin() + i);

      std::unordered_set<int> removedLinks;
      for (const auto& path : kPaths) {
        if (path.nodePath.size() > i &&
            std::equal(rootNodes.begin(), rootNodes.end(), path.nodePath.begin())) {
          if (i < path.linkPath.size()) {
            removedLinks.insert(path.linkPath[i]);
          }
        }
      }

      std::unordered_set<int> excludedNodes;
      for (size_t j = 0; j + 1 < rootNodes.size(); ++j) {
        excludedNodes.insert(rootNodes[j]);
      }

      auto spurPath = this->dijkstra(spurNode, dst, removedLinks, excludedNodes);
      if (spurPath.empty()) {
        continue;
      }

      ShortestPathResult totalPath;
      totalPath.nodePath = rootNodes;
      totalPath.nodePath.insert(totalPath.nodePath.end(),
                                spurPath.nodePath.begin() + 1,
                                spurPath.nodePath.end());

      totalPath.linkPath = rootLinks;
      totalPath.linkPath.insert(totalPath.linkPath.end(),
                                spurPath.linkPath.begin(),
                                spurPath.linkPath.end());

      if (bestPathSet.count(totalPath.linkPath) || candidateSet.count(totalPath.linkPath)) {
        continue;
      }

      double rootLength = 0.0;
      for (int linkId : rootLinks) {
        rootLength += this->links.at(linkId)->getLength();
      }
      totalPath.totalLength = rootLength + spurPath.totalLength;

      candidates.push({totalPath.totalLength, totalPath});
      candidateSet.insert(totalPath.linkPath);
    }

    if (candidates.empty()) {
      break;
    }

    auto bestCandidate = candidates.top();
    candidates.pop();
    candidateSet.erase(bestCandidate.second.linkPath);
    bestPathSet.insert(bestCandidate.second.linkPath);
    kPaths.push_back(std::move(bestCandidate.second));
  }

  return kPaths;
}

std::shared_ptr<Fiber> Network::readSingleFiber(const nlohmann::json& fiberData) {
  const auto& slotsData = fiberData["slots"];

  // Create fiber based on slots configuration
  std::shared_ptr<Fiber> fiber;

  if (slotsData.is_number()) {
    // SSMF: "slots": 320
    fiber = std::make_shared<Fiber>(slotsData.get<int>());
  } else if (slotsData.is_array()) {
    if (slotsData[0].is_number()) {
      // MCF: [80, 90, 70] - transform to C: [[80], [90], [70]]
      std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix;
      for (int slotCount : slotsData.get<std::vector<int>>()) {
        bandSlotMatrix[fns::Band::C].push_back({slotCount});
      }
      fiber = std::make_shared<Fiber>(bandSlotMatrix);
    } else if (slotsData[0].is_array()) {
      // FMMCF: [[80, 60], [90, 70]] - transform to C: [[80, 60], [90, 70]]
      std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix;
      bandSlotMatrix[fns::Band::C] = slotsData.get<std::vector<std::vector<int>>>();
      fiber = std::make_shared<Fiber>(bandSlotMatrix);
    }
  } else if (slotsData.is_object()) {
    if (slotsData.begin().value().is_number()) {
      // Multi-Band SSMF: {"C":344, "L":480} - transform to C: [[344]], L: [[480]]
      std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix;
      for (const auto& [bandStr, slots] : slotsData.items()) {
        bandSlotMatrix[fns::charToBand(bandStr[0])].push_back({slots});
      }
      fiber = std::make_shared<Fiber>(bandSlotMatrix);
    } else if (slotsData.begin().value().is_array()) {
      const auto& firstBandArray = slotsData.begin().value();
      if (firstBandArray[0].is_number()) {
        // Multi-Band MCF: {"C": [80, 90], "L": [60, 70]} - transform to C: [[80], [90]], L: [[60], [70]]
        std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix;
        for (const auto& [bandStr, coreSlots] : slotsData.items()) {
          fns::Band band = fns::charToBand(bandStr[0]);
          for (int slotCount : coreSlots.get<std::vector<int>>()) {
            bandSlotMatrix[band].push_back({slotCount});
          }
        }
        fiber = std::make_shared<Fiber>(bandSlotMatrix);
      } else if (firstBandArray[0].is_array()) {
        // Multi-Band FMMCF: {"C": [[100, 80], [90, 70]], "L": [[80, 60], [70, 50]]}
        std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix;
        for (const auto& [bandStr, coreMatrix] : slotsData.items()) {
          fns::Band band = fns::charToBand(bandStr[0]);
          bandSlotMatrix[band] = coreMatrix.get<std::vector<std::vector<int>>>();
        }
        fiber = std::make_shared<Fiber>(bandSlotMatrix);
      }
    }
  } else {
    throw std::runtime_error("Unknown fiber configuration in JSON");
  }
  // Override auto-detected type if explicit type is provided
  if (fiberData.contains("type") && fiberData["type"].is_string()) {
    std::string typeStr = fiberData["type"].get<std::string>();
    if (typeStr == "SSMF") {
      fiber->setType(fns::FiberType::SSMF);
    } else if (typeStr == "FMF") {
      fiber->setType(fns::FiberType::FMF);
    } else if (typeStr == "MCF") {
      fiber->setType(fns::FiberType::MCF);
    } else if (typeStr == "FMMCF") {
      fiber->setType(fns::FiberType::FMMCF);
    } else if (typeStr == "HCF") {
      fiber->setType(fns::FiberType::HCF);
    } else {
      throw std::runtime_error("Unknown fiber type: " + typeStr);
    }
  }
  return fiber;
}

std::vector<std::shared_ptr<Fiber>> Network::readMultiFiber(const nlohmann::json& linkData) {
    std::vector<std::shared_ptr<Fiber>> fibers;

    // linkData is an array of fiber configurations
    for (const auto& fiberData : linkData) {
        auto fiber = readSingleFiber(fiberData);
        fibers.push_back(fiber);
    }
    
    return fibers;
}

void Network::networkToJson() const {
  nlohmann::ordered_json output;
  
  // Add network name
  output["name"] = this->name;
  
  // Export nodes
  output["nodes"] = nlohmann::json::array();
  for (const auto& nodePtr : this->nodes) {
    nlohmann::ordered_json nodeJson;
    // Fields exported in order: id, label, DC, IXP, pop, param1, param2, longitude, latitude
    nodeJson["id"] = nodePtr->getId();
    
    // Add optional fields only if they have values
    // Use try-catch since getters throw NodeAttributeNotSetException if not set
    try {
      auto label = nodePtr->getLabel();
      if (label.has_value()) {
        nodeJson["label"] = label.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    try {
      auto dc = nodePtr->getDCs();
      if (dc.has_value()) {
        nodeJson["DC"] = dc.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    try {
      auto ixp = nodePtr->getIXPs();
      if (ixp.has_value()) {
        nodeJson["IXP"] = ixp.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    try {
      auto pop = nodePtr->getPopulation();
      if (pop.has_value()) {
        nodeJson["pop"] = pop.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    try {
      auto p1 = nodePtr->getParam1();
      if (p1.has_value()) {
        nodeJson["param1"] = p1.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    try {
      auto p2 = nodePtr->getParam2();
      if (p2.has_value()) {
        nodeJson["param2"] = p2.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    try {
      auto lon = nodePtr->getLongitude();
      if (lon.has_value()) {
        nodeJson["longitude"] = lon.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    try {
      auto lat = nodePtr->getLatitude();
      if (lat.has_value()) {
        nodeJson["latitude"] = lat.value();
      }
    } catch (const NodeAttributeNotSetException&) {
      // Field not set, skip it
    }
    
    output["nodes"].push_back(nodeJson);
  }
  
  // Export links
  output["links"] = nlohmann::json::array();
  for (const auto& linkPtr : this->links) {
    nlohmann::ordered_json linkJson;
    linkJson["id"] = linkPtr->getId();
    linkJson["src"] = linkPtr->getSrc();
    linkJson["dst"] = linkPtr->getDst();
    linkJson["length"] = linkPtr->getLength();
    
    // Handle fiber configuration
    const auto& fibers = linkPtr->getFibers();
    if (fibers.size() == 1) {
      // Single fiber - export at link level
      const auto& fiber = fibers[0];
      const auto& bands = fiber->getBands();
      
      // Simple case: single band (C)
      if (bands.size() == 1 && bands[0] == fns::Band::C) {
        int numCores = fiber->getNumberOfCores();
        
        if (numCores == 1) {
          int numModes = fiber->getNumberOfModes(0, fns::Band::C);
          if (numModes == 1) {
            // SSMF: single core, single mode
            linkJson["slots"] = fiber->getNumberOfSlots(0, fns::Band::C, 0);
          } else {
            // FMF: single core, multiple modes
            std::vector<int> modeSlots;
            for (int mode = 0; mode < numModes; ++mode) {
              modeSlots.push_back(fiber->getNumberOfSlots(0, fns::Band::C, mode));
            }
            linkJson["slots"] = nlohmann::json::array({modeSlots});
          }
        } else {
          // MCF or FMMCF: multiple cores
          std::vector<std::vector<int>> coreMatrix;
          bool isMCF = true;
          for (int core = 0; core < numCores; ++core) {
            int numModes = fiber->getNumberOfModes(core, fns::Band::C);
            if (numModes > 1) {
              isMCF = false;
            }
            std::vector<int> modeSlots;
            for (int mode = 0; mode < numModes; ++mode) {
              modeSlots.push_back(fiber->getNumberOfSlots(core, fns::Band::C, mode));
            }
            coreMatrix.push_back(modeSlots);
          }
          
          if (isMCF) {
            // MCF: extract just first mode from each core
            std::vector<int> coreSlots;
            for (const auto& modes : coreMatrix) {
              coreSlots.push_back(modes[0]);
            }
            linkJson["slots"] = coreSlots;
          } else {
            // FMMCF: full matrix
            linkJson["slots"] = coreMatrix;
          }
        }
      } else {
        // Multi-band fiber
        nlohmann::json bandsJson;
        int numCores = fiber->getNumberOfCores();
        
        for (const auto& band : bands) {
          std::string bandStr(1, fns::bandToChar(band));
          
          if (numCores == 1) {
            int numModes = fiber->getNumberOfModes(0, band);
            if (numModes == 1) {
              // Multi-band SSMF
              bandsJson[bandStr] = fiber->getNumberOfSlots(0, band, 0);
            } else {
              // Multi-band FMF
              std::vector<int> modeSlots;
              for (int mode = 0; mode < numModes; ++mode) {
                modeSlots.push_back(fiber->getNumberOfSlots(0, band, mode));
              }
              bandsJson[bandStr] = nlohmann::json::array({modeSlots});
            }
          } else {
            // Multi-band MCF or FMMCF
            std::vector<std::vector<int>> coreMatrix;
            bool isMCF = true;
            for (int core = 0; core < numCores; ++core) {
              int numModes = fiber->getNumberOfModes(core, band);
              if (numModes > 1) {
                isMCF = false;
              }
              std::vector<int> modeSlots;
              for (int mode = 0; mode < numModes; ++mode) {
                modeSlots.push_back(fiber->getNumberOfSlots(core, band, mode));
              }
              coreMatrix.push_back(modeSlots);
            }
            
            if (isMCF) {
              // Multi-band MCF
              std::vector<int> coreSlots;
              for (const auto& modes : coreMatrix) {
                coreSlots.push_back(modes[0]);
              }
              bandsJson[bandStr] = coreSlots;
            } else {
              // Multi-band FMMCF
              bandsJson[bandStr] = coreMatrix;
            }
          }
        }
        linkJson["slots"] = bandsJson;
      }
      
      // Add type if not SSMF (default)
      if (fiber->getType() != fns::FiberType::SSMF) {
        linkJson["type"] = fns::fiberTypeToString(fiber->getType());
      }
    } else {
      // Multiple fibers
      linkJson["fibers"] = nlohmann::json::array();
      for (const auto& fiber : fibers) {
        nlohmann::ordered_json fiberJson;
        const auto& bands = fiber->getBands();
        
        // Same logic as single fiber for slots
        if (bands.size() == 1 && bands[0] == fns::Band::C) {
          int numCores = fiber->getNumberOfCores();
          
          if (numCores == 1) {
            int numModes = fiber->getNumberOfModes(0, fns::Band::C);
            if (numModes == 1) {
              fiberJson["slots"] = fiber->getNumberOfSlots(0, fns::Band::C, 0);
            } else {
              std::vector<int> modeSlots;
              for (int mode = 0; mode < numModes; ++mode) {
                modeSlots.push_back(fiber->getNumberOfSlots(0, fns::Band::C, mode));
              }
              fiberJson["slots"] = nlohmann::json::array({modeSlots});
            }
          } else {
            std::vector<std::vector<int>> coreMatrix;
            bool isMCF = true;
            for (int core = 0; core < numCores; ++core) {
              int numModes = fiber->getNumberOfModes(core, fns::Band::C);
              if (numModes > 1) {
                isMCF = false;
              }
              std::vector<int> modeSlots;
              for (int mode = 0; mode < numModes; ++mode) {
                modeSlots.push_back(fiber->getNumberOfSlots(core, fns::Band::C, mode));
              }
              coreMatrix.push_back(modeSlots);
            }
            
            if (isMCF) {
              std::vector<int> coreSlots;
              for (const auto& modes : coreMatrix) {
                coreSlots.push_back(modes[0]);
              }
              fiberJson["slots"] = coreSlots;
            } else {
              fiberJson["slots"] = coreMatrix;
            }
          }
        } else {
          nlohmann::json bandsJson;
          int numCores = fiber->getNumberOfCores();
          
          for (const auto& band : bands) {
            std::string bandStr(1, fns::bandToChar(band));
            
            if (numCores == 1) {
              int numModes = fiber->getNumberOfModes(0, band);
              if (numModes == 1) {
                bandsJson[bandStr] = fiber->getNumberOfSlots(0, band, 0);
              } else {
                std::vector<int> modeSlots;
                for (int mode = 0; mode < numModes; ++mode) {
                  modeSlots.push_back(fiber->getNumberOfSlots(0, band, mode));
                }
                bandsJson[bandStr] = nlohmann::json::array({modeSlots});
              }
            } else {
              std::vector<std::vector<int>> coreMatrix;
              bool isMCF = true;
              for (int core = 0; core < numCores; ++core) {
                int numModes = fiber->getNumberOfModes(core, band);
                if (numModes > 1) {
                  isMCF = false;
                }
                std::vector<int> modeSlots;
                for (int mode = 0; mode < numModes; ++mode) {
                  modeSlots.push_back(fiber->getNumberOfSlots(core, band, mode));
                }
                coreMatrix.push_back(modeSlots);
              }
              
              if (isMCF) {
                std::vector<int> coreSlots;
                for (const auto& modes : coreMatrix) {
                  coreSlots.push_back(modes[0]);
                }
                bandsJson[bandStr] = coreSlots;
              } else {
                bandsJson[bandStr] = coreMatrix;
              }
            }
          }
          fiberJson["slots"] = bandsJson;
        }
        
        if (fiber->getType() != fns::FiberType::SSMF) {
          fiberJson["type"] = fns::fiberTypeToString(fiber->getType());
        }
        
        linkJson["fibers"].push_back(fiberJson);
      }
    }
    
    output["links"].push_back(linkJson);
  }
  
  // Write to file
  std::ofstream file("network_export.json");
  if (!file.is_open()) {
    throw std::runtime_error("Could not create file: network_export.json");
  }
  file << output.dump(4);  // Pretty print with 4-space indent
  file.close();
}

void Network::routesToJson() const {
  if (!this->paths || this->paths->empty()) {
    throw std::runtime_error("No paths have been computed yet. Call setPaths() before exporting routes.");
  }
  
  nlohmann::ordered_json output;
  output["routes"] = nlohmann::json::array();
  
  const int numberOfNodes = this->getNumberOfNodes();
  
  for (int src = 0; src < numberOfNodes; ++src) {
    for (int dst = 0; dst < numberOfNodes; ++dst) {
      if (src == dst) continue;
      
      const auto& routesForPair = (*this->paths)[src][dst];
      if (routesForPair.empty()) continue;
      
      nlohmann::ordered_json routeEntry;
      routeEntry["src"] = src;
      routeEntry["dst"] = dst;
      routeEntry["paths"] = nlohmann::json::array();
      
      for (const auto& route : routesForPair) {
        std::vector<int> pathIds;
        for (const auto& link : route) {
          pathIds.push_back(link->getId());
        }
        routeEntry["paths"].push_back(pathIds);
      }
      
      output["routes"].push_back(routeEntry);
    }
  }
  
  // Write to file
  std::ofstream file("routes_export.json");
  if (!file.is_open()) {
    throw std::runtime_error("Could not create file: routes_export.json");
  }
  file << output.dump(4);
  file.close();
}

