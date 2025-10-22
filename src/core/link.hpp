#ifndef __LINK_H__
#define __LINK_H__

// STL
#include <memory>
#include <vector>
// core
#include "fiber.hpp"
// utils
#include "../util/utils.hpp"

/**
 * @brief Class Link
 *
 * The Link class represents a physical connection between two nodes in the network.
 * Each link can have multiple fibers, each with its own configuration (cores, modes, bands, slots).
 * Provides methods to manage fibers and access link attributes.
 */

class Link {
  friend class Network;
 public:
  /**
   * @brief Default constructor for Link.
   *
   * Initializes members to default values:
   * - id = -1
   * - length = DEFAULT_LENGTH
   * - src = -1
   * - dst = -1
   * - fibers empty
   * (defaults are defined in CONSTANTS_HPP).
   */
  Link(void);
  /**
   * @brief Constructor for Link with specified id.
   * Other parameters are set to default values.
   *
   * @param id Identifier for the link.
   */
  Link(int id);
  /**
   * @brief Constructor for Link with specified id and length.
   * Other parameters are set to default values.
   *
   * @param id Identifier for the link.
   * @param length Length of the link (km, must be > 0).
   */
  Link(int id, float length);
  /**
   * @brief Constructor for Link with specified id, length, and single fiber.
   *
   * @param id Identifier for the link.
   * @param length Length of the link (km, must be > 0).
   * @param fiber Shared pointer to the fiber to add to this link.
   */
  Link(int id, float length, std::shared_ptr<Fiber> fiber);
  /**
   * @brief Constructor for Link with specified id, length, and vector of fibers.
   *
   * @param id Identifier for the link.
   * @param length Length of the link (km, must be > 0).
   * @param fibers Vector of shared pointers to fibers to add to this link.
   */
  Link(int id, float length, std::vector<std::shared_ptr<Fiber>> fibers);

  /**
   * @brief Set the Id of a link. This method only works on links that were
   * created without a specified id. That is, objects created with the
   * constructor Link(void).
   *
   * @param id Identifier of the object. Type int. It serves to differentiate
   * the created object from the others.
   */
  void setId(int id);
  /**
   * @brief Set the Length of the link. Must be > 0.
   *
   * @param length Length to set (km).
   * @throws std::invalid_argument if length is not positive.
   */
  void setLength(float length);
  /**
   * @brief Set the source node id for this link.
   *
   * @param src Source node id.
   */
  void setSrc(int src);
  /**
   * @brief Set the destination node id for this link.
   *
   * @param dst Destination node id.
   */
  void setDst(int dst);

  /**
   * @brief Get the id of the Link object.
   *
   * @return int The id of this Link object.
   */
  int getId(void) const;
  /**
   * @brief Get the Length attribute of the Link object.
   *
   * @return float, the Length of this Link object.
   */
  double getLength(void) const;
  /**
   * @brief Get the identifier of the source node for this Link.
   *
   * @return int The source node id.
   */
  int getSrc(void) const;
  /**
   * @brief Get the identifier of the destination node for this Link.
   *
   * @return int The destination node id.
   */
  int getDst(void) const;
  /**
   * @brief Get the current usage percentage of this Link.
   *
   * Calculated as (used slots / total slots) * 100 across all fibers.
   *
   * @return float Usage percentage (0.0 to 100.0).
   */
  float getUsagePercentage(void) const;
  /**
   * @brief Get all fibers in this Link.
   *
   * @return std::vector<std::shared_ptr<Fiber>> Vector of shared pointers to Fiber objects.
   */
  std::vector<std::shared_ptr<Fiber>> getFibers(void) const;
  /**
   * @brief Get a specific fiber by index.
   *
   * @param index Index of the fiber (0-based).
   * @return std::shared_ptr<Fiber> Shared pointer to the Fiber object.
   * @throws std::out_of_range if index is invalid.
   */
  std::shared_ptr<Fiber> getFiber(int index) const;
  /**
   * @brief Get the number of fibers in this Link.
   *
   * @return int Number of fibers.
   */
  int getNumberOfFibers(void) const;

  /**
   * @brief Add a fiber to the link (by shared pointer).
   *
   * @param fiber Shared pointer to Fiber to add.
   */
  void addFiber(std::shared_ptr<Fiber> fiber);

  /**
   * @brief Add a bundle of fibers of the same type to the link.
   *
   * Creates multiple fibers with predefined configurations based on fiber type:
   * - SSMF: 1 core, 1 mode, fns::defaults::SLOTS slots in C-band
   * - MCF: DEFAULT_CORES cores, 7 modes each, fns::defaults::SLOTS slots in C-band  
   * - FMF: 1 core, 6 modes, fns::defaults::SLOTS slots in C-band
   * - FMMCF: 7 cores, 5 modes each, fns::defaults::SLOTS slots in C-band
   *
   * @param type Type of fiber to create (SSMF, MCF, FMF, FMMCF)
   * @param numberOfFibers Number of identical fibers to add to the bundle
   * @throws std::invalid_argument if numberOfFibers <= 0 or unknown fiber type
   */
  void addCable(fns::FiberType type, int numberOfFibers);

 private:
  int id;
  int src;
  int dst;
  float length;

  std::vector<std::shared_ptr<Fiber>> fibers;
};

/**
 * @brief Route type: a vector of shared pointers to Link objects.
 */
using Route = std::vector<std::shared_ptr<Link>>;

/**
 * @brief Paths type: 3D vector of routes between node pairs.
 */
using Paths = std::vector<std::vector<std::vector<Route>>>;

#endif


