#include "simulator.hpp"

Simulator::Simulator(void) {
  this->defaultValues();
  this->controller = std::make_unique<Controller>();
  this->events = std::list<Event>();

  // Bit Rates - Using band-specific modulation format maps
  this->bitRates = std::vector<std::shared_ptr<BitRate>>();
  std::shared_ptr<BitRate> auxB = std::make_shared<BitRate>(10.0);
  auxB->addModulation("BPSK", {{fns::Band::C, 1}}, {{fns::Band::C, 5520}});
  this->bitRates.push_back(auxB);
  auxB = std::make_shared<BitRate>(40.0);
  auxB->addModulation("BPSK", {{fns::Band::C, 4}}, {{fns::Band::C, 5520}});
  this->bitRates.push_back(auxB);
  auxB = std::make_shared<BitRate>(100.0);
  auxB->addModulation("BPSK", {{fns::Band::C, 8}}, {{fns::Band::C, 5520}});
  this->bitRates.push_back(auxB);
  auxB = std::make_shared<BitRate>(400.0);
  auxB->addModulation("BPSK", {{fns::Band::C, 32}}, {{fns::Band::C, 5520}});
  this->bitRates.push_back(auxB);
  auxB = std::make_shared<BitRate>(1000.0);
  auxB->addModulation("BPSK", {{fns::Band::C, 80}}, {{fns::Band::C, 5520}});
  this->bitRates.push_back(auxB);
}

Simulator::Simulator(std::string networkFilename, std::string pathsFilename)
    : Simulator() {
  this->controller->setNetwork(std::make_shared<Network>(networkFilename));
  this->controller->setPaths(pathsFilename);
}

Simulator::Simulator(std::string networkFilename, std::string pathsFilename,
                     std::string bitratesFilename) {
  this->defaultValues();
  this->controller = std::make_unique<Controller>();
  this->controller->setNetwork(std::make_shared<Network>(networkFilename));
  this->controller->setPaths(pathsFilename);
  this->events = std::list<Event>();
  this->bitRates = BitRate::readBitRatesFile(bitratesFilename);
}

Simulator::Simulator(std::string networkFilename, std::string pathsFilename,
                     std::string bitratesFilename, std::string demandsFilename)
    : Simulator(networkFilename, pathsFilename, bitratesFilename) {
  this->readDemandsFile(demandsFilename);
}

Simulator::Simulator(std::string networkFilename)
    : Simulator() {
  this->controller->setNetwork(std::make_shared<Network>(networkFilename));
  this->controller->setPaths(); // Uses default k=3
}

Simulator::Simulator(std::string networkFilename, int k)
    : Simulator() {
  this->controller->setNetwork(std::make_shared<Network>(networkFilename));
  this->controller->setPaths(k);
}

Simulator::Simulator(std::string networkFilename,
                     std::string bitratesFilename, int k) {
  this->defaultValues();
  this->controller = std::make_unique<Controller>();
  this->controller->setNetwork(std::make_shared<Network>(networkFilename));
  this->controller->setPaths(k);
  this->events = std::list<Event>();
  this->bitRates = BitRate::readBitRatesFile(bitratesFilename);
}

Simulator::Simulator(std::string networkFilename, std::string bitratesFilename,
                     std::string demandsFilename, int k)
    : Simulator(networkFilename, bitratesFilename, k) {
  this->readDemandsFile(demandsFilename);
}

void Simulator::setAllocator(std::unique_ptr<Allocator> newAllocator) {
  if (this->initReady) {
    throw std::runtime_error(
        "You can not set allocator parameter AFTER calling init simulator "
        "method.");
  }
  newAllocator->setNetwork(this->controller->getNetwork());
  // Note: Paths are managed by Network, accessed via network->getPaths()
  this->controller->setAllocator(std::move(newAllocator));
}

void Simulator::setCallbackFunction(
  void (*callbackFunction)(
      Network& network,
      std::vector<std::vector<Demand>>& demands,
      std::vector<std::unique_ptr<Connection>>& connections,
      double time)) {
      
  if (this->initReady) {
    throw std::runtime_error(
        "You can not set callback function AFTER calling init simulator "
        "method.");
  }
  this->controller->setCallbackFunction(callbackFunction);
}

