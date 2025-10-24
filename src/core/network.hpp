#pragma once

// STL
#include <algorithm>
#include <fstream>
#include <limits>
#include <memory>
#include <unordered_set>
// core
#include "link.hpp"
#include "node.hpp"
// util
#include "../util/utils.hpp"
#include "../util/json.hpp"

/**
 * @brief Class with the network information.
 *
 * The Network class is used to represent an Optical Fiber Network architecture,
 * made up with connection Links and Nodes, inside the simulator. Hence, Network
 * class requires and implements Link and Node objects.
 *
 * The Network class consists of several methods for adding Links and Nodes, for
 * connecting them, check connection, use/unuse slots in Links, and getting
 * metrics.
 *
 */
class Network
{
public:
  /**
   * @brief
   *
   */
  Network(void);
  /**
   * @brief Constructs Network object from JSON file.
   *
   * Builds a network from a JSON file containing nodes and links arrays.
   * Supports both single-fiber and multi-fiber links with various configurations.
   *
   * **Node Structure:**
   * - Required: "id" (integer)
   * - Optional: "dcs" (data centers), "ixps" (Internet exchange points), "label" (description)
   * - Optional: "param1", "param2" (additional parameters for custom calculations during simulation)
   *
   * **Link Structure:**
   * - Required: "id", "src", "dst", "length"
   * - Single fiber: "slots" field
   * - Multi fiber: "fibers" array
   *
   * **Fiber Type Detection:**
   * - **Single fiber**: Link-level "type" field overrides automatic detection
   * - **Multi fiber**: Type must be specified individually for each fiber in "fibers" array
   * - **Warning**: Link-level "type" in multi-fiber configs is ignored with console warning
   * - If "type" field is omitted, type is auto-detected from slots structure
   * - Supported type values: "SSMF", "FMF", "MCF", "FMMCF", "HCF"
   * - Default type detection: SSMF (single core, mode, band)
   *
   * @param filename Path to the JSON configuration file
   *
   * @code{.json}
   * {
   *   "name": "Mixed Network Example",
   *   "nodes": [
   *     { "id": 0, "label": "Node A", "dcs": 2, "ixps": 1 },
   *     { "id": 1, "label": "Node B" },
   *     { "id": 2, "label": "Node C", "param1": 10.5, "param2": -3.2 },
   *     { "id": 3, "label": "Node D", "dcs": 4, "ixps": 2, "param1": 5.0, "param2": 0.0 }
   *   ],
   *   "links": [
   *     {
   *       "id": 0, "src": 0, "dst": 1, "length": 100.0,
   *       "slots": 320,
   *       "type": "SSMF"
   *     },
   *     {
   *       "id": 1, "src": 1, "dst": 2, "length": 250.0,
   *       "slots": [80, 80, 90],
   *       "comment": "This is a multi-core fiber with 3 cores"
   *     },
   *     {
   *       "id": 2, "src": 0, "dst": 2, "length": 150.0,
   *       "slots": [[80, 60], [90, 70]]
   *     },
   *     {
   *       "id": 3, "src": 1, "dst": 2, "length": 200.0,
   *       "slots": {"C": [[100, 80], [90, 70]], "L": [[80, 60], [70, 50]]}
   *     },
   *     {
   *       "id": 4, "src": 0, "dst": 1, "length": 120.0,
   *       "fibers": [
   *         { "slots": 400, "type": "SSMF" },
   *         { "slots": [100, 90, 80], "type": "MCF" },
   *         { "slots": {"C": 320, "L": 280} }
   *       ]
   *     }
   *   ]
   * }
   * @endcode
   *
   * **Single Fiber Configurations:**
   * - **SSMF**: `"slots": 320` → Standard Single-Mode Fiber (1 core, 1 mode)
   * - **FMF**: `"slots": [[80, 60, 40]]` → Few-Mode Fiber (1 core, 3 modes)
   * - **MCF**: `"slots": [80, 90, 70]` → Multi-Core Fiber (3 cores, 1 mode each)
   * - **FMMCF**: `"slots": [[80, 60], [90, 70]]` → Few-Mode Multi-Core Fiber (2 cores, 2 modes each)
   * - **Multi-Band SSMF**: `"slots": {"C": 320, "L": 240}` → Multi-Band Single-Mode (detected as SSMF)
   * - **Multi-Band MCF**: `"slots": {"C": [100, 90], "L": [80, 70]}` → Multi-Band Multi-Core (detected as MCF)
   * - **Multi-Band FMF**: `"slots": {"C": [[80, 60, 40]], "L": [[70, 50, 30]]}` → Multi-Band Few-Mode (detected as FMF)
   * - **Multi-Band FMMCF**: `"slots": {"C": [[100, 80], [90, 70]], "L": [[80, 60], [70, 50]]}` → Multi-Band Few-Mode Multi-Core (detected as FMMCF)
   *
   * **Multi Fiber Configurations:**
   * - `"fibers": [fiber1, fiber2, ...]` → Array of fiber objects
   * - Each fiber can have different slot configurations and types
   * - Mix of fiber types in same link is supported
   */
  Network(std::string filename);
  /**
   * @brief
   *
   */
  Network(const Network &net);
  /**
   * @brief Adds a new Node object to the Network object. To add a new Node to a
   * Network, the new Node's Id must match the amount of nodes that were already
   * on the network.
   *
   * @param node the pointer to the Node desired to be added into the Network
   * object.
   */
  void addNode(std::unique_ptr<Node> node);
  /**
   * @brief Adds a new Link object to the Network object. To add a new Link to a
   * Network, the new Link's Id must match the amount of links that were already
   * on the network.
   *
   * @param link the pointer to the Link desired to be added into the Network
   * object.
   */
  void addLink(std::shared_ptr<Link> link);

