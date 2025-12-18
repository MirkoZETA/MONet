#pragma once

// STL
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>
// util
#include "../util/utils.hpp"

/**
 * @brief Class Fiber
 *
 * The Fiber class represents an optical fiber within the network simulator. Each fiber
 * can be of different types (SSMF, FMF, MCF, FMMCF, HCF) and can support multiple
 * bands and modes.
 *
 * This is the lowest level class representing fiber characteristics and resources. Owns
 * the resource allocation matrix and provides methods to manage the slots allocation.
 */

class Fiber
{
public:
	/**
	 * @brief Default constructor for Fiber.
	 *
	 * Initializes members to default values:
	 * - cores = 1
	 * - modes = 1
	 * - band = C band
	 * - slots = 320
	 * (defaults are defined in CONSTANTS_HPP).
	 */
	Fiber();

	/**
	 * @brief Constructor for SSMF Fiber with specified number of slots in C-band.
	 *
	 * @param slots Number of slots in C-band (must be > 0).
	 */
	Fiber(int slots);

	/**
	 * @brief Constructor for Fiber with complete slot specification per core and mode for multiple bands.
	 *
	 * Creates a fiber with specified bands and sets individual slot counts for each band/core/mode combination.
	 * Each core can have a different number of modes, and each core/mode combination can have different slot counts.
	 * All bands must have the same number of cores (physical constraint), but modes can vary per band and core.
	 *
	 * @param bandSlotMatrix Map where each band maps to a 2D vector [core][mode] = slot_count.
	 *                       Format: bandSlotMatrix[Band][core_index][mode_index] = slot_count
	 *                       Structure requirements:
	 *                       - All bands must have the same number of cores (physical constraint)
	 *                       - Each core can have different number of modes per band
	 *                       - Each slot_count value must be > 0
	 *
	 *                       Example for 1 band, 1 core, 1 mode and 320 slots:
	 *                       bandSlotMatrix = {
	 *                         {Band::C, {
	 *                           {320}  // Core 0: 1 mode in C-band
	 *                         }}
	 *                       };
	 *
	 *                       Example for 2 bands, 3 cores with different mode configurations:
	 *                       bandSlotMatrix = {
	 *                         {Band::C, {
	 *                           {100, 80, 60},    // Core 0: 3 modes in C-band
	 *                           {90, 70},         // Core 1: 2 modes in C-band
	 *                           {120}             // Core 2: 1 mode in C-band
	 *                         }},
	 *                         {Band::L, {
	 *                           {110, 85},        // Core 0: 2 modes in L-band
	 *                           {95},             // Core 1: 1 mode in L-band
	 *                           {125, 105, 80}    // Core 2: 3 modes in L-band
	 *                         }}
	 *                       };
	 *
	 * @throws std::invalid_argument If bandSlotMatrix is empty, bands have different core counts, or any slot count is <= 0.
	 */
	Fiber(const std::map<fns::Band, std::vector<std::vector<int>>> &bandSlotMatrix);

	/**
	 * @brief Set the fiber type.
	 *
	 * @param type Fiber type to set.
	 */
	void setType(fns::FiberType type);
	/**
	 * @brief Get the fiber type.
	 *
	 * @return FiberType The type of the fiber (SSMF, FMF, MCF, FMMCF, HCF).
	 */
	fns::FiberType getType(void) const;

	/**
	 * @brief Add a new band to the fiber.
	 *
	 * This method adds a new band to the fiber with the specified number of slots
	 * and modes. The new band will be initialized with the same number of cores
	 * as the existing fiber structure (this constraint cannot be violated).
	 * If the fiber has no existing bands, it will use default values.
	 * If the specified band already exists, an exception will be thrown.
	 *
	 * @param band  The band to add (e.g., Band::L).
	 * @param slots Number of slots for this band (must be > 0).
	 * @param modes Number of modes for this band (must be > 0).
	 */
	void addBand(fns::Band band, int modes, int slots); /**
	 * @brief Get a list of all initialized bands in the fiber.
	 *
	 * @return std::vector<fns::Band> Vector of initialized bands.
	 */
	std::vector<fns::Band> getBands(void) const;
	/**
	 * @brief Get the number of initialized bands in the fiber.
	 *
	 * @return int Number of initialized bands.
	 */
	int getNumberOfBands(void) const;

