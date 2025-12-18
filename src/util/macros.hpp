#ifndef MACROS_HPP
#define MACROS_HPP

// Dependencies for macros
#include "../sim/simulator.hpp"

/**
 *  @brief Begin the definition of an allocator function This is
 *  the main entry point for your allocation logic.
 * 
 *  @param name__ Identifier for the allocator (e.g., FirstFit).
 * 
 */
#define BEGIN_ALLOC_FUNCTION(name__)                                            \
  class f_##name__ : public Allocator                                           \
  {                                                                             \
  public:                                                                       \
    f_##name__() : Allocator() { this->name = std::string(#name__); }           \
    void exec(Network& _network,                                                \
              std::vector<std::vector<Demand>>& _demands,                     \
              const std::vector<std::shared_ptr<BitRate>>& _bitRates,         \
              const std::vector<std::unique_ptr<Connection>>& _connections, \
              std::vector<std::unique_ptr<Connection>>& _newConnections) override
#define END_ALLOC_FUNCTION \
  };
/**
 * @brief Use the defined allocator function in the simulator.
 * 
 * @param fun__ The name of the allocator function defined using
 * @param simObject__ The simulator object where the allocator will be set.
 * 
 */
#define USE_ALLOC_FUNCTION(fun__, simObject__) \
  simObject__.setAllocator(std::make_unique<f_##fun__>())
/**
 * @brief Begin the definition of a callback function. This function is
 * called at the end of each simulation period.
 * 
 * @param name__ Identifier for the callback function (e.g., MyCallback).
 * 
 */
#define BEGIN_CALLBACK_FUNCTION(name__)                                                 \
  void cb_##name__(                                                                     \
      Network& _network,                                                               \
      std::vector<std::vector<Demand>>& _demands,                                       \
      std::vector<std::unique_ptr<Connection>>& _connections,                           \
      double time) {

#define END_CALLBACK_FUNCTION                                                           \
  }
/**
 * @brief Use the defined callback function in the simulator.
 * 
 * @param fun__ The name of the callback function defined using
 * @param simObject__ The simulator object where the callback will be set.
 * 
 */
