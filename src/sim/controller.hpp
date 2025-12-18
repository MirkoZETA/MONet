#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

// STL
#include <memory>
#include <unordered_set>
// alloc
#include "../alloc/allocator.hpp"
// core
#include "../core/connection.hpp"
#include "../core/network.hpp"
#include "../core/p2p.hpp"
#include "../core/demand.hpp"
// sim
#include "event.hpp"

/**
 * @brief Class with the controller information.
 *
 * This class allows you to create the object Controller and manipulate it by
 * its methods. The importance of this object is that it handles connections
 * allocation and P2P management. The Controller is the link between the Simulator
 * and the Network.
 *
 */
class Controller
{
public:

	/**
	 * @brief Constructs a new Controller object. The attribute connections is
	 * assigned to an empty Connection vector. This object does not have a network
	 * or connections registered.
	 *
	 */
	Controller();
	/**
	 * @brief Constructs a new Controller object. It takes a Network object and
	 * stores it as an attribute. An empty Connection vector is created and
	 * stored as well.
	 *
	 * @param network The pointer type Network object. This contains all
	 * the information about the network, nodes, routes, path length and slots.
	 */
	Controller(std::shared_ptr<Network> network);

	/**
	 * @brief Sets the Network object as the network attribute of the controller.
	 * This is the network that the controller will now handle and who will
	 * interact with the simulator
	 *
	 * @param network The pointer type Network object. This contains all
	 * the information about the network, nodes, routes, path length and slots.
	 */
	void setNetwork(std::shared_ptr<Network> network);
	/**
	 * @brief Get the Network in the controller. The return contains all
	 * the information about the network, nodes, routes, path length and slots.
	 *
	 * @return Network. a pointer to the network object from the controller. This
	 * is the network the controller is currently using
	 */
	// 1. Non-const version: allows modifying the original network pointer
	std::shared_ptr<Network>& getNetwork();

	// 2. Const version: keeps existing behavior (returns a copy)
	std::shared_ptr<Network> getNetwork() const;

	/**
	 * @brief Set the allocator of the controller. The allocator determines how
	 * the connections will be assigned in the network.
	 *
	 * @param allocator A pointer to the allocator object.
	 */
	void setAllocator(std::unique_ptr<Allocator> allocator);
	/**
	 * @brief Get the Allocator object of the controller. The allocator determines
	 * how the connections will be assigned in the network.
	 *
	 * @return Allocator* a pointer to the allocator object.
	 */
	std::unique_ptr<Allocator> &getAllocator(void);

	/**
	 * @brief Set the Callback object
	 * This function sets a callback to the simulation. It is called
	 * after every event. It is useful to
	 * get the time between last provisioning and the next update event.
	 *
	 * @param callbackFunction a pointer to inter-event function
	 */
	void setCallbackFunction(
			void (*callbackFunction)(
					Network &network,																			 // modifiable reference
					std::vector<std::vector<Demand>> &demands,						 // modifiable reference
					std::vector<std::unique_ptr<Connection>> &connections, // modifiable reference
					double time));
	/**
	 * @brief Set the failure management function.
	 * This function will be called in case of a failure in the simulation.
	 * It allows to implement custom failure handling strategies.
	 * @param failureManagementFunction a pointer to the failure management function
	 */
	void setFailureManagementFunction(
			void (*failureManagementFunction)(
					Network &network,																			 // modifiable reference
					std::vector<std::vector<Demand>> &demands,						 // modifiable reference
					std::vector<std::unique_ptr<Connection>> &connectionsAffected, // modifiable reference
					eventType eventType,
					double time));