	/**
	 * @brief Set the core configuration with detailed mode and slot specification.
	 *
	 * This method sets the complete core configuration where each core can have
	 * different numbers of modes, and each mode can have different numbers of slots.
	 * Cannot be used if any slots are already allocated.
	 *
	 * @param coreConfig Vector where:
	 *   - First index: core number (0, 1, 2, ...)
	 *   - Second vector: modes for that core
	 *   - Integer values: number of slots for each mode
	 *
	 * Example: {{80, 120}, {160}} means:
	 *   - Core 0: 2 modes (80 slots, 120 slots)
	 *   - Core 1: 1 mode (160 slots)
	 *
	 * @throws std::invalid_argument If coreConfig is empty or contains invalid slot counts
	 * @throws std::runtime_error If any slots are already allocated
	 */
	void setCores(const std::vector<std::vector<int>> &coreConfig);
	/**
	 * @brief Get the number of cores in the fiber.
	 *
	 * @return int Number of cores.
	 */
	int getNumberOfCores(void) const;
	/**
	 * @brief Set the modes for a specific core in a specific band with individual slot counts.
	 *
	 * This setter cannot be used if slots are already allocated in that core/band combination.
	 * The vector size determines the number of modes, and each element specifies the slot count for that mode.
	 *
	 * @param core  Core index to set (0-based).
	 * @param band  Band to modify.
	 * @param slotsPerMode Vector where each element specifies the number of slots for that mode (all must be > 0).
	 *                     The vector size determines the number of modes to create.
	 *
	 * @throws std::invalid_argument If slotsPerMode is empty or any slot count is <= 0.
	 * @throws std::out_of_range If core index is invalid.
	 */
	void setModes(int core, fns::Band band, const std::vector<int> &slotsPerMode);
	/**
	 * @brief Get the number of modes for a specific core and band in the fiber.
	 *
	 * @param core Core index to query (0-based).
	 * @param band Band to query.
	 * @return int Number of modes for the specified core and band.
	 */
	int getNumberOfModes(int core, fns::Band band) const;

	/**
	 * @brief Get the value of a specific slot (connection ID or -1 if free).
	 *
	 * @param core Core index (0-based).
	 * @param band Band to query.
	 * @param mode Mode index (0-based).
	 * @param slotPos Slot position index (0-based).
	 * @return int Connection ID occupying the slot, or -1 if the slot is free.
	 */
	int getSlot(int core, fns::Band band, int mode, int slotPos) const;

	std::vector<int> getSlots (int core, fns::Band band, int mode) const;
	/**
	 * @brief Allocate a specific slot to a connection.
	 *
	 * @param core Core index (0-based).
	 * @param band Band to allocate in.
	 * @param mode Mode index (0-based).
	 * @param slotPos Slot position index (0-based).
	 * @param connectionId Connection ID to assign to the slot.
	 */
	void setSlot(int core, fns::Band band, int mode, int slotPos, int connectionId);

	/**
	 * @brief Set the number of slots for a specific band, core and mode in the fiber. This setter cannot be
	 * used if slots are already allocated.
	 *
	 * @param band  Band to set (e.g., Band::L).
	 * @param core  Core index to set (0-based).
	 * @param mode  Mode index to set (0-based).
	 * @param slots Number of slots to set (must be > 0).
	 */
	void setSlots(int core, fns::Band band, int mode, int slots);

	/**
	 * @brief Get the number of slots for a specific core, band, and mode in the fiber.
	 *
	 * @param core Core index to query (0-based).
	 * @param band Band to query.
	 * @param mode Mode index to query (0-based).
	 * @return int Number of slots for the specified core, band, and mode.
	 */
	int getNumberOfSlots(int core, fns::Band band, int mode) const;

