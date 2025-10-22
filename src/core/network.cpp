#include "network.hpp"

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
}
// Connects two Nodes through one Link (order is important: src != dst):
//
//       (Source Node) ---Link---> (Destination Node)

int Network::isConnected(int src, int dst) const {
  for (int i = this->nodesOut[src]; i < this->nodesOut[src + 1]; i++) {
    for (int j = nodesIn[dst]; j < nodesIn[dst + 1]; j++) {
      if (linksOut[i]->getId() == linksIn[j]->getId()) {
        return linksOut[i]->getId();
      }
    }
  }
  return -1;
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