#ifndef __MODULATION_FORMAT_H__
#define __MODULATION_FORMAT_H__

// STL
#include <string>
#include <map>
// util
#include "../util/utils.hpp"

/**
 * @brief Class ModulationFormat
 *
 * The ModulationFormat class represents a specific modulation format (e.g., BPSK, QPSK, 8-QAM)
 * with its characteristics including required slots per band, reach per band, and other
 * optical transmission parameters.
 */
class ModulationFormat
{
public:
	/**
	 * @brief Construct a new ModulationFormat object
	 *
	 * @param modulationStr The name of the modulation format (e.g., "BPSK", "QPSK")
	 */
	ModulationFormat(const std::string &modulationStr);

	/**
	 * @brief Construct a new ModulationFormat object with band-specific parameters
	 *
	 * @param modulationStr The name of the modulation format
	 * @param requiredSlotsPerBand Map of required slots for each band
	 * @param reachPerBand Map of maximum reach distance for each band
	 */
	ModulationFormat(const std::string &modulationStr,
									 const std::map<fns::Band, int> &requiredSlotsPerBand,
									 const std::map<fns::Band, double> &reachPerBand);

	/**
	 * @brief Get the modulation format name
	 *
	 * @return std::string The modulation format name
	 */
	std::string getModulationStr() const;

	/**
	 * @brief Get required slots for a specific band
	 *
	 * @param band The band to query
	 * @return int Number of required slots for the band
	 */
	int getRequiredSlots(fns::Band band) const;

	/**
	 * @brief Get required slots for default band (C)
	 *
	 * @return int Number of required slots for C band
	 */
	int getRequiredSlots() const;

	/**
	 * @brief Get maximum reach for a specific band
	 *
	 * @param band The band to query
	 * @return double Maximum reach distance for the band
	 */
	double getReach(fns::Band band) const;

	/**
	 * @brief Get maximum reach for default band (C)
	 *
	 * @return double Maximum reach distance for C band
	 */
	double getReach() const;

	/**
	 * @brief Set required slots for a specific band
	 *
	 * @param band The band to set
	 * @param slots Number of required slots
	 */
	void setRequiredSlots(fns::Band band, int slots);

	/**
	 * @brief Set maximum reach for a specific band
	 *
	 * @param band The band to set
	 * @param reach Maximum reach distance
	 */
	void setReach(fns::Band band, double reach);

	/**
	 * @brief Get required GSNR (for future use)
	 *
	 * @return double Required GSNR in dB
	 */
	double getRequiredGSNR() const;

	/**
	 * @brief Set required GSNR (for future use)
	 *
	 * @param gsnr Required GSNR in dB
	 */
	void setRequiredGSNR(double gsnr);

	/**
	 * @brief Get baud rate (for future use)
	 *
	 * @return double Baud rate in GBaud
	 */
	double getBaudRate() const;

	/**
	 * @brief Set baud rate (for future use)
	 *
	 * @param baudRate Baud rate in GBaud
	 */
	void setBaudRate(double baudRate);

private:
	std::string modulationStr;
	std::map<fns::Band, int> requiredSlotsPerBand;
	std::map<fns::Band, double> reachPerBand;

	// Future use parameters
	double requiredGSNR = 0.0; // Required GSNR in dB
	double baudRate = 0.0;		 // Baud rate in GBaud
};

#endif
