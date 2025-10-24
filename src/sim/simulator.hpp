#pragma once

// STL
#include <chrono>
#include <iomanip>
#include <list>
#include <filesystem>
#include <memory>
// math
#include "../math/normal_variable.hpp"
#include "../math/uniform_variable.hpp"
// sim
#include "event.hpp"
#include "demand.hpp"
#include "controller.hpp"
// util
#include "../util/utils.hpp"
#include "../util/version.hpp"
#include "../util/json.hpp"
#include "../util/macros.hpp"

// TODO REVISAR:
#include <fstream>
#include <cstdlib>
#include <limits>
#include <sys/stat.h>

// TODO DOCUMENTATION

/**
 * @brief Class Simulator, represents network execution.
 */
class Simulator
{
public:
  /**
   * @brief Construct a new Simulator object.
   */
  Simulator(void);

  /**
   * @brief
   *
   */
  Simulator(std::string networkFilename, std::string pathsFilename);

  /**
   * @brief
   */
  Simulator(std::string networkFilename, std::string pathsFilename,
            std::string bitratesFilename);

  /**
   * @brief
   */
  Simulator(std::string networkFilename, std::string pathsFilename,
            std::string bitratesFilename, std::string demandsFilename);

  /**
   * @brief Construct a new Simulator object using k-shortest paths with default k=3.
   * Uses Yen's algorithm to compute multiple paths between all node pairs.
   * @param networkFilename Path to the network topology file
   */
  Simulator(std::string networkFilename);

  /**
   * @brief Construct a new Simulator object using k-shortest paths with specified k.
   * Uses Yen's algorithm to compute k shortest paths between all node pairs.
   * @param networkFilename Path to the network topology file
   * @param k Number of shortest paths to compute for each node pair
   */
  Simulator(std::string networkFilename, int k);

  /**
   * @brief Construct a new Simulator object using k-shortest paths with specified k.
   * Uses Yen's algorithm to compute k shortest paths between all node pairs.
   * @param networkFilename Path to the network topology file
   * @param bitratesFilename Path to the bitrates definition file
   * @param k Number of shortest paths to compute for each node pair
   */
  Simulator(std::string networkFilename, std::string bitratesFilename, int k);

  /**
   * @brief Construct a new Simulator object using k-shortest paths with specified k.
   * Uses Yen's algorithm to compute k shortest paths between all node pairs.
   * @param networkFilename Path to the network topology file
   * @param bitratesFilename Path to the bitrates definition file
   * @param demandsFilename Path to the initial demands definition file
   * @param k Number of shortest paths to compute for each node pair
   */
  Simulator(std::string networkFilename, std::string bitratesFilename,
            std::string demandsFilename, int k);

  /**
   * @brief Saves the simulator processes to run and leaves them ready to start,
   * does not start them.
   */
  void init(void);

  /**
   * @brief Start the simulator processes.
   */
  void run(void);

  /**
   * @brief
   */
  void run(bool highVerbose);

  /**
   * @brief
   *
   * @param newAllocator Pointer of object type Allocator.
   */
  void setAllocator(std::unique_ptr<Allocator> newAllocator);
  /**
   * @brief
   */
  void setCallbackFunction(
    void (*callbackFunction)(
      Network& network,
      std::vector<std::vector<Demand>>& demands,
      std::vector<std::unique_ptr<Connection>>& connections,
      double time));

  /**
   * @brief Set the failure management function.
   * This function will be called in case of a failure in the simulation.
   *
   * @param failureManagementFunction Pointer to the failure management function.
   */
  void setFailureManagementFunction(
    void (*failureManagementFunction)(
      Network& network,
      std::vector<std::vector<Demand>>& demands,
      std::vector<std::unique_ptr<Connection>>& connectionsAffected,
      eventType eventType,
      double time));