  /**
   * @brief Parses a single fiber configuration from JSON and creates a Fiber object.
   *
   * This method handles various JSON fiber configurations and transforms them into
   * a unified Fiber object using the comprehensive bandSlotMatrix constructor.
   * All configurations are normalized to the multi-dimensional format internally.
   *
   * **Supported JSON Formats:**
   *
   * **Simple SSMF (Single-mode):**
   * @code{.json}
   * "slots": 320
   * @endcode
   * Creates a single-core, single-mode fiber with 320 slots in C-band.
   *
   * **Explicit Type Override:**
   * @code{.json}
   * { "slots": [80, 90, 70], "type": "HCF" }
   * @endcode
   * Creates a 3-core fiber but explicitly sets type as HCF instead of auto-detected MCF.
   *
   * **MCF (Multi-core):**
   * @code{.json}
   * "slots": [80, 90, 70]
   * @endcode
   * Creates a 3-core fiber with different slot counts per core, single mode each.
   *
   * **FMMCF (Few-mode Multi-core):**
   * @code{.json}
   * "slots": [[80, 60], [90, 70], [85, 65]]
   * @endcode
   * Creates a 3-core fiber where each core has 2 modes with different slot counts.
   *
   * **Multi-band SSMF:**
   * @code{.json}
   * "slots": {"C": 320, "L": 240, "S": 280}
   * @endcode
   * Creates a single-core, single-mode fiber across multiple bands.
   *
   * **Multi-band MCF:**
   * @code{.json}
   * "slots": {"C": [100, 90, 80], "L": [80, 70, 60]}
   * @endcode
   * Creates a 3-core fiber across C and L bands with different slot counts.
   *
   * **Multi-band FMMCF:**
   * @code{.json}
   * "slots": {
   *   "C": [[100, 80], [90, 70]],
   *   "L": [[80, 60], [70, 50]]
   * }
   * @endcode
   * Creates a 2-core, 2-mode fiber across C and L bands.
   *
   * @param linkConfig JSON object containing the fiber configuration with "slots" field and optional "type" field
   * @return std::shared_ptr<Fiber> Pointer to the created Fiber object
   * @throws std::runtime_error If the JSON format is not recognized, invalid, or contains unknown fiber type
   *
   * @note All JSON patterns are internally transformed to use the bandSlotMatrix
   *       constructor format: `std::map<Band, std::vector<std::vector<int>>>`
   * @note Physical constraint: All bands must have the same number of cores
   * @note **Type Detection**: If "type" field is provided, it overrides auto-detection.
   *       Supported values: "SSMF", "FMF", "MCF", "FMMCF", "HCF"
   * @note **Auto-detection**: When "type" is omitted, type is detected from slot structure
   */
  std::shared_ptr<Fiber> readSingleFiber(const nlohmann::json &linkConfig);

