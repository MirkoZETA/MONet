#include "p2p.hpp"
#include "link.hpp"
#include <stdexcept>

P2P::P2P(int id, int src, int dst)
    : id(id), srcNode(src), dstNode(dst) {
    if (src == dst) {
        throw std::invalid_argument("P2P container cannot have the same source and destination");
    }
}

int P2P::getId() const {
    return id;
}

int P2P::getSrc() const {
    return srcNode;
}

int P2P::getDst() const {
    return dstNode;
}

std::vector<int> P2P::getLinkIds() const {
    return linkIds;
}

std::vector<int> P2P::getFiberIdxs() const {
    return fiberIdxs;
}

void P2P::addFiber(int linkId, int fiberIdx, std::shared_ptr<Fiber> fiber) {
    if (!fiber) {
        throw std::invalid_argument("Cannot add null fiber to P2P container");
    }
    if (fiber.get()->isActive()) {
        throw std::invalid_argument("Cannot add an active fiber to P2P container");
    }
    if (fiber.get()->isDedicatedToP2P()) {
        throw std::invalid_argument("Fiber is already assigned to another P2P connection");
    }
    fiber.get()->setDedicatedToP2P(true); // Mark fiber as dedicated to P2P
    fibers.push_back(fiber);
    linkIds.push_back(linkId);
    fiberIdxs.push_back(fiberIdx);
}

std::shared_ptr<Fiber> P2P::getFiber(int index) const {
    if (index < 0 || index >= static_cast<int>(fibers.size())) {
        throw std::out_of_range("Fiber index out of range in P2P container");
    }
    return fibers[index];
}

std::vector<std::shared_ptr<Fiber>> P2P::getFibers() const {
    return fibers;
}

void P2P::useSlots(int core, fns::Band band, int mode, int slotFrom, int slotTo, int connectionId) {
    // Basic parameter validation
    if (slotFrom < 0) {
        throw std::invalid_argument("P2P::useSlot: slotFrom cannot be negative");
    }
    if (slotTo <= slotFrom) {
        throw std::invalid_argument("P2P::useSlot: slotTo must be greater than slotFrom");
    }
    if (core < 0) {
        throw std::invalid_argument("P2P::useSlot: core index cannot be negative");
    }
    if (mode < 0) {
        throw std::invalid_argument("P2P::useSlot: mode index cannot be negative");
    }
    if (fibers.empty()) {
        throw std::runtime_error("P2P::useSlot: no fibers available in P2P container");
    }

    // Reserve slots across all fibers in the P2P connection
    for (auto fiber : fibers) {
        if (!fiber) {
            throw std::runtime_error("P2P::useSlot: encountered null fiber in P2P container");
        }
        if (!fiber.get()->isDedicatedToP2P()) {
            throw std::runtime_error("P2P::useSlot: fiber is not dedicated to P2P traffic");
        }
        // Set each slot in the range [slotFrom, slotTo) for this fiber
        for (int slot = slotFrom; slot < slotTo; slot++) {
            fiber->setSlot(core, band, mode, slot, connectionId);
        }
    }
}

