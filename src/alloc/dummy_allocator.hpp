#ifndef __DUMMY_ALLOCATOR_H__
#define __DUMMY_ALLOCATOR_H__

// alloc
#include "allocator.hpp"

/**
 * @brief Class "DummyAllocator" which extends class father "Allocator".
 */

class DummyAllocator : public Allocator {
 public:
  /**
   * @brief Constructs a new DummyAllocator object.
   */
  DummyAllocator();
  /**
   * @brief Constructs a new DummyAllocator object and initialize the network.
   *
   * @param network pointer type network representing the network of nodes.
   */
  DummyAllocator(std::shared_ptr<Network> network);
  /**
   * @brief Delete the object DummyAllocator.
   */
  ~DummyAllocator();
  /**
   * @brief The method to allocate resources. It must be implemented on an
   * inherited class.
   *
   * @param demands NxN matrix of demand objects representing traffic requirements
   * @param bitRates Available bitrate options for connection establishment
   * @param connections Vector to fill with the resulting connection specifications
   * @return void
   */
  void exec(
    Network &network,
    std::vector<std::vector<Demand>>& _demands,
    const std::vector<std::shared_ptr<BitRate>>& _bitRates,
    const std::vector<std::unique_ptr<Connection>>& _connections,
    std::vector<std::unique_ptr<Connection>>& _newConnections) override;
};

#endif