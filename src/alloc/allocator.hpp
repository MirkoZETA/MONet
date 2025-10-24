#pragma once

// STL
#include <memory>
#include <string>
// core
#include "../core/bitrate.hpp"
#include "../core/connection.hpp"
#include "../core/network.hpp"
#include "../core/demand.hpp"

/**
 * @brief Class with the allocator information.
 *
 * The Allocator class handles the assignment of connections inside a Network.
 * This class must be inherited by another class, in which a method to allocate
 * resources has to be implemented.
 *
 * The Allocator class consists of a Network object, a Path vector to the Links
 * inside said Network and a Name attribute and methods for setting them.
 */
class Allocator {
public:
  /**
   * @brief Constructs a new Allocator object that represents the default
   * initialization of an Allocator invoked by calling the void constructor.
   This
   * constructor sets all the information about the Allocator's Networks and
   Paths as
   * empty so the Allocator is ready to be built from a clean state.

   */
  Allocator(void);
  /**
   * @brief Constructs a new Allocator object that represents the
   * initialization of an Allocator with a given (already existing) Network
   * to be set as the Allocator's own Network.
   *
   * @param network the pointer to the Network object that will be set as the
   * Allocator's own Network.
   */
  Allocator(std::shared_ptr<Network> network);
  /**
   * @brief Destroys the Allocator object.
   *
   */
  virtual ~Allocator() = default;


  /**
   * @brief Allocate resources inside the Network managed by the Allocator.
   *        This method must be implemented in an inherited class (e.g. Controller).
   *
   * @param demands     NxN matrix of Demand objects representing traffic requirements.
   * @param bitRates    Available bitrate options for connection establishment.
   * @param connections Output vector to be filled with newly created Connection
   *                    objects. Each Connection is returned as a std::unique_ptr,
   *                    transferring ownership to the caller.
   */
  virtual void exec(
      Network &network,
      std::vector<std::vector<Demand>>& _demands,
      const std::vector<std::shared_ptr<BitRate>>& _bitRates,
      const std::vector<std::unique_ptr<Connection>>& _connections,
      std::vector<std::unique_ptr<Connection>>& _newConnections
  );
  /**
   * @brief Allocates the given connection in the network.
   *
   * @param connection The connection to be allocated.
   */
  void alloc(Connection connection);


  /**
   * @brief Set the Allocator's Network as the one given by the pointer of an
   * already existing Network object.
   *
   * @param network the pointer to the Network object that will be set as the
   * Allocator's own Network.
   */
  void setNetwork(std::shared_ptr<Network> network);
  /**
   * @brief Get the Allocator's Network.
   *
   * @return std::shared_ptr<Network> the Allocator's Network.
   */
  std::shared_ptr<Network> getNetwork(void);
  /**
   * @brief Get the name attribute of the allocator object.
   *
   * @return std::string the name of the allocator.
   */
  std::string getName(void) const;

protected:
  /**
   * @brief A pointer to the Network object that the Allocator will generate
   * connections on.
   */
  std::shared_ptr<Network> network;
  /**
   * @brief The Name of the allocation algorithm.
   */
  std::string name;
};