void Simulator::setNumberOfPeriods(int numberOfPeriods) {
  if (this->initReady) {
    throw std::runtime_error(
        "You can not set 'numberOfPeriods' parameter AFTER calling init "
        "simulator method.");
  }
  if (numberOfPeriods <= 0) {
    throw std::invalid_argument(
        "You can not set 'numberOfPeriods' parameter to a value lower than "
        "1.");
  }
  this->numberOfPeriods = numberOfPeriods;
}

void Simulator::setBaseGrowthRate(double baseGrowthRate) {
  if (this->initReady) {
    throw std::runtime_error(
        "You can not set 'baseGrowthRate' parameter AFTER calling init "
        "simulator method.");
  }
  if (baseGrowthRate < 0) {
    throw std::invalid_argument(
        "You can not set 'baseGrowthRate' parameter to a value lower than "
        "0.");
  }
  this->baseGrowthRate = baseGrowthRate;
}
void Simulator::setGrowthRateStdDev(double growthRateStdDev) {
  if (this->initReady) {
    throw std::runtime_error(
        "You can not set 'growthRateStdDev' parameter AFTER calling init "
        "simulator method.");
  }
  if (growthRateStdDev < 0) {
    throw std::invalid_argument(
        "You can not set 'growthRateStdDev' parameter to a value lower than "
        "0.");
  }
  this->growthRateStdDev = growthRateStdDev;
}
void Simulator::setSeedGrowthRate(unsigned int seedGrowthRate) {
  if (this->initReady) {
    throw std::runtime_error(
        "You can not set 'seedGrowthRate' parameter AFTER calling init "
        "simulator method.");
  }
  this->seedGrowthRate = seedGrowthRate;
}
void Simulator::setGrowthRates(std::vector<double> growthRates) {
  if (this->initReady) {
    throw std::runtime_error(
        "You can not set growth rates AFTER calling init simulator method.");
  }
  if (growthRates.size() != static_cast<size_t>(this->numberOfPeriods)) {
    throw std::invalid_argument(
        "The size of the growth rates vector must be equal to the number of "
        "periods.");
  }
  this->growthRates = std::move(growthRates);
}


double Simulator::getBaseGrowthRate() const {
  return this->baseGrowthRate;
}
double Simulator::getGrowthRateStdDev() const {
  return this->growthRateStdDev;
}
std::vector<double> Simulator::getGrowthRates() const {
  return this->growthRates;
}
int Simulator::getNumberOfPeriods() const {
  return this->numberOfPeriods;
}
int Simulator::getCurrentPeriod() const {
  return this->currentPeriod;
}
std::vector<std::shared_ptr<BitRate>> Simulator::getBitRates(void) {
    return this->bitRates;
}
Demand* Simulator::getDemand(int src, int dst) {
    if (this->demands[src][dst].isNull()) {
        return nullptr;
    }
    return &this->demands[src][dst];
}
Demand* Simulator::getDemand(int id) {
    if (id < 0) return nullptr;
    for (auto& row : this->demands) {
        for (auto& demand : row) {
            if (!demand.isNull() && demand.getId() == id) {
                return &demand;
            }
        }
    }
    return nullptr;
}
Paths* Simulator::getPaths() {
  return this->controller->getPaths();
}
std::unique_ptr<Controller>& Simulator::getController() {
  return this->controller;
}
std::vector<std::vector<Demand>>& Simulator::getDemands() {
  return this->demands;
}
unsigned int Simulator::getTimeDuration(void) {
  return static_cast<unsigned int>(this->timeDuration.count());
}