#define USE_CALLBACK_FUNCTION(fun__, simObject__) \
  (simObject__).setCallbackFunction(&cb_##fun__)



/**
 * @brief Get the current simulation period.
 * 
 * @return int the current simulation period.
 * 
 */
#define GET_CURRENT_PERIOD _period
/**
 * @brief Get the network object.
 * 
 * @return Network& Reference to the network object.
 * 
 */
#define GET_NETWORK _network



/**
 * @brief Get the demands matrix. This is a 2D vector where each element
 * represents a demand from a source node to a destination node.
 * 
 * @return std::vector<std::vector<Demand>>& Reference to the demands matrix.
 * 
 */
#define GET_DEMANDS _demands
/**
 * @brief Get the demand at specific source and destination nodes.
 * 
 * @param _src Source node ID.
 * @param _dst Destination node ID.
 * 
 * @return Demand* Pointer to the Demand object.
 * 
 */
#define GET_DEMAND_AT(_src, _dst) _demands[_src][_dst].get()
/**
 * @brief Iterate over each demand in the demands matrix.
 * 
 */
#define FOR_EACH_DEMAND                                                    \
  for (size_t _i = 0; _i < _demands.size(); ++_i)                      \
    for (size_t _j = 0; _j < _demands[_i].size(); ++_j)                \
      if (auto& _demand = _demands[_i][_j];                            \
          _demand.isNull() || _i == _j)                                    \
        ;                                                                    \
      else
/**
 * 
 * @brief Iterate over each demand in the demands matrix, considering only
 * one direction for symmetric demands (i.e., from lower to higher node IDs).
 * This is useful for simulation networks where demands are symmetric and so
 * is the provisioning.
 * 
 */
#define FOR_EACH_DEMAND_SYMMETRIC                                            \
  for (size_t _i = 0; _i < _demands.size(); ++_i)                      \
    for (size_t _j = 0; _j < _i; ++_j)                                        \
      if (auto& _demand = _demands[_i][_j];                            \
          _demand.isNull() || _i == _j)                                    \
        ;                                                                    \
      else
/**
 * @brief Get the current demand being processed in the allocation loop.
 * This macro is intended to be used within the \c FOR_EACH_DEMAND or
 * \c FOR_EACH_DEMAND_SYMMETRIC loops.
 * 
 * @return Demand& Reference to the current Demand object.
 * 
 */
#define CURRENT_DEMAND _demand
/**
 * @brief Get the mirror demand of the current demand being processed.
 * This macro is intended to be used within the \c FOR_EACH_DEMAND or
 * \c FOR_EACH_DEMAND_SYMMETRIC loops.
 * 
 * @return Demand& Reference to the mirror Demand object.
 * 
 */
#define CURRENT_MIRROR_DEMAND _demands[CURRENT_DEMAND.getDst()][CURRENT_DEMAND.getSrc()]



/**
 * @brief Get the connections vector.
 * 
 * @return std::vector<std::unique_ptr<Connection>>& Reference to the connections vector.
 * 
 */
#define GET_CONNECTIONS _connections
/**
 * @brief Get the new connections vector.
 * 
 * @return std::vector<std::unique_ptr<Connection>>& Reference to the new connections vector.
 * 
 */
#define GET_NEW_CONNECTIONS _newConnections
/**
 * @brief Create a new connection object. This macro creates a new
 * connection with the specified bit rate, source, and destination.
 * Necesary for the ALLOCATE_CONNECTION macro.
 * 
 * @return std::unique_ptr<Connection> Unique pointer to the new Connection object.
 * 
 */
#define NEW_CONNECTION(_bitRate, _src, _dst) \
  std::make_unique<Connection>(_bitRate, _src, _dst)
/**
 * @brief Add a link to a connection. This macro populates a connection with 
 * the specified link information. After populating the connection, you should
 * use the ALLOCATE_CONNECTION macro to allocate it in the network.
 * 
 * @param _connection The connection object to which the link will be added.
 * @param _link The object of ID of the link to be added to the connection.
 * @param _fiberIdx The index of the fiber in the link.
 * @param _coreIdx The index of the core in the fiber.
 * @param _band The band in the core (e.g., Band::C).
 * @param _modeIdx The index of the mode in the band.
 * @param _slotFrom The starting slot index for the connection.
 * @param _slotTo The ending slot index for the connection.
 * 
 */
#define ADD_LINK_TO_CONNECTION(_connection, _link, _fiberIdx, _coreIdx, _band, _modeIdx, _slotFrom, _slotTo) \
  _connection->addLink(_link, _fiberIdx, _coreIdx, _band, _modeIdx, _slotFrom, _slotFrom + _slotTo)
/**
 * @brief Allocate a connection in the network. This macro performs the
 * allocation of a connection by adding it to the connections vector,
 * calling the allocation function to allocate resources in the network,
 * and updating the allocated capacity of the current demand.
 * 
 * @param _newConnection The connection object to be allocated.
 * 
 */
#define ALLOCATE_CONNECTION(_newConnection)                            \
  do {                                                                 \
    const double __br = (_newConnection)->getBitrate()->getBitRate();  \
    this->alloc(*(_newConnection));                                    \
    _newConnections.push_back(std::move(_newConnection));              \
    CURRENT_DEMAND.addAllocatedCapacity(__br);                         \
  } while (0)



/**
 * @brief Get the bitrates vector. This vector holds all the
 * bitrates that are available for allocation.
 * 
 * @return std::vector<std::shared_ptr<BitRate>>& Reference to the bitrates vector.
 * 
 */
#define GET_BITRATES _bitRates
/**
 * @brief Get the bitrate object at a specific index.
 * 
 * @param _idx Index of the bitrate.
 * 
 * @return BitRate& Reference to the bitrate object.
 * 
 */
#define GET_BITRATE_AT(_idx) _bitRates.at(_idx)
/**
 * @brief Get the bitrate value (in Gbps) at a specific index.
 * 
 * @param _idx Index of the bitrate.
 * 
 * @return double Bitrate value in Gbps.
 */
#define GET_BITRATE_VALUE(_idx) _bitRates.at(_idx)->getBitRate()
/**
 * @brief Get the bitrate string (e.g., "100") at a specific index.
 * 
 * @param _idx Index of the bitrate.
 * 
 * @return std::string Bitrate string.
 * 
 */
#define GET_BITRATE_STR(_idx) _bitRates.at(_idx)->getBitRateStr()
/**
 * @brief Get the number of available bitrates. Useful for iteration.
 * 
 * @return size_t Number of available bitrates.
 *
 */
#define GET_NUM_BITRATES _bitRates.size()
/**
 * @brief Get the bitrate object by its numeric value (Gbps).
 * 
 * @param _value The value of the bitrate in Gbps.
 * 
 * @return BitRate& Reference to the bitrate object.
 * 
 */
#define GET_BITRATE_BY_VALUE(_value) \
  GET_BITRATE_AT(GET_BITRATE_IDX_BY_VALUE(_value))
/**
 * @brief Get the index of a bitrate by its numeric value (Gbps).
 * 
 * @param _value The value of the bitrate in Gbps.
 * 
 * @return size_t Index of the bitrate in the bitrates vector.
 */
#define GET_BITRATE_IDX_BY_VALUE(_value)                              \
([&](){                                                                          \
  auto&& __rates = (_bitRates);                                                  \
  auto   __val   = (_value);                                                     \
  for (size_t __i = 0; __i < __rates.size(); ++__i)                              \
    if (std::abs(__rates[__i]->getBitRate() - __val) < 1e-6) return __i;         \
  throw std::runtime_error("BitRate value not found");                           \
}())


/**
 * @brief Get a modulation format of a BitRate.
 *
 * @param _bitRateIdx BitRate index.
 * @param _mfIdx      Modulation index within that BitRate.
 * 
 * @return ModulationFormat (API-dependent).
 * 
 */
#define GET_MF_AT(_bitRateIdx, _mfIdx) GET_BITRATE_AT(_bitRateIdx).getModulationFormat(_mfIdx)
/** @brief Get the number of modulation formats for a BitRate. */
#define GET_NUM_MF(_bitRateIdx) \
  GET_BITRATE_AT(_bitRateIdx).getNumberOfModulations()
/** @brief Get a human-readable modulation label for a BitRate/modulation index. */
#define GET_MF_STR(_bitRateIdx, _mfIdx) \
  GET_BITRATE_AT(_bitRateIdx).getModulationStr(_mfIdx)
/**
 * @brief Pick a distance-adaptive modulation for a route and band.
 *
 * @param _src,_dst   Endpoints.
 * @param _bitrateIdx BitRate index.
 * @param _routeIdx   Candidate route index (within K-shortest list).
 * @param _band       Band to evaluate (e.g., \c fns::Band::C).
 * 
 * @return int Modulation index, or -1 if none is feasible.
 * 
 */
#define GET_BEST_ADAPTIVE_MF(_src, _dst, _bitrateIdx, _routeIdx, _band) \
  GET_BITRATE_AT(_bitrateIdx).get()->getAdaptiveModulation((*this->network->getPaths())[_src][_dst][_routeIdx], _band)
/**
 * @brief Maximum reach (km) for the given BitRate/modulation/band.
 *
 * @return double Distance in kilometers.
 */
#define GET_MF_MAX_REACH(_bitrateIdx, _modulationIdx, _band) \
  GET_BITRATE_AT(_bitrateIdx).getReach(_modulationIdx, _band)



/** @brief Number of candidate routes between \p _src and \p _dst. */
#define GET_NUM_ROUTES(_src, _dst) (*this->network->getPaths())[_src][_dst].size()




/** @brief Shorthand: all link objects from the (copied) network. */
#define GET_LINKS GET_NETWORK.getLinks()
/**
 * @brief Get the link on a route at index.
 *
 * @param _src,_dst   Endpoints.
 * @param _routeIdx   Route index.
 * @param _linkIdx    Index along the route.
 * 
 * @return std::shared_ptr<Link> The link object.
 * 
 */
#define GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx) \
  (*this->network->getPaths())[_src][_dst][_routeIdx][_linkIdx]
#define GET_NUM_LINKS(_src, _dst, route) \
  (*this->network->getPaths())[_src][_dst][route].size()
#define GET_LINK_ID_AT(_src, _dst, route, link) \
  GET_LINK_AT(_src,_dst,route,link)->getId()
/**
 * @brief Iterate over each link in the network.
 * 
 */
#define FOR_EACH_LINK                             \
  for (auto& _link : GET_LINKS)                   \
    if (!_link)                           \
      ;                                           \
    else
/**
 * @brief Get the current link being processed in the link iteration loop.
 * This macro is intended to be used within the \c FOR_EACH_LINK loop.
 * 
 * @return Link& Reference to the current Link object.
 * 
 */
#define CURRENT_LINK (*_link)
#define GET_AVG_LINK_USAGE_PERCENTAGE() \
  ([&](){ \
    float __totalUsage = 0.0f; \
    for (auto& _link : GET_LINKS) { \
      if (_link) __totalUsage += _link->getUsagePercentage(); \
    } \
    return __totalUsage / static_cast<float>(GET_LINKS.size()); \
  }())

// Fiber
#define GET_FIBERS(_src,_dst,_routeIdx,_linkIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx).get()->getFibers()
#define GET_FIBER_AT(_src,_dst,_routeIdx,_linkIdx,_fiberIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx).get()->getFiber(_fiberIdx)
#define GET_NUM_FIBERS(_src, _dst, _routeIdx, _linkIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx).get()->getNumberOfFibers()
#define NEW_FIBER(_bandSlotMatrix) \
  std::make_shared<Fiber>(_bandSlotMatrix)

// Core, Band, Mode
#define GET_NUM_CORES(_src,_dst,_routeIdx,_linkIdx,_fiberIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx)->getFiber(_fiberIdx)->getNumberOfCores()
#define GET_MIN_NUM_CORES(_src, _dst, _routeIdx, _selectedFibersIdx) \
  ([&](){ \
    size_t __minCores = std::numeric_limits<size_t>::max(); \
    for (size_t l = 0; l < GET_NUM_LINKS(_src, _dst, _routeIdx); l++) { \
      __minCores = std::min(__minCores, static_cast<size_t>(GET_NUM_CORES(_src, _dst, _routeIdx, l, _selectedFibersIdx[l]))); \
    } \
    return __minCores; \
  }())
#define GET_BANDS(_src,_dst,_routeIdx,_linkIdx,_fiberIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx)->getFiber(_fiberIdx)->getBands()
#define GET_NUM_BANDS(_src,_dst,_routeIdx,_linkIdx,_fiberIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx)->getFiber(_fiberIdx)->getNumberOfBands()
#define GET_NUM_MODES(_src,_dst,_routeIdx,_linkIdx,_fiberIdx,_coreIdx,_band) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx)->getFiber(_fiberIdx)->getNumberOfModes(_coreIdx, _band)
#define GET_MIN_NUM_MODES(_src, _dst, _routeIdx, _selectedFibersIdx, _coreIdx, _band) \
  ([&](){ \
    size_t __minModes = std::numeric_limits<size_t>::max(); \
    for (size_t l = 0; l < GET_NUM_LINKS(_src, _dst, _routeIdx); l++) { \
      __minModes = std::min(__minModes, static_cast<size_t>(GET_NUM_MODES(_src, _dst, _routeIdx, l, _selectedFibersIdx[l], _coreIdx, _band))); \
    } \
    return __minModes; \
  }())

// Slots
#define GET_SLOTS(_src,_dst,_routeIdx,_linkIdx,_fiberIdx,_coreIdx,_band,_modeIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx)->getFiber(_fiberIdx)->getSlots(_coreIdx, _band, _modeIdx)
#define GET_NUM_SLOTS(_src,_dst,_routeIdx,_linkIdx,_fiberIdx,_coreIdx,_band,_modeIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx)->getFiber(_fiberIdx)->getNumberOfSlots(_coreIdx, _band, _modeIdx)
#define GET_SLOT(_src,_dst,_routeIdx,_linkIdx,_fiberIdx,_coreIdx,_band,_modeIdx,_slotIdx) \
  GET_LINK_AT(_src,_dst,_routeIdx,_linkIdx)->getFiber(_fiberIdx)->getSlot(_coreIdx, _band, _modeIdx, _slotIdx)
#define GET_REQ_SLOTS(_bitrateIdx, _modulationIdx, _band) \
  GET_BITRATE_AT(_bitrateIdx).get()->getRequiredSlots(_modulationIdx, _band)
#define GET_MIN_NUM_SLOTS(_src, _dst, _routeIdx, _selectedFibersIdx, _coreIdx, _band, _modeIdx) \
  ([&](){ \
    size_t __minSlots = std::numeric_limits<size_t>::max(); \
    for (size_t l = 0; l < GET_NUM_LINKS(_src, _dst, _routeIdx); l++) { \
      __minSlots = std::min(__minSlots, static_cast<size_t>(GET_NUM_SLOTS(_src, _dst, _routeIdx, l, _selectedFibersIdx[l], _coreIdx, _band, _modeIdx))); \
    } \
    return __minSlots; \
  }())

#endif // MACROS_HPP