	/**
	 * @brief Check if the fiber has any allocated slots (is being used).
	 *
	 * @return bool True if any slot is occupied, false if all slots are free.
	 */
	bool isActive(void) const;
	/**
	 * @brief Check if fiber is dedicated to P2P traffic
	 *
	 * @return true if fiber is exclusively for P2P use
	 */
	bool isDedicatedToP2P(void) const;
	/**
	 * @brief Set P2P dedication status
	 * @param dedicated true to mark as P2P dedicated, false otherwise
	 * @throws std::runtime_error if trying to dedicate an active fiber
	 */
	void setDedicatedToP2P(bool dedicated);

	/**
	 * @brief Reset fiber to initial state with all slots free but structure preserved.
	 *
	 * This method sets all slots in all bands, cores, and modes to 0 (free),
	 * but maintains the fiber's band/core/mode structure intact. Use this to
	 * "clean" a fiber for reuse while keeping its configuration.
	 */
	void resetFiber(void);
	/**
	 * @brief Clear all fiber resources and structure completely.
	 *
	 * This method completely empties the fiber's resource map, removing all
	 * bands, cores, modes, and slots. The fiber becomes empty and needs to be
	 * reconfigured before use. Use this for complete fiber destruction/cleanup.
	 *
	 * @warning If the fiber has any allocated slots (active connections), a
	 * warning message will be printed to stderr. Consider using resetFiber()
	 * to preserve structure while clearing allocations, or ensure the fiber
	 * is inactive before calling this method.
	 */
	void clearFiber(void);

	/**
	 * @brief Validate that the given core, band, mode, and slot position are within valid ranges.
	 *
	 * @param core Core index to validate (0-based).
	 * @param band Band to validate.
	 * @param mode Mode index to validate (0-based).
	 * @param slotPos Slot position index to validate (0-based).
	 * @throws std::out_of_range If any index is out of range.
	 * @throws std::invalid_argument If the band is not found.
	 */
	void validateAux(int core, fns::Band band, int mode, int slotPos) const;

	/**
	 * @brief Convert a string representation to a FiberType enum value.
	 *
	 * @param type String representation of the fiber type (e.g., "SSMF", "FMF", "MCF", "FMMCF", "HCF").
	 * @return FiberType The corresponding enum value.
	 * @throws std::runtime_error If the string doesn't match any known fiber type.
	 */
	static fns::FiberType stringToFiberType(const std::string &type);
	/**
	 * @brief Convert a character representation to a Band enum value.
	 *
	 * @param c Character representation of the band ('C', 'L', 'S', 'E', 'U', 'O').
	 * @return Band The corresponding enum value.
	 * @throws std::runtime_error If the character doesn't match any known band.
	 */
	static fns::Band charToBand(char c);

	/**
	 * @brief Convert a Band enum value to its character representation.
	 *
	 * @param band Band enum value to convert.
	 * @return char Character representation of the band ('C', 'L', 'S', 'E', 'U', 'O').
	 */
	static char bandToChar(fns::Band band);

	/**
	 * @brief Detects and assigns the fiber type based on its characteristics.
	 *
	 * Determines fiber type using the following rules:
	 * - SSMF: 1 core, 1 mode (any number of bands)
	 * - FMF: 1 core, multiple modes (any number of bands)
	 * - MCF: multiple cores, 1 mode (any number of bands)
	 * - FMMCF: multiple cores, multiple modes (any number of bands)
	 * - HCF: default fallback (not implemented)
	 *
	 * The number of bands does NOT affect fiber type detection.
	 * Multi-band support is available for all fiber types.
	 *
	 */
	void detectType(void);

private:
	fns::FiberType fiberType;
	bool _isDedicatedToP2P;
	std::map<fns::Band, std::vector<std::vector<std::vector<int>>>> resources;
};