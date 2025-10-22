#ifndef __P2P_H__
#define __P2P_H__

// STL
#include <memory>
#include <vector>
// core
#include "fiber.hpp"

/**
 * @brief Point-to-Point class
 * 
 * P2P Class represent direct connections between two nodes.
 * They can share fibers with the original Link, allowing some fibers
 * to be dedicated to P2P traffic while others remain available for routing.
 */
class P2P
{
private:
	int id;
	int srcNode;
	int dstNode;
	std::vector<std::shared_ptr<Fiber>> fibers;  // Shared fibers with Link
	std::vector<int> fiberIdxs; // Indices of fibers in the Link
	std::vector<int> linkIds; // IDs of Links from which fibers are taken

public:
	P2P(int id, int src, int dst);

	// Getters
	int getId() const;
	int getSrc() const;
	int getDst() const;
	std::vector<int> getLinkIds() const;
	std::vector<int> getFiberIdxs() const;

	// Resource management
	void addFiber(int linkId, int fiberIdx, std::shared_ptr<Fiber> fiber);
	std::shared_ptr<Fiber> getFiber(int index) const;
	std::vector<std::shared_ptr<Fiber>> getFibers() const;

	/**
	 * @brief Reserve spectrum slots across all fibers in the P2P connection.
	 * 
	 * This method provides a convenient way to allocate the same spectrum slots across
	 * all fibers that comprise this point-to-point connection. Unlike Network::useSlot(),
	 * which operates on individual links and requires separate calls for each link in a
	 * multi-hop path, this method simultaneously reserves the specified slots on all
	 * fibers in the P2P container with a single call.
	 * 
	 * @param core Core index within each fiber (0-based indexing)
	 * @param band Spectral band (C, L, S, E, U, or O band) - see Band enum
	 * @param mode Spatial mode index within the specified core (0-based indexing)
	 * @param slotFrom Starting slot index (inclusive) - first slot to reserve
	 * @param slotTo Ending slot index (exclusive) - first slot NOT to reserve
	 * @param connectionId Unique identifier of the connection claiming these slots
	 * 
	 * @note All fibers in this P2P container will have the same slots reserved
	 * @note The slot range [slotFrom, slotTo) follows half-open interval convention
	 * @see Network::useSlot() for single-link slot allocation
	 */
	void useSlots(int core, fns::Band band, int mode, int slotFrom, int slotTo, int connectionId);
};

#endif