void Simulator::readDemandsFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file) throw std::runtime_error("Could not open demands file: " + filename);

  nlohmann::json updates;
  try {
      file >> updates;
  } catch (const nlohmann::json::exception& e) {
      throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
  }
  if (!updates.is_array()) {
      throw std::runtime_error("JSON must be an array of demand updates");
  }

  int totalNodes = this->controller->getNetwork()->getNumberOfNodes();

  // Build an NxN grid of Demands.
  // Diagonal entries remain default-constructed (id/src/dst = -1)
  this->demands.resize(totalNodes);
  for (int i = 0; i < totalNodes; ++i) {
      this->demands[i].resize(totalNodes);
  }

  int nextId = 0;
  for (int i = 0; i < totalNodes; ++i) {
      for (int j = 0; j < totalNodes; ++j) {
          if (i == j) continue; // leave default-constructed
          this->demands[i][j] = Demand(nextId++, i, j, 0.0);
      }
  }

  // Apply updates
  for (const auto& u : updates) {
      try {
          // Helper lambda to parse node field (supports both string labels and integer IDs)
          auto parseNodeField = [&](const std::string& primaryKey, const std::string& altKey) -> int {
              nlohmann::json nodeField;
              if (u.contains(primaryKey)) {
                  nodeField = u[primaryKey];
              } else if (u.contains(altKey)) {
                  nodeField = u[altKey];
              } else {
                  throw std::invalid_argument("Missing '" + primaryKey + "'/" + altKey + "'");
              }
              
              if (nodeField.is_string()) {
                  std::string label = nodeField.get<std::string>();
                  // Search for node with this label in the network
                  for (int nodeId = 0; nodeId < totalNodes; nodeId++) {
                      auto nodeLabelOpt = this->controller->getNetwork()->getNode(nodeId)->getLabel();
                      if (nodeLabelOpt.has_value() && nodeLabelOpt.value() == label) {
                          return nodeId;
                      }
                  }
                  throw std::invalid_argument("Unknown node label: " + label);
              } else if (nodeField.is_number_integer()) {
                  return nodeField.get<int>();
              } else {
                  throw std::invalid_argument("Node field must be string (label) or integer (ID)");
              }
          };

          int src = parseNodeField("src", "source");
          int dst = parseNodeField("dst", "destination");

          if (src == dst) throw std::invalid_argument("src == dst");
          if (src < 0 || dst < 0 || src >= totalNodes || dst >= totalNodes)
              throw std::out_of_range("src/dst out of range");

          if (!u.contains("required"))
              throw std::invalid_argument("Missing 'required'");
          double req = u.at("required").get<double>();
          if (req < 0) throw std::invalid_argument("'required' < 0");

          this->demands[src][dst].setRequiredCapacity(req);
      } catch (const std::exception& e) {
          std::cerr << "Skipping update: " << e.what() << "\n";
      }
  }
}