  /**
   * @brief Parses multiple fiber configurations from a JSON array.
   *
   * This method processes an array of fiber configurations, where each element
   * follows the same format as accepted by readSingleFiber(). It creates a
   * vector of Fiber objects that can be used for multi-fiber links.
   *
   * **Usage Example:**
   * @code{.json}
   * "fibers": [
   *   { "slots": 320 },
   *   { "slots": [80, 90, 70] },
   *   { "slots": {"C": 400, "L": 320} },
   *   { "slots": [[100, 80], [90, 70]] }
   * ]
   * @endcode
   *
   * This creates a vector with 4 different fiber types:
   * - SSMF with 320 slots
   * - MCF with 3 cores
   * - Multi-band SSMF
   * - FMMCF with 2 cores and 2 modes each
   *
   * @param fibersData JSON array where each element is a fiber configuration object
   * @return std::vector<std::shared_ptr<Fiber>> Vector of Fiber objects
   * @throws std::runtime_error If any fiber configuration is invalid (propagated from readSingleFiber)
   *
   * @see readSingleFiber() for supported fiber configuration formats
   * @note Each fiber in the array can have a different type and configuration
   * @note Used internally by Network constructor for multi-fiber link parsing
   * @warning Any link-level "type" field is ignored when "fibers" array is used.
   *          A console warning is issued to alert users of this behavior.
   */
  std::vector<std::shared_ptr<Fiber>> readMultiFiber(const nlohmann::json &fibersData);

  /**
   * @brief Returns a unique pointer reference to the Node with the specified ID.
   *
   * @param id The ID of the Node to retrieve.
   * @return std::unique_ptr<Node>& Reference to the unique pointer of the Node object.
   * @throws std::invalid_argument if the ID is invalid or out of bounds.
   */
  std::unique_ptr<Node> &getNode(int id);
  
  /**
   * @brief Returns a unique pointer reference to the Node with the specified label.
   *
   * @param label The label of the Node to retrieve.
   * @return std::unique_ptr<Node>& Reference to the unique pointer of the Node object.
   * @throws std::invalid_argument if no node with the given label is found.
   */
  std::unique_ptr<Node> &getNode(std::string label);
  /**
   * @brief Returns a shared pointer to the Link with the specified ID.
   *
   * @param id The ID of the Link to retrieve.
   * @return std::shared_ptr<Link> Pointer to the Link object, or nullptr if not found.
   */
  std::shared_ptr<Link> getLink(int id);
  /**
   * @brief The Connect methods establishes an Optical Fiber connection between
   * two Nodes through a Link inside the Network object. The different
   * connections between the different Links and Nodes of the Network build up
   * the Network's architecture.
   *
   * To connect the two Nodes through a Link, both Link
   * and (the 2) Nodes must already exist inside the Network object, that is,
   * they need to have been added previously.
   *
   * @param src the Id/position of the source node of the connection.
   * @param id the Id/position of the Link used to connect the nodes.
   * @param dst the Id/position of the destination node of the connection.
   */
  void connect(int src, int id, int dst);
  /**
   * @brief Return all link IDs that connect the given source and destination nodes.
   *
   * @param src Source node index.
   * @param dst Destination node index.
   * @return std::vector<int> containing the IDs of matching links (empty if none).
   */
  std::vector<int> isConnected(int src, int dst) const;

/**
 * @brief Computes k-shortest paths for all node pairs using Yen's algorithm.
 * 
 * This method calculates the k-shortest paths between all pairs of nodes in the network
 * using Yen's algorithm. The algorithm works by:
 * 1. Finding the shortest path between each pair of nodes (using Dijkstra)
 * 2. Iteratively finding the next shortest path by deviating from previously found paths
 * 3. Storing results in the paths data structure for fast lookup during routing
 * 
 * The results are stored as shared pointers to Link objects, forming routes that can be
 * used by the allocator during connection establishment.
 * 
 * @param k Number of shortest paths to compute per node pair
 * 
 * @note Time complexity: O(kN^2(M + N log N)) where N is nodes and M is links
 * @note Space complexity: O(N^2 * k * avgPathLength)
 * @note ShortestPathResult is an internal structure used by Yen's algorithm that stores
 *       paths as vectors of link IDs. We convert these to Route objects (vectors of Link pointers)
 *       for efficient use during connection allocation.
 */
  void setPaths(int k);

/**
 * @brief Loads paths from a JSON file without computing or inferring reverse routes.
 * 
 * This method reads precomputed routes from a JSON file. Each route entry in the file
 * specifies a source, destination, and the explicit paths between them. The reverse
 * direction must be explicitly declared in the file - this method does NOT automatically
 * create reverse routes.
 * 
 * Being explicit (one direction per entry) avoids confusion and ensures that routing
 * behavior matches exactly what is specified in the configuration file.
 * 
 * @param filename Path to JSON file containing routes
 * 
 * Expected JSON format:
 * @code{.json}
 * {
 *   "routes": [
 *     { "src": 0, "dst": 1, "paths": [[0, 2], [4, 5]] },
 *     { "src": 1, "dst": 0, "paths": [[1, 3], [6, 7]] }
 *   ]
 * }
 * @endcode
 * 
 * @note Each entry defines paths in ONE direction only
 * @note Reverse paths must be explicitly declared in separate entries
 * @note This ensures clarity and avoids assumptions about bidirectional routing
 */
  void setPaths(const std::string& filename);