  // SETTERS USER LEVEL:
  /**
   * @brief Sets the number of periods to simulate.
   *
   * @param numberOfPeriods
   */
  void setNumberOfPeriods(int numberOfPeriods);
  /**
   * @brief
   */
  void setBaseGrowthRate(double baseGrowthRate);
  /**
   * @brief
   */
  void setGrowthRateStdDev(double growthRateStdDev);
  /**
   * @brief Sets the seed for the growth factor in INCREMENTAL MODE.
   *
   * @param seed the unsigned int that represent the new seed for the growth factor.
   */
  void setSeedGrowthRate(unsigned int seedGrowthRate);

  // GETTERS USER LEVEL
  /**
   * @brief
   */
  int getNumberOfPeriods() const;

  /**
   * @brief Get the current time.
   */
  int getCurrentPeriod() const;
  /**
   * @brief Get the Base Growth Rate
   */
  double getBaseGrowthRate() const;
  /**
   * @brief Get the Base Growth Rate Standard Deviation
   */
  double getGrowthRateStdDev() const;
  /**
   * @brief
   */
  std::vector<double> getGrowthRates() const;
  /**
   * @brief
   */
  std::unique_ptr<Controller> &getController();
  /**
   * @brief Get the Time Duration object, that corresponds to the simulation
   * time.
   *
   * @return Unsigned int the number of seconds that the simulation was running.
   */
  unsigned int getTimeDuration();

  // UTIL METHODS
  /**
   * @brief Get the Demands matrix.
   */
  std::vector<std::vector<Demand>> &getDemands();
  /**
   * @brief Get the Demand object for a specific source and destination.
   *
   * @param src The source node ID.
   * @param dst The destination node ID.
   * @return Demand* Pointer to the Demand object, or nullptr if not found.
   */
  Demand *getDemand(int src, int dst);
  /**
   * @brief Get the Demand object for a specific ID.
   *
   * @param id The ID of the demand.
   * @return Demand* Pointer to the Demand object, or nullptr if not found.
   */
  Demand *getDemand(int id);
  /**
   * @brief Get the BitRates vector attribute of the Simulator object.
   *
   * @return std::vector<BitRate>
   */
  std::vector<std::shared_ptr<BitRate>> getBitRates(void);
  /**
   * @brief Gets the Paths vector of the network.
   *
   * @return Raw pointer to the paths matrix (owned by Network), or nullptr if not set.
   */
  Paths* getPaths();
  /**
   * @brief
   */
  void setGrowthRates(std::vector<double> growthRates);

private:
  // Core parameters
  std::unique_ptr<Controller> controller;
  std::list<Event> events;
  std::vector<std::shared_ptr<BitRate>> bitRates;
  std::vector<std::vector<Demand>> demands;

  NormalVariable growthVariable;
  double baseGrowthRate;
  double growthRateStdDev;

  // Seteable parameters
  bool initReady;
  unsigned int seedGrowthRate;
  int numberOfPeriods;
  std::string demandsFilename;
  std::vector<double> growthRates;

  // Tracking parameters
  Event currentEvent;
  int currentPeriod;

  // Time related
  std::chrono::high_resolution_clock::time_point startingTime;
  std::chrono::high_resolution_clock::time_point checkTime;
  std::chrono::duration<double> timeDuration;
  double clock;

  // Metrics TODO:
  std::vector<double> totalRequiredTbps;
  std::vector<double> totalProvisionedTbps;
  std::vector<double> totalUnderProvisionedTbps;

  /**
   * @brief
   */
  void eventRoutine(void);
  /**
   * @brief Initializes the demands for the simulation for period 1.
   */
  void initializeDemands(void);
  /**
   * @brief Updates the demands values using the growth factors.
   */
  void updateDemands(void);
  /**
   * @brief Set the default values for the different fields.
   */
  void defaultValues();
  /**
   * @brief Shows on screen the values of the current simulator configuration.
   */
  void printInitialInfo();
  /**
   * @brief
   */
  void printRow(bool highVerbose);
  /**
   * @brief
   */
  void printFinalInfo();
  /**
   * @brief Reads a JSON file and extracts demand information.
   * Supports both node IDs (integers) and node labels (strings).
   * Node labels are resolved using the loaded network topology.
   *
   * @param demandsFilename The name of the JSON file to read
   */
  void readDemandsFile(const std::string &demandsFilename);
};