void Simulator::defaultValues(void) {
  this->initReady = false;
  this->seedGrowthRate = 12345;
  this->baseGrowthRate = 0.35;
  this->growthRateStdDev = 0.1;
  this->currentPeriod = 0;
  this->numberOfPeriods = 3;
}
void Simulator::init(void) {
  this->initReady = true;
  this->clock = 0;
  if (this->demands.empty()) initializeDemands();
  this->growthVariable = NormalVariable(this->seedGrowthRate,
                          this->baseGrowthRate,
                          this->growthRateStdDev);
  this->events.push_back(Event(PERIOD_UPDATE, this->clock));
}
void Simulator::initializeDemands(void) {

  // Calculate network characteristics
  int totalNodes = controller->getNetwork()->getNumberOfNodes();
  int totalLinks = controller->getNetwork()->getNumberOfLinks();
  this->demands.resize(totalNodes);
  for (int i = 0; i < totalNodes; ++i) {
    this->demands[i].resize(totalNodes);
  }
  
  // N_bar: The average node degree of the topology
  double avgNodeDegree = static_cast<double>(totalLinks) / totalNodes;

  try {
    // Generate demand matrix based on network topology
    // Node degrees are updated alongside topology changes (via Network::connect/setPaths).
    int demandId = 0;
    for (int i = 0; i < totalNodes; i++) {
      for (int j = 0; j < totalNodes; j++) {
          if (i == j) {
            continue;
          }
          
          Node* srcNode = controller->getNetwork()->getNode(i).get();
          Node* dstNode = controller->getNetwork()->getNode(j).get();
          
          // N: Combined node degree (N_i + N_j)
          double combinedNodeDegree = srcNode->getDegree() + dstNode->getDegree();

          // Delta_k: Absolute difference between DCs and IXPs at node k
          // This will throw NodeAttributeNotSetException if DCs or IXPs are not set
          double deltaI = std::abs(srcNode->getDCs().value() - srcNode->getIXPs().value());
          double deltaJ = std::abs(dstNode->getDCs().value() - dstNode->getIXPs().value());
          
          // Apply Patri et al. formula for initial traffic demand (in Gbps)
          double initialTrafficGbps = 0.0;

          // Condition: if N > 2 * N_bar
          if (combinedNodeDegree > 2 * avgNodeDegree) {
            // High-connectivity pairs: N * (N - 1) * Delta_i * Delta_j
            initialTrafficGbps = combinedNodeDegree * (combinedNodeDegree - 1) * deltaI * deltaJ; 
          } else {
            // Standard connectivity: N * Delta_i * Delta_j
            initialTrafficGbps = combinedNodeDegree * deltaI * deltaJ;
          }
          
          demands[i][j] = Demand(demandId++, i, j, initialTrafficGbps);
      }
    }
  }
  catch (const NodeAttributeNotSetException& e) {
    // Specific catch for missing DCs/IXPs
    std::cerr << "\nWarning: " << e.what() << "\n";
    std::cerr << "Please provide DCs and IXPs values for each node in the network file,\n";
    std::cerr << "or provide a demands file to skip automatic demand generation.\n";
    std::cerr << "Initializing with random SYMMETRIC demand values instead...\n\n";
    
    UniformVariable uniformDemand(505, 50.0, 500.0); // Random demands between 50-500 Gbps
    
    int demandId = 0;
    for (int i = 0; i < totalNodes; i++) {
      for (int j = 0; j < i; j++) {  // Only iterate lower triangle (j < i)
        // Generate one random value for both directions
        double randomTraffic = uniformDemand.getNextValue();
        
        // Assign same value to both directions (symmetric)
        demands[i][j] = Demand(demandId++, i, j, randomTraffic);
        demands[j][i] = Demand(demandId++, j, i, randomTraffic);
      }
    }
  }
  catch (const std::exception& e) {
    // Catch any other unexpected errors
    throw std::runtime_error("Error initializing demands: " + std::string(e.what()));
  }
}
void Simulator::run(void) {
    run(false);
}
void Simulator::run(bool highVerbose) {
    printInitialInfo();
    
    while (!this->events.empty() && this->currentPeriod < this->numberOfPeriods) {
        eventRoutine();
        printRow(highVerbose);
    }
    printFinalInfo();
}
void Simulator::eventRoutine() {
  this->currentEvent = this->events.front();
  this->events.pop_front();
  this->clock = this->currentEvent.getPeriod();

  if (this->currentEvent.getType() == PERIOD_UPDATE) {
    // 1. Update demands based on growth model
    if (this->currentPeriod != 0) {
      updateDemands();
    }
    // 2. Increment period
    this->currentPeriod++;
    // 3. Provision network based on current demands
    this->controller->assignConnections(this->demands, this->bitRates,
                                        static_cast<double>(this->currentPeriod));
    // 4. Schedule next provisioning event
    if (this->currentPeriod < this->numberOfPeriods) {
        this->events.push_back(Event(PERIOD_UPDATE, this->clock + 1.0));
    }

    // TODO:
    // 5. Schedule future failure events if failure management is set
    if (this->controller->failureManagementFunction) {
      // Depending on the failure rate, schedule failure events of various types
      // LINK_FAILURE
      // NODE_FAILURE
      // FIBER_FAILURE
      /**
       * Each failure event has its own failure rate (failures per time unit),
       * from 0 to 1.0 (so if a failure happens depends on a binomial distribution?).
       * Every time is more likely that a failure happens, but it is not guaranteed.
       * The failure event also has a duration, which can be fixed.
       * 
       * Time to recover is set by a random variable, e.g., exponential distribution
       * with a given mean time to repair (MTTR). This time is in time units.
       * 
       * Also is a type event called:
       * LINK_RECOVERY
       * NODE_RECOVERY
       * FIBER_RECOVERY
       * 
       * The failureManagementFunction is called every time a failure event happens,
       * the type of failure is given as parameter. This event:
       * contains if node, node id, if link link id, if fiber, link+fiber id.
       * 
       * Also a connectionsAffected vector is given, with all connections affected
       * by the failure. The user can do whatever he wants with this information.
       * 
       * Either reallocate them, drop them, log them, etc.
       * 
       * Inside this code block we will decide if schedule a failure event or not,
       * based on the failure rates. and aslo which one. How to do it...dont know yet.
       */
  }
}
  else {
    /**
     * Here we fill the connectionsAffected vector, based on the type of failure.
     * 
     * Also we actually make fail the Link, Node or Fiber, changing the network state.
     * (we have to add a new method that puts in the "failure state" a link, node or fiber)
     * and check every time we want to allocate a connection if the link/node/fiber is in failure state.
     * That is really tedious to do, but we have to do it.
     * 
     * We will have to also decide how to implement a protection path in the connection class.
     *
     * The use of the protection path will be labeled in the slots by the connection id but in negative.
     */
    std::vector<std::unique_ptr<Connection>> connectionsAffected;
    
    switch (this->currentEvent.getType())
    {
    case LINK_FAILURE:
      /* code */
      break;
    case NODE_FAILURE:
      /* code */
      break;
    case FIBER_FAILURE:
      /* code */
      break;
    case LINK_RECOVERY:
      /* code */
      break;
    case NODE_RECOVERY:
      /* code */
      break;
    case FIBER_RECOVERY:
      /* code */
      break;
    default:
      break;
    }
    // Despite of the failure type, we call the failure management function
    // with the affected connections.
    this->controller->failureManagementFunction(
        *(this->controller->getNetwork()),
        this->demands,
        connectionsAffected,
        this->currentEvent.getType(),
        static_cast<double>(this->currentPeriod));
  }
}
void Simulator::updateDemands() {
  // If no manual growth rates are defined, use the variable growth model
  if (this->growthRates.empty()) {
    double currentGrowthRate = this->growthVariable.getNextValue();
    for (auto& row : this->demands) {
        for (auto& demand : row) {
            if (!demand.isNull()) {
                demand.setRequiredCapacity(demand.getRequiredCapacity() * (currentGrowthRate + 1.0));
            }
        }
    }
  }
  // If manual growth rates are defined, use them
  else {
    for (auto& row : this->demands) {
        for (auto& demand : row) {
            if (!demand.isNull()) {
                demand.setRequiredCapacity(demand.getRequiredCapacity() * (this->growthRates[this->currentPeriod] + 1.0));
            }
        }
    }
  }
}





