#include "demand.hpp"

Demand::Demand(int id, int src, int dst, double requiredCapacity) 
	: id(id), src(src), dst(dst),
  requiredCapacity(requiredCapacity), allocatedCapacity(0.0) {
	if (id < 0) {
		throw std::invalid_argument("Demand ID must be non-negative.");
	}
	if (src < 0) {
		throw std::invalid_argument("Source node ID must be non-negative.");
	}
	if (dst < 0) {
		throw std::invalid_argument("Destination node ID must be non-negative.");
	}
	if (src == dst) {
		throw std::invalid_argument("Source and destination nodes must be different.");
	}
	if (requiredCapacity < 0.0) {
		throw std::invalid_argument("Required capacity must be non-negative.");
	}
}

void Demand::setId(int id) {
  if (id < 0) {
    throw std::invalid_argument("Demand ID must be non-negative.");
  }
  this->id = id; 
}
void Demand::setSrc(int src) {
  if (src < 0) {
    throw std::invalid_argument("Source node ID must be non-negative.");
  }
  this->src = src; 
}
void Demand::setDst(int dst) {
  if (dst < 0) {
    throw std::invalid_argument("Destination node ID must be non-negative.");
  }
  this->dst = dst; 
}
void Demand::setRequiredCapacity(double capacity) {
	if (capacity < 0.0) {
		throw std::invalid_argument("Required capacity must be non-negative.");
	}
	requiredCapacity = capacity; 
}

int Demand::getId() const { return this->id; }
int Demand::getSrc() const { return this->src; }
int Demand::getDst() const { return this->dst; }

double Demand::getRequiredCapacity() const { return this->requiredCapacity; }
double Demand::getAllocatedCapacity() const { return this->allocatedCapacity; }

double Demand::getUnprovisionedCapacity() const { 
    double underprovisioned = requiredCapacity - allocatedCapacity;
    if (underprovisioned < 0.0) return 0.0;
    else return underprovisioned; 
}

void Demand::addAllocatedCapacity(double capacity) {
	if (capacity < 0.0) {
		throw std::invalid_argument("Capacity to add must be non-negative.");
	}
	allocatedCapacity += capacity; 
}

void Demand::subtractAllocatedCapacity(double capacity) {
	if (capacity < 0.0) {
		throw std::invalid_argument("Capacity to subtract must be non-negative.");
	}
	if (allocatedCapacity < capacity) {
		throw std::runtime_error("Cannot subtract more capacity than currently allocated.");
	}
	allocatedCapacity -= capacity; 
}

bool Demand::isProvisioned() const { 
	return allocatedCapacity >= requiredCapacity; 
}

double Demand::getProvisioningRatio() const {
	if (requiredCapacity == 0.0) return 0.0;
	return allocatedCapacity / requiredCapacity;
}

bool Demand::isNull() const {
    return id < 0;
}




