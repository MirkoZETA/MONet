#include "connection.hpp"

Connection::Connection(void) {
  this->id = -1;
  this->src = -1;
  this->dst = -1;
  this->links = std::vector<int>();
  this->slots = std::vector<std::vector<int> >();
  this->cores = std::vector<int>();
  this->modes = std::vector<int>();
  this->bands = std::vector<fns::Band>();
  this->timeConnection = 0.0;
  this->bitRate = nullptr;
  this->_isAllocatedInP2P = false;
}

Connection::Connection(std::shared_ptr<const BitRate> bitRate, int src, int dst) {
  if (!bitRate) {
    throw std::invalid_argument("BitRate cannot be null.");
  }
  if (src < 0) {
    throw std::invalid_argument("Source node ID cannot be negative.");
  }
  if (dst < 0) {
    throw std::invalid_argument("Destination node ID cannot be negative.");
  }
  this->id = -1;
  this->src = src;
  this->dst = dst;
  this->links = std::vector<int>();
  this->slots = std::vector<std::vector<int> >();
  this->cores = std::vector<int>();
  this->modes = std::vector<int>();
  this->bands = std::vector<fns::Band>();
  this->timeConnection = 0.0;
  this->bitRate = bitRate;
  this->_isAllocatedInP2P = false;
}

Connection::Connection(int id, double time, std::shared_ptr<const BitRate> bitRate, bool _isAllocatedInP2P, int src, int dst) {
  if (id < 0) {
    throw std::invalid_argument("Connection ID cannot be negative.");
  }
  if (time < 0) {
    throw std::invalid_argument("Connection time cannot be negative.");
  }
  if (!bitRate) {
    throw std::invalid_argument("BitRate cannot be null.");
  }
  if (src < 0) {
    throw std::invalid_argument("Source node ID cannot be negative.");
  }
  if (dst < 0) {
    throw std::invalid_argument("Destination node ID cannot be negative.");
  }
  this->id = id;
  this->src = src;
  this->dst = dst;
  this->links = std::vector<int>();
  this->slots = std::vector<std::vector<int> >();
  this->cores = std::vector<int>();
  this->modes = std::vector<int>();
  this->bands = std::vector<fns::Band>();
  this->timeConnection = time;
  this->bitRate = bitRate;
  this->_isAllocatedInP2P = _isAllocatedInP2P;
}

void Connection::addLink(int idLink, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo) {
  if (idLink < 0) {
    throw std::invalid_argument("Link ID cannot be negative.");
  }
  if (fiber < 0) {
    throw std::invalid_argument("Fiber index cannot be negative.");
  }
  if (core < 0) {
    throw std::invalid_argument("Core index cannot be negative.");
  }
  if (mode < 0) {
    throw std::invalid_argument("Mode index cannot be negative.");
  }
  if (slotFrom < 0 || (slotTo < 0 && slotTo != -1)) {
    throw std::invalid_argument("Slot indices cannot be negative.");
  }
  if (slotFrom >= slotTo && slotTo != -1) {
    throw std::invalid_argument("Invalid slot range.");
  }
  this->links.push_back(idLink);
  this->fibers.push_back(fiber);
  this->cores.push_back(core);
  this->bands.push_back(band);
  this->modes.push_back(mode);
  this->slots.push_back(std::vector<int>(slotTo - slotFrom));
  int j = 0;
  for (int i = slotFrom; i < slotTo; i++) {
    this->slots.back()[j] = i;
    j++;
  }
}

void Connection::addLink(std::shared_ptr<Link> link, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo) {
  if (!link) {
    throw std::invalid_argument("Link cannot be null.");
  }
  Connection::addLink(link.get()->getId(), fiber, core, band, mode, slotFrom, slotTo);
}

void Connection::addLink(int idLink, int fiber, int core, fns::Band band, int mode, std::vector<int> slots) {
  if (idLink < 0) {
    throw std::invalid_argument("Link ID cannot be negative.");
  }
  // Check that the linkId is not already present
  for (size_t i = 0; i < this->links.size(); ++i) {
    if (this->links[i] == idLink) {
      throw std::invalid_argument("Link ID already exists in the connection.");
    }
  }
  if (fiber < 0) {
    throw std::invalid_argument("Fiber index cannot be negative.");
  }
  if (core < 0) {
    throw std::invalid_argument("Core index cannot be negative.");
  }
  if (mode < 0) {
    throw std::invalid_argument("Mode index cannot be negative.");
  }
  if (slots.empty()) {
    throw std::invalid_argument("Slots cannot be empty.");
  }
  for (int slot : slots) {
    if (slot < 0) {
      throw std::invalid_argument("Slot indices cannot be negative.");
    }
  }
  this->links.push_back(idLink);
  this->fibers.push_back(fiber);
  this->cores.push_back(core);
  this->bands.push_back(band);
  this->modes.push_back(mode);
  this->slots.push_back(slots);
}

bool Connection::isAllocatedInP2P() const {
  return this->_isAllocatedInP2P;
}

void Connection::setAllocatedInP2P(bool value) {
  this->_isAllocatedInP2P = value;
}

void Connection::changeBitRate(std::shared_ptr<const BitRate> newBitRate) {
  if (!newBitRate) {
    throw std::invalid_argument("New BitRate cannot be null.");
  }
  this->bitRate = newBitRate;
}




void Connection::setId(int id) {
  if (id < 0) {
    throw std::invalid_argument("Connection ID cannot be negative.");
  }
  if (this->id != -1) {
    throw std::runtime_error("Connection ID is already set and cannot be changed.");
  }
  this->id = id;
}
void Connection::setTime(double time) {
  if (time < 0) {
    throw std::invalid_argument("Connection time cannot be negative.");
  }
  this->timeConnection = time;
}


int Connection::getSrc(void) const { return this->src; }
int Connection::getDst(void) const { return this->dst; }
int Connection::getId(void) const { return this->id; }
double Connection::getTimeConnection(void) const { return this->timeConnection; }
std::shared_ptr<const BitRate> Connection::getBitrate(void) const { return this->bitRate; }


const std::vector<int>& Connection::getLinks(void) const { return this->links; }
const std::vector<int>& Connection::getFibers(void) const { return this->fibers; }
const std::vector<int>& Connection::getCores(void) const { return this->cores; }
const std::vector<fns::Band>& Connection::getBands(void) const { return this->bands; }
const std::vector<int>& Connection::getModes(void) const { return this->modes; }
const std::vector<std::vector<int>>& Connection::getSlots(void) const { return this->slots; }