void Simulator::printInitialInfo() {
  std::cout << "\n--- Flex Net Sim (" << VERSION_MAJOR << "." << VERSION_MINOR
            << "." << VERSION_REVISION << ") ---"
            << "\n\n";
  std::cout << std::setfill(' ') << std::setw(20) << std::left << "Network:";
  std::cout << std::setw(30)
            << this->controller->getNetwork()->getName() << "\n";
  std::cout << std::setfill(' ') << std::setw(20) << std::left << "Nodes:";
  std::cout << std::setw(30)
            << this->controller->getNetwork()->getNumberOfNodes() << "\n";
  std::cout << std::setw(20) << "Links:";
  std::cout << std::setw(30)
            << this->controller->getNetwork()->getNumberOfLinks() << "\n";
  std::cout << std::setw(20) << "Periods:";
  std::cout << std::setw(30) << this->numberOfPeriods << "\n";
  if (!this->growthRates.empty()) {
    std::cout << std::setw(20) << "Growth Rate:";
    std::cout << std::setw(30) << "USER DEFINED" << "\n";
  }
  else {
    std::cout << std::setw(20) << "Growth Rate:";
    std::cout << std::setw(30) << (this->baseGrowthRate) << "\n";
    std::cout << std::setw(20) << "Std deviation:";
    std::cout << std::setw(30) << this->growthRateStdDev << "\n";
  }
  std::cout << std::setw(20) << "Algorithm:";
  std::cout << std::setw(30) << this->controller->getAllocator()->getName() << "\n";

  std::cout << "\n";
  std::cout << std::setfill('-') << std::setw(11) << std::left << "+";
  std::cout << std::setw(15) << "+";
  std::cout << std::setw(23) << "+";
  std::cout << std::setw(25) << "+";
  std::cout << std::setw(15) << "+";
  std::cout << std::setw(20) << "+";
  std::cout << std::setw(17) << "+";
  std::cout << std::setw(13) << "+";
  std::cout << std::setw(1)  << "+\n";

  std::cout << std::setfill(' ') << std::setw(11) << "| Period";
  std::cout << std::setw(15) << "| Total LPs";
  std::cout << std::setw(23) << "| Req. Cap. (Tbps)";
  std::cout << std::setw(25) << "| Alloc. Cap. (Tbps)";
  std::cout << std::setw(15) << "| Util. (%)";
  std::cout << std::setw(20) << "| Underprv. (%)";
  std::cout << std::setw(17) << "| Growth (%)";
  std::cout << std::setw(13) << "| time(s)";
  std::cout << std::setw(1)  << "|\n";

  std::cout << std::setfill('-') << std::setw(11) << std::left << "+";
  std::cout << std::setw(15) << "+";
  std::cout << std::setw(23) << "+";
  std::cout << std::setw(25) << "+";
  std::cout << std::setw(15) << "+";
  std::cout << std::setw(20) << "+";
  std::cout << std::setw(17) << "+";
  std::cout << std::setw(13) << "+";
  std::cout << std::setw(1)  << "+\n";
  this->startingTime = std::chrono::high_resolution_clock::now();
}