	/**
	 * @brief Adds a new connection to the controller's list of connections.
	 * The new connection is created with the given parameters and added to the
	 * connections vector. The connection ID is automatically assigned based on
	 * the internal connectionCounter.
	 */
	void addConnection(std::unique_ptr<Connection> &&connection);
	/**
	 * @brief Get a connection by its ID.
	 * 
	 * Note: This method searches for the connection by ID, not by vector index.
	 * Connection IDs are managed by an internal counter and may not correspond
	 * to the vector index if connections are removed or reordered.
	 *
	 * @param idConnection The ID of the connection.
	 * @return Connection& A reference to the connection object.
	 * @throws std::out_of_range if no connection with the given ID is found.
	 */
	Connection &getConnection(int idConnection);
	/**
	 * @brief Get the connections vector. This vector contains all the
	 * connections that are currently active in the network. It is used to keep
	 * track the connections and their respective slots.
	 *
	 * @return a vector of Connection objects.
	 */
	std::vector<std::unique_ptr<Connection>> &getConnections();
	/**
	 * @brief Provisions the demands by allocating the necessary resources in the
	 * network. This function pointer is set based on the network type (EON, SDM, MB).
	 *
	 * @param demands The vector of unique pointers to Demand objects.
	 * @param bitRates The vector of shared pointers to BitRate objects.
	 * @param time The current time in the simulation.
	 */
	void assignConnections(std::vector<std::vector<Demand>> &demands,
												 const std::vector<std::shared_ptr<BitRate>> &bitRates,
												 double time);

	/**
	 * @brief Computes k shortest paths for all node pairs using Yen's algorithm.
	 * For each pair of nodes (src, dst) where src < dst, computes k shortest paths
	 * and creates the reverse paths for (dst, src).
	 * @param k Number of shortest paths to compute (default: 3)
	 */
	void setPaths(int k = 3);
	/**
	 * @brief Sets the paths vector from the routes on the JSON file.
	 * @param pathsFilename name of the JSON file that contains the routes.
	 */
	void setPaths(std::string pathsFilename);
	/**
	 * @brief Get the Paths vector. This vector represents all the routes present
	 * in the network between the source and destination nodes.
	 *
	 * @return a raw pointer to the four dimensional vector which represents the
	 * paths (owned by Network), or nullptr if not set.
	 */
	Paths* getPaths(void) const;
	/**
	 * @brief Clears all stored paths.
	 */
	void clearPaths(void);

	/**
	 * @brief Adds a new P2P connection to the controller's list of P2P connections.
	 * The new P2P connection is created with the given source and destination nodes,
	 * and new fibers are created for each link in the specified path.
	 * The P2P ID is automatically assigned based on the internal p2pCounter.
	 *
	 * @param src Source node id
	 * @param dst Destination node id
	 * @param pathIdx Index of the path to use from the precomputed paths
	 * @param bandSlotMatrix Configuration matrix for fiber creation [band][core][mode] = slots
	 */
	void addP2P(int src, int dst, int pathIdx, const std::map<fns::Band, std::vector<std::vector<int>>> &bandSlotMatrix);
	/**
	 * @brief Adds a new P2P connection to the controller's list of P2P connections.
	 * The new P2P connection is created with the given source and destination nodes,
	 * and the specified fibers are used for each link in the specified path.
	 * The P2P ID is automatically assigned based on the internal p2pCounter.
	 *
	 * @param src Source node id
	 * @param dst Destination node id
	 * @param pathIdx Index of the path to use from the precomputed paths
	 * @param fiberIdxs Vector of fiber indices to be used in each link of the path
	 *
	 * @throws std::invalid_argument if the size of fiberIdxs does not match the number of links in the path
	 * @throws std::invalid_argument if any fiber is active or already assigned to another P2P
	 */
	void addP2P(int src, int dst, int pathIdx, std::vector<int> fiberIdxs);
	/**
	 * @brief Get a P2P connection by its ID.
	 * 
	 * Note: This method searches for the P2P by ID, not by vector index.
	 * P2P IDs are managed by an internal counter and may not correspond
	 * to the vector index if P2Ps are removed or reordered.
	 *
	 * @param id The ID of the P2P connection.
	 * @return P2P& A reference to the P2P connection object.
	 * @throws std::out_of_range if no P2P with the given ID is found.
	 */
	P2P &getP2P(int id);
	/**
	 * @brief Get all P2P connections.
	 *
	 * @return std::vector<std::unique_ptr<P2P>>& A reference to the vector of P2P connections.
	 */
	std::vector<std::unique_ptr<P2P>> &getP2Ps();
	/**
	 * @brief Migrates an existing connection to a specified P2P connection.
	 * This involves checking if the connection is not already allocated in a P2P,
	 * allocating the required slots in the specified P2P, and updating the
	 * connection's allocation status and bitrate.
	 *
	 * @param p2pId The ID of the P2P connection to migrate to.
	 * @param core The core index to use in the P2P fiber.
	 * @param band The band to use in the P2P fiber.
	 * @param mode The mode to use in the P2P fiber.
	 * @param slotFrom The starting slot index (inclusive) for allocation.
	 * @param slotTo The ending slot index (exclusive) for allocation.
	 * @param idConnection The ID of the connection to be migrated.
	 * @param bitRate A shared pointer to the new BitRate object for the connection.
	 *
	 * @throws std::runtime_error if the connection is already allocated in a P2P.
	 */
	void migrateConnectionToP2P(int p2pId, 
															int core, 
															fns::Band band, 
															int mode, 
															int slotFrom, 
															int slotTo, 
															int idConnection, 
															std::shared_ptr<const BitRate> bitRate);

