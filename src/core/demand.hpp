#ifndef __DEMAND_H__
#define __DEMAND_H__

// STL
#include <stdexcept>
#include <vector>
// core
#include "connection.hpp"

/**
 * @brief Class Demand represents traffic requirements between node pairs.
 *
 * This class tracks both required bitrate (driven by growth models) and
 * currently allocated bitrate for each source-destination pair in
 * incremental simulation mode.
 */
class Demand
{
public:
	/**
	 * @brief Default constructor for Demand, initializes members to default values.
	 */
	Demand() noexcept
			: id(-1), src(-1), dst(-1),
				requiredCapacity(0.0), allocatedCapacity(0.0) {}
	/**
	 * @brief Construct a new Demand object
	 *
	 * @param id Unique identifier for this demand
	 * @param src Source node ID
	 * @param dst Destination node ID
	 * @param requiredCapacity Initial required capacity
	 */
	Demand(int id, int src, int dst, double requiredCapacity);

	/**
	 * @brief Set the unique identifier for this demand
	 * @param id Unique identifier
	 */
	void setId(int id);
	/**
	 * @brief Set the source node ID
	 * @param src Source node ID
	 */
	void setSrc(int src);
	/**
	 * @brief Set the destination node ID
	 * @param dst Destination node ID
	 */
	void setDst(int dst);
	/**
	 * @brief Set the required capacity
	 * @param bitRate New required capacity
	 */
	void setRequiredCapacity(double bitRate);
	/**
	 * @brief Get the unique identifier for this demand
	 * @return int Unique identifier
	 */
	int getId() const;

	/**
	 * @brief Get the source node ID
	 * @return int Source node ID
	 */
	int getSrc() const;

	/**
	 * @brief Get the destination node ID
	 * @return int Destination node ID
	 */
	int getDst() const;

	/**
	 * @brief Get the required capacity (in Gbps)
	 * @return double Required capacity
	 */
	double getRequiredCapacity() const;

	/**
	 * @brief Get the allocated capacity (in Gbps)
	 * @return double Allocated capacity
	 */
	double getAllocatedCapacity() const;

	/**
	 * @brief Get the unprovisioned capacity (required - allocated) (in Gbps)
	 * @return double Unprovisioned capacity
	 */
	double getUnprovisionedCapacity() const;

	/**
	 * @brief Add to the allocated capacity
	 * @param bitRate Amount to add
	 */
	void addAllocatedCapacity(double bitRate);

	/**
	 * @brief Subtract from the allocated capacity
	 * @param bitRate Amount to subtract
	 */
	void subtractAllocatedCapacity(double bitRate);

	/**
	 * @brief Check if the demand is overprovisioned
	 * @return true if overprovisioned, false otherwise
	 */
	bool isProvisioned() const;

	/**
	 * @brief Get the provisioning ratio (allocated / required)
	 * @return double Provisioning ratio
	 */
	double getProvisioningRatio() const;

	/**
	 * @brief Check if the demand is null (uninitialized)
	 * @return true if null, false otherwise
	 */
	bool isNull() const;

	// TODO:
	void addConnection(Connection *conn);
	void removeConnection(Connection *conn);
	const std::vector<Connection *> &getConnections() const;



private:
	int id;
	int src;
	int dst;
	/**
	 * @brief Required capacity in Gbps
	 */
	double requiredCapacity;
	/**
	 * @brief Currently allocated capacity in Gbps
	 */
	double allocatedCapacity;
	/**
	 * @brief Connections fulfilling this demand
	 */
	std::vector<Connection *> connections;
};

#endif // __DEMAND_H_