void Simulator::printRow(bool highVerbose) {
  // Calculate elapsed time
  this->checkTime = std::chrono::high_resolution_clock::now();
  this->timeDuration = std::chrono::duration_cast<std::chrono::duration<double>>(
      this->checkTime - this->startingTime);

  // Initialize metrics variables
  int totalNodes = this->controller->getNetwork()->getNumberOfNodes();
  int totalConnections = this->controller->getConnections().size();
  int totalDemands = 0;
  int underProvisionedCount = 0;
  
  double totalRequired = 0.0;
  double totalAllocated = 0.0;
  double totalUnderprovisioned = 0.0;
  double totalOverprovisioned = 0.0;
  
  bool isSymmetric = true;
  const double epsilon = 1e-9;

  // Calculate demand statistics
  for (int src = 0; src < totalNodes; src++) {
    for (int dst = 0; dst < totalNodes; dst++) {
      if (this->getDemand(src, dst)) {
        // Get demand object
        auto demand = this->getDemand(src, dst);
        double required = demand->getRequiredCapacity();
        double allocated = demand->getAllocatedCapacity();

        // Update total demand statistics
        totalDemands++;
        totalRequired += required;
        totalAllocated += allocated;

        // Calculate deficit and excess per demand
        double deficit = std::max(0.0, required - allocated);
        double excess = std::max(0.0, allocated - required);
        totalUnderprovisioned += deficit;
        totalOverprovisioned += excess;

        if (!demand->isProvisioned() || deficit > epsilon) {
          underProvisionedCount++;
        }

        // Check symmetry of allocated bit rates matrix
        if (this->getDemand(src, dst)) {
          double rate2 = this->getDemand(dst, src)->getAllocatedCapacity();
          if (std::abs(allocated - rate2) > epsilon) {
            isSymmetric = false;
          }
        }
      }
    }
  }

  // Calculate ratios
  double underProvisioningRatio = totalRequired > 0 ? (totalUnderprovisioned / totalRequired) * 100 : 0.0;

  // Calculate network utilization based on used slots
  std::shared_ptr<Network> network = this->controller->getNetwork();
  int totalSlots = 0;
  int usedSlots = 0;
  for (auto link : network->getLinks()) {
    // Per fiber
    for (auto fiber : link->getFibers()) {
      // Per band
      for (auto band : fiber.get()->getBands()) {
        // Per core
        for (int coreIndex = 0; coreIndex < fiber.get()->getNumberOfCores(); coreIndex++) {
          // Per mode
          for (int modeIndex = 0; modeIndex < fiber.get()->getNumberOfModes(coreIndex, band); modeIndex++) {
            // Per slot
            for (int slotIndex = 0; slotIndex < fiber.get()->getNumberOfSlots(coreIndex, band, modeIndex); slotIndex++) {
              if (fiber.get()->getSlot(coreIndex, band, modeIndex, slotIndex)) {
                usedSlots++;
              }
              totalSlots++;
            }
          }
        }
      }
    }
  }
  double utilization = totalSlots > 0 ? (static_cast<double>(usedSlots) / static_cast<double>(totalSlots)) * 100.0 : 0.0;

  // Print tabular row
  std::cout << "|" << std::setfill(' ') << std::right << std::setw(9) << this->currentPeriod << " |";
  std::cout << std::setw(13) << this->controller.get()->getConnections().size() << " |";
  std::cout << std::setw(21) << std::fixed << std::setprecision(2) << totalRequired / 1000.0 << " |";
  std::cout << std::setw(23) << std::fixed << std::setprecision(2) << totalAllocated / 1000.0 << " |";
  std::cout << std::setw(13) << std::fixed << std::setprecision(1) << utilization << " |";
  std::cout << std::setw(18) << std::fixed << std::setprecision(1) << underProvisioningRatio << " |";

  // Print growth percentage or dash for first period
  std::cout << std::setw(15);
  if (this->currentPeriod == 1) {
    std::cout << "-";
  } else {
    std::cout << std::fixed << std::setprecision(1) << (this->growthVariable.getCurrentValue()) * 100.0;
  }
  std::cout << " |";

  std::cout << std::setw(11) << std::fixed << std::setprecision(0) << this->timeDuration.count() << " |";
  std::cout << "\n";

  // Generate detailed report if verbose mode is enabled
  if (highVerbose) {

    // Create results directory if it doesn't exist
    std::filesystem::path outDir = std::filesystem::path("results");
    std::error_code ec;
    std::filesystem::create_directories(outDir, ec); // no-op if exists

    if (ec) {
      std::cerr << "Warning: Could not create results directory '" << outDir.string()
                << "': " << ec.message() << "\n";
      return; // Skip file output if directory creation fails
    }
    
    // Guard against a path that exists but isn't a directory
    if (!std::filesystem::exists(outDir) || !std::filesystem::is_directory(outDir)) {
      std::cerr << "Warning: Path '" << outDir.string()
                << "' is not a directory.\n";
      return;
    }

    // Define output file path
    std::filesystem::path filePath = outDir / "period_report.txt";

    // Open file for appending
    std::ofstream outFile(filePath, std::ios::app);
    if (!outFile.is_open()) {
      std::cerr << "Error: Could not open file " << filePath.string() << "\n";
      return;
    }

    // Write period header
    outFile << "\nPeriod " << this->currentPeriod << ":\n";
    outFile << std::string(140, '=') << "\n";
    
    // Write table header
    outFile << std::setfill(' ') << std::left;
    outFile << std::setw(13) << "Demand ID";
    outFile << "| " << std::setw(40) << "Src -> Dst";
    outFile << "| " << std::setw(17) << "Required (Gbps)";
    outFile << "| " << std::setw(18) << "Allocated (Gbps)";
    outFile << "| " << std::setw(16) << "Deficit (Gbps)";
    outFile << "| " << std::setw(18) << "Status" << "\n";
    
    // Write table separator
    outFile << std::setfill('-');
    outFile << std::setw(13) << "-";
    outFile << "+" << std::setw(41) << "-";
    outFile << "+" << std::setw(18) << "-";
    outFile << "+" << std::setw(19) << "-";
    outFile << "+" << std::setw(17) << "-";
    outFile << "+" << std::setw(19) << "-" << "\n";
    
    // Write demand details
    for (int src = 0; src < totalNodes; src++) {
      for (int dst = 0; dst < totalNodes; dst++) {
        // Skip diagonal, non-existent demands, or lower triangle if symmetric
        if (src == dst || !this->getDemand(src, dst) || (isSymmetric && src > dst)) {
          continue;
        }

        auto demand = this->getDemand(src, dst);
        double required = demand->getRequiredCapacity();
        double allocated = demand->getAllocatedCapacity();
        double deficit = std::max(0.0, required - allocated);
        std::string status = !demand->isProvisioned() ? "Underprovisioned" : "OK";

        // Get node names (use labels if available)
        std::unique_ptr<Node>& srcNode = this->controller->getNetwork()->getNode(src);
        std::unique_ptr<Node>& dstNode = this->controller->getNetwork()->getNode(dst);
        std::string srcName = srcNode->getLabel().value_or("Node " + std::to_string(src));
        std::string dstName = dstNode->getLabel().value_or("Node " + std::to_string(dst));
        std::string connector = isSymmetric ? " <-> " : " -> ";
        std::string srcDstStr = srcName + connector + dstName;
        
        // Write demand row
        outFile << std::setfill(' ') << std::left;
        outFile << std::setw(13) << demand->getId();
        outFile << "| " << std::setw(40) << srcDstStr;
        outFile << "| " << std::setw(17) << std::fixed << std::setprecision(1) << required;
        outFile << "| " << std::setw(18) << std::fixed << std::setprecision(1) << allocated;
        outFile << "| " << std::setw(16) << std::fixed << std::setprecision(1) << deficit;
        outFile << "| " << std::setw(18) << status << "\n";
      }
    }
    
    outFile << std::string(140, '=') << "\n";
    
    // Write period summary
    double underProvisioningRatio = totalRequired > 0 ? (totalUnderprovisioned / totalRequired) : 0.0;
    double overProvisioningRatio = totalRequired > 0 ? (totalOverprovisioned / totalRequired) : 0.0;
    
    outFile << "PERIOD SUMMARY:\n";
    outFile << "Total Connections: " << totalConnections << "\n";
    outFile << "Aggregate Required Capacity: " << std::fixed << std::setprecision(2) << totalRequired / 1000.0 << " Tbps\n";
    outFile << "Aggregate Allocated Capacity: " << std::fixed << std::setprecision(2) << totalAllocated / 1000.0 << " Tbps\n";
    outFile << "Underprovisioned Demands: " << underProvisionedCount << "/" << totalDemands 
              << " (" << std::fixed << std::setprecision(1) << (totalDemands > 0 ? (100.0 * underProvisionedCount / totalDemands) : 0.0) << "%)\n";
    outFile << "Underprovisioning Ratio: " << std::fixed << std::setprecision(3) << underProvisioningRatio
              << " (" << std::fixed << std::setprecision(1) << (underProvisioningRatio * 100.0) << "% of capacity deficit)\n";
    outFile << "Overprovisioning Ratio: " << std::fixed << std::setprecision(3) << overProvisioningRatio 
              << " (" << std::fixed << std::setprecision(1) << (overProvisioningRatio * 100.0) << "% excess capacity)\n";
    outFile << "Resource Utilization: " << std::fixed << std::setprecision(1) << utilization << "%\n";
    outFile << std::string(140, '=') << "\n\n";

    outFile.close();
  }
}

void Simulator::printFinalInfo() {
  this->checkTime = std::chrono::high_resolution_clock::now();
  this->timeDuration = std::chrono::duration_cast<std::chrono::duration<double>>(
      this->checkTime - this->startingTime);

  std::cout << std::setfill('-') << std::setw(11) << std::left << "+";
  std::cout << std::setw(15) << "+";
  std::cout << std::setw(23) << "+";
  std::cout << std::setw(25) << "+";
  std::cout << std::setw(15) << "+";
  std::cout << std::setw(20) << "+";
  std::cout << std::setw(17) << "+";
  std::cout << std::setw(13) << "+";
  std::cout << std::setw(1)  << "+\n";

  std::cout << "\n--- Simulation Completed in "
            << std::fixed << std::setprecision(1)
            << this->timeDuration.count()
            << " seconds ---\n\n";
}
