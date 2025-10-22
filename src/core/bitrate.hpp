#pragma once

// STL
#include <string>
#include <vector>
#include <fstream>
// core
#include "link.hpp"
#include "modulation_format.hpp"
// util
#include "../util/utils.hpp"
#include "../util/json.hpp"

/**
 * @brief Class BitRate
 *
 * The BitRate class is used to represent the Bit Rate, which is the amount of
 * data transmitted/processed (in bits) per unit of time inside a digital
 * transmission system or a computer network (and, for simulation purposes,
 * inside the Simulator (class) itself.)
 *
 * A BitRate of certain magnitude can be represented in different modulation
 * formats.
 *
 * The BitRate class contains the bit rate itself (as a magnitude/quantity in Gbps) 
 * represented as a double (bitRate) and its stringified equivalent (bitRateStr), 
 * along with a vector of ModulationFormat objects that represent the available 
 * modulation formats for this bit rate.
 *
 * The BitRate class contains several methods: getters for all its attributes,
 * a method to add a new modulation format and another to get BitRate objects
 * vector filled with information from a given JSON file.
 */
class BitRate {

 public:
  /**
   * @brief Construct a new BitRate object using the given double for setting
   * the object's bit rate magnitude and its respective string.
   *
   * @param bitRate (double): The desired bit rate magnitude.
   */
  BitRate(double bitRate);
  /**
   * @brief Adds a new modulation format to the current BitRate object.
   *
   * @param modulationFormat The ModulationFormat object to add.
   */
  void addModulationFormat(const ModulationFormat& modulationFormat);

  /**
   * @brief Adds a new modulation to the current BitRate object, given its
   * name, slots numbers and reach distance.
   *
   * @param modulation (std::string): The name of the desired modulation.
   * @param slotsPerBand Map of required slots per band for the modulation.
   * @param reachPerBand Map of maximum reach distance per band for this modulation.
   */
  void addModulation(const std::string& modulation, 
                     const std::map<fns::Band, int>& slotsPerBand, 
                     const std::map<fns::Band, double>& reachPerBand);
  /**
   * @brief Gets the modulation format at a given position.
   *
   * @param modulationPos (int): The position index for the desired modulation.
   *
   * @return ModulationFormat: The modulation format at the specified position.
   */
  ModulationFormat getModulationFormat(int modulationPos) const;

  /**
   * @brief Gets the modulation format by its name.
   *
   * @param modulation (std::string): The name of the desired modulation.
   *
   * @return ModulationFormat: The modulation format with the specified name.
   */
  ModulationFormat getModulationFormat(const std::string& modulation) const;

  /**
   * @brief Gets the modulation name in a given position (argument) inside the
   * modulation formats vector.
   *
   * @param modulationPos (int): The position index for the desired modulation.
   *
   * @return std::string: The modulation name of the desired modulation.
   */
  std::string getModulationStr(int modulationPos) const;
  /**
   * @brief Gets the optimal modulation for a given route following the distance adaptive 
   * modulation scheme. Uses C band by default.
   *
   * Selection criteria (in priority order):
   * 1. Must meet distance requirement (reach >= route length)  
   * 2. Minimum number of slots (highest spectral efficiency)
   * 3. Maximum reach (highest reliability for same slot count)
   *
   * @param route The route to be evaluated.
   * @return (int): The index of the optimal modulation for the given route or -1 if no 
   * suitable modulation is found.
   */
  int getAdaptiveModulation(const Route &route) const;

  /**
   * @brief Gets the optimal modulation for a given route following the distance adaptive 
   * modulation scheme.
   *
   * Selection criteria (in priority order):
   * 1. Must meet distance requirement (reach >= route length)
   * 2. Minimum number of slots (highest spectral efficiency)  
   * 3. Maximum reach (highest reliability for same slot count)
   *
   * @param route The route to be evaluated.
   * @param band The band for which to evaluate modulations.
   * @return (int): The index of the optimal modulation for the given route or -1 if no 
   * suitable modulation is found.
   */
  int getAdaptiveModulation(const Route &route, fns::Band band) const;

  /**
   * @brief Gets the required slots for a given modulation. Assumes C band.
   * 
   * @param modulationPos (int): The position index for the desired modulation.
   *
   * @return int: The number of required slots for the desired modulation.
   */
  int getRequiredSlots(int modulationPos) const;

  /**
   * @brief Gets the required slots for a given modulation and band.
   *
   * @param modulationPos (int): The position index for the desired modulation.
   * @param band (Band): The band for which the required slots are to be 
   * retrieved.
   *
   * @return int: The number of required slots for the desired modulation and 
   * band.
   */
  int getRequiredSlots(int modulationPos, fns::Band band) const;

  /**
   * @brief Gets the maximum reach distance for a given modulation. Assumes C band.
   *
   * @param modulationPos (int): The position index for the desired modulation.
   *
   * @return double: The maximum reach distance of the desired modulation.
   */
  double getReach(int modulationPos) const;

  /**
   * @brief Gets the maximum reach distance for a given modulation and band.
   *
   * @param modulationPos (int): The position index for the desired modulation.
   * @param band (Band): The band for which the reach is to be retrieved.
   *
   * @return double: The maximum reach distance of the desired modulation and 
   * band.
   */
  double getReach(int modulationPos, fns::Band band) const;

  /**
   * @brief Gets the bit rate magnitude (quantity) in string format, 
   * i.e. "10.0" (Gbps).
   *
   * @return std::string: The bit rate of the current BitRate object in string
   * format.
   */
  std::string getBitRateStr() const;

  /**
   * @brief Gets the number of modulation formats available in the current object
   *
   * @return int: The number of modulation formats in the current BitRate object.
   */
  int getNumberOfModulations() const;

  /**
   * @brief Gets the bit rate magnitude (quantity) in numerical (double) format,
   * i.e. 10.0 (Gbps).
   *
   * @return double: The bit rate magnitude of the current BitRate object.
   */
  double getBitRate() const;

  /**
   * @brief Reads a JSON file and automatically detects the structure to fill
   * a vector of BitRate objects.
   *
   * The method supports two JSON formats:
   *
   * 1. Simple format (single band, C band assumed):
   * \code{.json}
   * {
   *     "10": [
   *         {
   *             "BPSK": {
   *                 "slots": 2,
   *                 "reach": 14400
   *             }
   *         }
   *     ]
   * }
   * \endcode
   *
   * 2. Multi-band format:
   * \code{.json}
   * {
   *     "40": [
   *         {
   *             "BPSK": [
   *                 {
   *                     "C": {
   *                         "slots": 1,
   *                         "reach": 19700
   *                     },
   *                     "L": {
   *                         "slots": 2,
   *                         "reach": 16700
   *                     }
   *                 }
   *             ]
   *         }
   *     ]
   * }
   * \endcode
   *
   * The method automatically detects the format:
   * - If a modulation contains "slots" and "reach" fields directly, it's treated as single band (C band).
   * - If a modulation contains an array with band-specific data, it's treated as multi-band.
   *
   * Each JSON item corresponds to a specific bit rate, and each bit rate has a
   * list of modulation formats. Every modulation format has its own optical
   * reach and slots demand per band.
   *
   * @param fileName location of JSON file, as a relative path. For example:
   * "../bitrate/bitrate.json"
   */
  static std::vector<std::shared_ptr<BitRate>> readBitRatesFile(std::string fileName);

  
private:
    double bitRate;
    std::string bitRateStr;
    std::vector<ModulationFormat> modulationFormats;

    void validateAux(int modulationPos) const;
};