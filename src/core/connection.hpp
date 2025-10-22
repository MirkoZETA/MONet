#ifndef __CONNECTION_H__
#define __CONNECTION_H__

// STL
#include <vector>
// core
#include "bitrate.hpp"
// util
#include "../util/utils.hpp"

/**
 * @brief Class with the connection information.
 *
 * This class contains the information regarding the connections that are made
 * between the nodes on a network during the allocation process.
 *
 */
class Connection {
 public:

  Connection(void);
  Connection(std::shared_ptr<const BitRate> bitRate, int src, int dst);
  Connection(int id, double time, std::shared_ptr<const BitRate> bitRate, bool _isAllocatedInP2P, int src, int dst);

  // Link management
  void addLink(int idLink, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo);
  void addLink(int idLink, int fiber, int core, fns::Band band, int mode, std::vector<int> slots);
  void addLink(std::shared_ptr<Link> link, int fiber, int core, fns::Band band, int mode, int slotFrom, int slotTo);

  // P2P management
  bool isAllocatedInP2P() const;
  void setAllocatedInP2P(bool value);
  void changeBitRate(std::shared_ptr<const BitRate> newBitRate);

  // Attributes
  void setId(int id);
  void setTime(double timeConnection);

  int getSrc(void) const;
  int getDst(void) const;
  int getId(void) const;
  double getTimeConnection(void) const;
  std::shared_ptr<const BitRate> getBitrate(void) const;

  const std::vector<int>& getLinks(void) const;
  const std::vector<int>& getFibers(void) const;
  const std::vector<int>& getCores(void) const;
  const std::vector<fns::Band>& getBands(void) const;
  const std::vector<int>& getModes(void) const;
  const std::vector<std::vector<int>>& getSlots(void) const;

 private:
  int id;
  int src;
  int dst;
  double timeConnection;
  std::shared_ptr<const BitRate> bitRate;
  bool _isAllocatedInP2P;

  std::vector<int> links;  // Link IDs
  std::vector<int> fibers; // Fiber index on the link
  std::vector<int> cores;  // Core index in the fiber
  std::vector<fns::Band> bands; // Band in the core (C, L, S, etc.)
  std::vector<int> modes;  // Mode index in the band
  std::vector<std::vector<int>> slots;  // Slots per link

  friend class Controller;
};
#endif