	/**
	 * @brief Exports current demands to a JSON file named "demands_export.json" in the current directory.
	 * 
	 * The exported JSON contains a demands array with source, destination, required and allocated capacity for each demand,
	 * along with a timestamp of when the export was generated.
	 * 
	 * @param demands The demand matrix to export.
	 * @param time The current simulation time.
	 */
	void demandsToJson(const std::vector<std::vector<Demand>>& demands, double time) const;

	// Network modification methods

	/**
	 * @brief Adds a bidirectional link between two nodes in the network.
	 *
	 * By default this just marks the topology as dirty;
	 * actual adjacency/path recomputation happens later (lazy).
	 *
	 * @param src Source node index.
	 * @param dst Destination node index.
	 * @param length Link length.
	 * @param bandSlotMatrix Fiber slot configuration for the new link(s).
	 */
	void addLink(int src,
							 int dst,
							 float length,
							 const std::map<fns::Band, std::vector<std::vector<int>>> &bandSlotMatrix);
	/**
	 * @brief Adds a new node between time periods to the network with the given attributes.
	 *
	 * By default this just marks the topology as dirty;
	 * actual adjacency/path recomputation happens later (lazy).
	 *
	 * @param id The ID of the node (must be unique).
	 * @param dcs Optional number of data centers at the node.
	 * @param ixps Optional number of Internet exchange points at the node.
	 * @param population Optional population value associated with the node.
	 * @param label Optional string label for the node.
	 * @param longitude Optional longitude coordinate for the node's geographical location.
	 * @param latitude Optional latitude coordinate for the node's geographical location.
	 * @param param1 Optional parameter 1 for custom use.
	 * @param param2 Optional parameter 2 for custom use.
	 */
	void addNode(int id,
							 std::optional<int> dcs = std::nullopt,
							 std::optional<int> ixps = std::nullopt,
							 std::optional<double> population = std::nullopt,
							 std::optional<std::string> label = std::nullopt,
							 std::optional<double> longitude = std::nullopt,
							 std::optional<double> latitude = std::nullopt,
							 std::optional<double> param1 = std::nullopt,
							 std::optional<double> param2 = std::nullopt);	


	// TODO: MAKE IT PRIVATE and set a getter
	void (*failureManagementFunction)(
		Network& network,
		std::vector<std::vector<Demand>>& demands,
		std::vector<std::unique_ptr<Connection>>& connectionsAffected,
		eventType eventType,
		double time);

private:
	std::shared_ptr<Network> network;
	std::unique_ptr<Allocator> allocator;
	std::vector<std::unique_ptr<Connection>> connections;
	std::vector<std::unique_ptr<P2P>> p2ps;
	int connectionCounter;
	int p2pCounter;
	bool recompute;

	/**
	 * @brief Callback function called after every event in the simulation.
	 */
	void (*callbackFunction)(
			Network &network,// modifiable reference
			std::vector<std::vector<Demand>> &demands,// modifiable reference
			std::vector<std::unique_ptr<Connection>> &connections, // modifiable reference
			double time);

	// Path helpers
};

#endif
