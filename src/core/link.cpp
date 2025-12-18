#include "link.hpp"

Link::Link()
  : id(-1),
    length(fns::defaults::LENGTH),
    src(-1),
    dst(-1)
{
  // Default constructor starts with zero fibers
}
Link::Link(int id)
  : id(id),
    length(fns::defaults::LENGTH),
    src(-1),
    dst(-1)
{
  // Constructor with ID starts with zero fibers
}
Link::Link(int id, float length){
  if (length <= 0)
    throw std::invalid_argument("Cannot create a link with non-positive length.");
  this->id = id;
  this->length = length;
  this->src = -1;
  this->dst = -1;

  // Constructor starts with zero fibers
}

// For network construction:
Link::Link(int id, float length, std::shared_ptr<Fiber> fiber) {
  if (length <= 0)
    throw std::invalid_argument("Cannot create a link with non-positive length.");
  if (!fiber)
    throw std::invalid_argument("Cannot create a link with null fiber.");
  
  this->id = id;
  this->length = length;
  this->src = -1;
  this->dst = -1;

  // Detect type for the fiber
  fiber->detectType();
  
  this->fibers.reserve(1);
  this->fibers.push_back(fiber);
}

Link::Link(int id, float length, std::vector<std::shared_ptr<Fiber>> fibers) {
  if (length <= 0)
    throw std::invalid_argument("Cannot create a link with non-positive length.");
  if (fibers.empty())
    throw std::invalid_argument("Cannot create a link with empty fiber vector.");
  for (const auto& fiber : fibers) {
    if (!fiber)
      throw std::invalid_argument("Cannot create a link with null fiber in vector.");
  }
  
  this->id = id;
  this->length = length;
  this->src = -1;
  this->dst = -1;

  // Detect type for each fiber 
  for (auto& fiber : fibers) {
    fiber->detectType();
  }
  
  this->fibers = fibers;
}



void Link::setId(int id) {
  if (this->id != -1)
    throw std::runtime_error(
        "Cannot set Id to a Link with Id different than -1.");
  this->id = id;
}
void Link::setLength(float length) {
  if (length <= 0)
    throw std::invalid_argument("Cannot set a link with non-positive length.");
  this->length = length;
}
void Link::setSrc(int src) {
  if (src < 0)
    throw std::invalid_argument("Cannot set a link with negative source node id.");
  this->src = src;
}
void Link::setDst(int dst) {
  if (dst < 0)
    throw std::invalid_argument("Cannot set a link with negative destination node id.");
  this->dst = dst;
}



int Link::getId(void) const {
  return this->id;
}
double Link::getLength(void) const {
  return this->length;
}
int Link::getSrc(void) const {
  return this->src;
}
int Link::getDst(void) const {
  return this->dst;
}
float Link::getUsagePercentage(void) const {
  int totalSlots = 0;
  int usedSlots = 0;

  for (const auto& fiber : this->fibers) {
    if (fiber) {
      for (fns::Band band : fiber->getBands()) {
        for (size_t coreIdx = 0; coreIdx < fiber->getNumberOfCores(); ++coreIdx) {
          for (size_t modeIdx = 0; modeIdx < fiber->getNumberOfModes(coreIdx, band); ++modeIdx) {
            int slots = fiber->getNumberOfSlots(coreIdx, band, modeIdx);
            totalSlots += slots;
            for (size_t slotIdx = 0; slotIdx < slots; ++slotIdx) {
              if (fiber->getSlot(coreIdx, band, modeIdx, slotIdx) != -1) {
                usedSlots++;
              }
            }
          }
        }
      }
    }
  }
  if (totalSlots == 0) return 0.0f; // Avoid division by zero

  return (static_cast<float>(usedSlots) / static_cast<float>(totalSlots)) * 100.0f;
}


std::vector<std::shared_ptr<Fiber>> Link::getFibers(void) const {
  return this->fibers;
}
std::shared_ptr<Fiber> Link::getFiber(int index) const {
  if (index < 0 || index >= this->fibers.size()) {
    throw std::out_of_range("Fiber index out of range.");
  }
  return this->fibers[index];
}
int Link::getNumberOfFibers(void) const {
  return this->fibers.size();
}


void Link::addFiber(std::shared_ptr<Fiber> fiber) {
  this->fibers.push_back(fiber);
}
void Link::addCable(fns::FiberType type, int numberOfFibers) {
  if (numberOfFibers <= 0)
    throw std::invalid_argument("Cannot add a cable with non-positive number of fibers.");
  
  for (int i = 0; i < numberOfFibers; i++) {
    std::shared_ptr<Fiber> newFiber;
    
    switch (type)
    {
    case fns::FiberType::SSMF:
      // SSMF: 1 core, 1 mode, fns::defaults::SLOTS slots
      newFiber = std::make_shared<Fiber>();
      break;
    case fns::FiberType::MCF:
      {
        // MCF: DEFAULT_CORES cores, 1 mode each, fns::defaults::SLOTS slots
        std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix;
        bandSlotMatrix[fns::Band::C] = std::vector<std::vector<int>>(fns::defaults::CORES, std::vector<int>(1, fns::defaults::SLOTS));
        newFiber = std::make_shared<Fiber>(bandSlotMatrix);
      }
      break;
    case fns::FiberType::FMF:
      {
        // FMF: 1 core, 6 modes, fns::defaults::SLOTS slots each
        std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrixFMF;
        bandSlotMatrixFMF[fns::Band::C] = std::vector<std::vector<int>>(1, std::vector<int>(6, fns::defaults::SLOTS));
        newFiber = std::make_shared<Fiber>(bandSlotMatrixFMF);
      }
      break;
    case fns::FiberType::FMMCF:
      {
        // FMMCF: 7 cores, 5 modes each, fns::defaults::SLOTS slots each
        std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrixFMMCF;
        bandSlotMatrixFMMCF[fns::Band::C] = std::vector<std::vector<int>>(7, std::vector<int>(5, fns::defaults::SLOTS));
        newFiber = std::make_shared<Fiber>(bandSlotMatrixFMMCF);
      }
      break;
    default:
      throw std::invalid_argument("Unknown fiber type.");
    }
    
    // Set the fiber type explicitly and detect if needed
    newFiber->setType(type);
    this->fibers.push_back(newFiber);
  }
}