  /**
   * @brief Retrieve the cached paths structure.
   * @return Paths* Raw pointer to the paths (owned by Network), or nullptr if not set.
   */
  Paths* getPaths() const;

  /**
   * @brief Clear any cached paths.
   */
  void clearPaths();

  /**
   * @brief Return the currently cached path multiplicity (max K).
   */
  int getPathK() const;

  int getNumberOfLinks(void) const;
  std::vector<std::shared_ptr<Link>> getLinks(void) const;
  std::shared_ptr<Link> getLink(int src, int dst) const;

  int getNumberOfNodes(void) const;
  std::vector<const Node *> getNodes(void) const;
  std::string getName() const;

  /**
   * @brief Exports the network topology to a JSON file named "network_export.json" in the current directory.
   * 
   * The exported JSON contains nodes and links arrays matching the format used by the Network(filename) constructor.
   */
  void networkToJson() const;

  /**
   * @brief Exports the computed routes to a JSON file named "routes_export.json" in the current directory.
   * 
   * The exported JSON contains the routes array with source, destination, and paths for each node pair.
   * @throws std::runtime_error if no paths have been computed yet (paths is null or empty).
   */
  void routesToJson() const;

  void useSlots(int idLink, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo, int connectionId);
  void unuseSlots(int idLink, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo);
  int isSlotUsed(int idLink, int fiber, int core, fns::Band band, int mode, int slotPos);

  // METRICS (TODO)
  float averageNeighborhood();
  float normalAverageNeighborhood();
  float nodalVariance();

private:
  std::string name;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::shared_ptr<Link>> links;
  std::vector<std::shared_ptr<Link>> linksIn;
  std::vector<std::shared_ptr<Link>> linksOut;
  std::vector<int> nodesIn;
  std::vector<int> nodesOut;
  std::unique_ptr<Paths> paths;
  int pathK = 0;
  int linkCounter;
  int nodeCounter;

  /**
   * @brief Internal structure for storing path computation results during Yen's algorithm.
   * 
   * This structure is used internally by yenKShortestPaths() and dijkstra() methods
   * to represent paths as vectors of node and link IDs. It's an intermediate format
   * that is more efficient for path computation algorithms.
   * 
   * These results are later converted to Route objects (vectors of Link shared_ptr)
   * for storage in the paths data structure, which is what external code uses.
   * 
   * @note We use link IDs instead of Link pointers during computation to allow
   *       efficient path comparison and deduplication using hash sets.
   */
  struct ShortestPathResult {
    std::vector<int> nodePath;     ///< Sequence of node IDs in the path
    std::vector<int> linkPath;     ///< Sequence of link IDs in the path
    double totalLength = std::numeric_limits<double>::infinity();  ///< Total path length
    bool empty() const { return linkPath.empty(); }  ///< Check if path is empty/invalid
  };

  /**
   * @brief Hash function for vectors of integers, used in Yen's algorithm.
   * 
   * This custom hash function allows us to use std::unordered_set with vectors
   * of link IDs. It's used in yenKShortestPaths() to track which paths we've
   * already found and avoid duplicate paths.
   * 
   * Without this, we couldn't use std::unordered_set<std::vector<int>> because
   * the standard library doesn't provide a default hash for vectors.
   * 
   * The hash combines the vector size with each element using a standard
   * hash-combining formula to minimize collisions.
   */
  struct VecHash {
    size_t operator()(const std::vector<int>& v) const {
      size_t seed = v.size();
      for (int i : v) {
        // Standard hash-combining formula
        seed ^= static_cast<size_t>(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };

  void validateAux(int idLink, int fiber, int core, int mode, int slotPos);
  void validateAux(int idLink, int fiber, int core, int mode, int slotFrom, int slotTo);
  std::vector<ShortestPathResult> yenKShortestPaths(int src, int dst, int k) const;
  ShortestPathResult dijkstra(int src,
                              int dst,
                              const std::unordered_set<int>& excludedLinks = {},
                              const std::unordered_set<int>& excludedNodes = {}) const;
};
