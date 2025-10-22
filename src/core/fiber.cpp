#include "fiber.hpp"

Fiber::Fiber(void) {
	this->resources = std::map<fns::Band, std::vector<std::vector<std::vector<int>>>>();
	this->resources[fns::defaults::BAND] = std::vector<std::vector<std::vector<int>>>(
			fns::defaults::CORES, std::vector<std::vector<int>>(
					fns::defaults::MODES, std::vector<int>(fns::defaults::SLOTS)
			)
	);
	this->setType(fns::FiberType::SSMF);
	this->_isDedicatedToP2P = false;
}

Fiber::Fiber(int slots) {
	if (slots < 1) {
		throw std::invalid_argument("Slots must be at least 1");
	}
	this->resources = std::map<fns::Band, std::vector<std::vector<std::vector<int>>>>();
	this->resources[fns::Band::C] = std::vector<std::vector<std::vector<int>>>(
			fns::defaults::CORES, std::vector<std::vector<int>>(
					fns::defaults::MODES, std::vector<int>(slots)
			)
	);
	this->setType(fns::FiberType::SSMF);
	this->_isDedicatedToP2P = false;
}

Fiber::Fiber(const std::map<fns::Band, std::vector<std::vector<int>>>& bandSlotMatrix) {
	if (bandSlotMatrix.empty()) {
		throw std::invalid_argument("Band slot matrix cannot be empty.");
	}
	
	// Get dimensions from first band
	auto firstBand = bandSlotMatrix.begin();
	const auto& firstMatrix = firstBand->second;
	
	if (firstMatrix.empty() || firstMatrix[0].empty()) {
		throw std::invalid_argument("Slots per core/mode matrix cannot be empty for any band.");
	}
	
	int cores = firstMatrix.size();
	int modes = firstMatrix[0].size();
	
	// Validate that all bands have same number of cores but allow different modes
	for (const auto& [band, matrix] : bandSlotMatrix) {
		if (matrix.empty() || matrix[0].empty()) {
			throw std::invalid_argument("Matrix cannot be empty for any band.");
		}
		if (matrix.size() != cores) {
			throw std::invalid_argument("All bands must have the same number of cores.");
		}
		for (const auto& row : matrix) {
			for (int slots : row) {
				if (slots < 1) {
					throw std::invalid_argument("All slot counts must be positive.");
				}
			}
		}
	}
	
	// Initialize resources
	this->resources = std::map<fns::Band, std::vector<std::vector<std::vector<int>>>>();
	
	// Set individual slot counts for each band/core/mode combination
	for (const auto& [band, matrix] : bandSlotMatrix) {
		this->resources[band] = std::vector<std::vector<std::vector<int>>>(cores);
		for (int core = 0; core < cores; core++) {
			int bandModes = matrix[core].size();
			this->resources[band][core].resize(bandModes);
			for (int mode = 0; mode < bandModes; mode++) {
				int slots = matrix[core][mode];
				this->resources[band][core][mode].resize(slots);
			}
		}
	}
	this->detectType();
	this->_isDedicatedToP2P = false;
}

void Fiber::setType(fns::FiberType type) {
	this->fiberType = type;
}
fns::FiberType Fiber::getType() const {
	return this->fiberType;
}
void Fiber::detectType(void) {
	// SSMF: 1 core, all core-band combinations have exactly 1 mode
	// FMF: 1 core, at least one core-band combination has multiple modes
	// MCF: multiple cores, all core-band combinations have exactly 1 mode
	// FMMCF: multiple cores, at least one core-band combination has multiple modes
	// HCF: default fallback (not implemented)
	int cores = this->getNumberOfCores();
	bool hasMultipleModes = false;
	// Check if any core-band combination has more than 1 mode
	for (const auto& [band, slotMatrix] : this->resources) {
		for (int core = 0; core < cores; core++) {
			if (slotMatrix[core].size() > 1) {
				hasMultipleModes = true;
				break;
			}
		}
		if (hasMultipleModes) break; // Early exit once we find multiple modes
	}
	
	// Determine fiber type based on cores and modes
	if (cores == 1 && !hasMultipleModes) {
		this->setType(fns::FiberType::SSMF);
	} else if (cores == 1 && hasMultipleModes) {
		this->setType(fns::FiberType::FMF);
	} else if (cores > 1 && !hasMultipleModes) {
		this->setType(fns::FiberType::MCF);
	} else if (cores > 1 && hasMultipleModes) {
		this->setType(fns::FiberType::FMMCF);
	} else {
		this->setType(fns::FiberType::HCF); // Default fallback
	}
}

void Fiber::addBand(fns::Band band, int modes, int slots){
	if (this->resources.find(band) != this->resources.end()) {
		throw std::invalid_argument("Band already exists.");
	}
	if (modes < 1) {
		throw std::invalid_argument("Cannot set a fiber with non-positive modes.");
	}
	if (slots < 1) {
		throw std::invalid_argument("Cannot set a fiber with non-positive slots.");
	}
	// Ensure the new band has the same number of cores as existing bands
	int cores = this->getNumberOfCores();
	if (cores == 0) {
		// If no bands exist yet, use default cores
		cores = fns::defaults::CORES;
	}
	this->resources[band] = std::vector<std::vector<std::vector<int>>>(
		cores, std::vector<std::vector<int>>(
			modes, std::vector<int>(slots)
		)
	);
}

std::vector<fns::Band> Fiber::getBands(void) const {
	std::vector<fns::Band> bands;
	for (const auto& [band, slotMatrix] : this->resources) {
		bands.push_back(band);
	}
	return bands;
}
int Fiber::getNumberOfBands(void) const {
	if (this->resources.empty()) {
		return 0;
	}
	return this->resources.size();
}


void Fiber::setCores(const std::vector<std::vector<int>>& coreConfig) {
	if (coreConfig.empty()) {
		throw std::invalid_argument("Cannot set cores with empty configuration.");
	}
	
	// Validate that all cores have at least one mode and all slot counts are positive
	for (int core = 0; core < coreConfig.size(); core++) {
		if (coreConfig[core].empty()) {
			throw std::invalid_argument("Core " + std::to_string(core) + " must have at least one mode.");
		}
		for (int mode = 0; mode < coreConfig[core].size(); mode++) {
			if (coreConfig[core][mode] < 1) {
				throw std::invalid_argument("All slot counts must be positive (Core " + 
					std::to_string(core) + ", Mode " + std::to_string(mode) + ").");
			}
		}
	}
	// Check if any slots are currently allocated across all bands
	for (const auto& [band, slotMatrix] : this->resources) {
		for (const auto& perCore : slotMatrix) {
			for (const auto& perMode : perCore) {
				for (const auto& slot : perMode) {
					if (slot != 0) {
						throw std::runtime_error("Cannot change core configuration when slots are already allocated.");
					}
				}
			}
		}
	}
	
	// Apply the new configuration to all bands
	int newCores = coreConfig.size();
	for (auto& [band, slotMatrix] : this->resources) {
		slotMatrix.clear();
		slotMatrix.resize(newCores);
		
		for (int core = 0; core < newCores; core++) {
			int modesForCore = coreConfig[core].size();
			slotMatrix[core].resize(modesForCore);
			
			for (int mode = 0; mode < modesForCore; mode++) {
				int slotsForMode = coreConfig[core][mode];
				slotMatrix[core][mode].resize(slotsForMode, 0); // Initialize all slots to 0
			}
		}
	}
}
int Fiber::getNumberOfCores(void) const {
	if (this->resources.empty()) {
		return 0;
	}
	return this->resources.begin()->second.size();
}

void Fiber::setModes(int core, fns::Band band, const std::vector<int>& slotsPerMode) {
	if (core < 0 || core >= this->getNumberOfCores()) {
		throw std::out_of_range("Core index out of range");
	}
	// Find the specific band
	auto bandIt = this->resources.find(band);
	if (bandIt == this->resources.end()) {
		throw std::invalid_argument("Band not found in fiber. Use addBand() first.");
	}
	if (slotsPerMode.empty()) {
		throw std::invalid_argument("Cannot set modes with empty slotsPerMode vector.");
	}
	// Validate that all slot counts are positive
	for (int slots : slotsPerMode) {
		if (slots < 1) {
			throw std::invalid_argument("All slot counts must be positive.");
		}
	}
	auto& slotMatrix = bandIt->second;
	auto& coreSlots = slotMatrix[core];
	
	// Check if any slots are allocated in this specific core/band
	for (const auto& perMode : coreSlots) {
		for (const auto& slot : perMode) {
			if (slot != 0) {
				throw std::runtime_error("Cannot change number of modes when slots are already allocated.");
			}
		}
	}
	// Set new mode configuration with individual slot counts
	coreSlots.clear();
	coreSlots.reserve(slotsPerMode.size());
	for (int slots : slotsPerMode) {
		coreSlots.emplace_back(slots, 0); // Create mode with 'slots' elements, all set to 0
	}
}

int Fiber::getNumberOfModes(int core, fns::Band band) const {
	if (this->resources.empty()) {
		return 0;
	}
	// Return modes for specified band and core
	auto bandIt = this->resources.find(band);
	if (bandIt == this->resources.end()) {
		throw std::invalid_argument("Band not found in fiber");
	}
	if (core < 0 || core >= bandIt->second.size()) {
		throw std::out_of_range("Core index out of range");
	}
	return bandIt->second[core].size();
}

void Fiber::setSlots(int core, fns::Band band, int mode, int slots) {
	auto bandIt = this->resources.find(band);
	if (bandIt == this->resources.end()) {
		throw std::invalid_argument("Band not found in fiber. Use addBand() first.");
	}
	
	auto& slotMatrix = bandIt->second;
	if (core < 0 || core >= slotMatrix.size()) {
		throw std::out_of_range("Core index out of range");
	}
	if (mode < 0 || mode >= slotMatrix[core].size()) {
		throw std::out_of_range("Mode index out of range");
	}
	if (slots < 1) {
		throw std::invalid_argument("Cannot set a fiber with non-positive slots.");
	}
	slotMatrix[core][mode].resize(slots);
}

int Fiber::getSlot(int core, fns::Band band, int mode, int slotPos) const {
	this->validateAux(core, band, mode, slotPos);
	return this->resources.at(band)[core][mode][slotPos];
}

std::vector<int> Fiber::getSlots(int core, fns::Band band, int mode) const {
	this->validateAux(core, band, mode, 0); // Validate core, band, mode (slotPos 0 is just to check mode exists)
	return this->resources.at(band)[core][mode];
}

int Fiber::getNumberOfSlots(int core, fns::Band band, int mode) const {
	if (this->resources.find(band) == this->resources.end()) {
		throw std::invalid_argument("Band not found");
	}
	if (core < 0 || core >= this->resources.at(band).size()) {
		throw std::out_of_range("Core index out of range");
	}
	if (mode < 0 || mode >= this->resources.at(band)[core].size()) {
		throw std::out_of_range("Mode index out of range");
	}
	return this->resources.at(band)[core][mode].size();
}

// Fiber Status
bool Fiber::isActive(void) const {
	// Check if ANY slot is occupied
	for (const auto& [band, slotMatrix] : this->resources) {
		for (const auto& coreSlots : slotMatrix) {
			for (const auto& modeSlots : coreSlots) {
				for (const auto& slot : modeSlots) {
					if (slot != 0) {
						return true;
					}
				}
			}
		}
	}
	return false;
}
bool Fiber::isDedicatedToP2P(void) const {
	return this->_isDedicatedToP2P;
}
void Fiber::setDedicatedToP2P(bool val) {
	if (val && this->isActive()) {
		throw std::runtime_error("Cannot dedicate fiber to P2P when it has active slot allocations");
	}
	this->_isDedicatedToP2P = val;
}

// For provisioning
void Fiber::setSlot(int core, fns::Band band, int mode, int slotPos, int connectionId) {
	this->validateAux(core, band, mode, slotPos);
    this->resources[band][core][mode][slotPos] = connectionId;
}

void Fiber::validateAux(int core, fns::Band band, int mode, int slotPos) const {
	if (this->resources.find(band) == this->resources.end()) {
		throw std::invalid_argument("Band not found");
	}
	if (core < 0 || core >= this->resources.at(band).size()) {
		throw std::out_of_range("Core index out of range");
	}
	if (mode < 0 || mode >= this->resources.at(band)[core].size()) {
		throw std::out_of_range("Mode index out of range");
	}
	if (slotPos < 0 || slotPos >= this->resources.at(band)[core][mode].size()) {
		throw std::out_of_range("Slot index out of range");
	}
}
void Fiber::resetFiber(void) {
	for (auto& [band, slotMatrix] : this->resources) {
		for (auto& coreSlots : slotMatrix) {
			for (auto& modeSlots : coreSlots) {
				std::fill(modeSlots.begin(), modeSlots.end(), 0);
			}
		}
	}
}
void Fiber::clearFiber(void) {
	// Warn user if fiber has active slot allocations
	if (this->isActive()) {
		std::cerr << "Warning: clearFiber() called on fiber with active slot allocations. "
		          << "All allocated connections will be lost. Use resetFiber() to keep structure "
		          << "and clear allocations, or ensure fiber is inactive before clearing." << std::endl;
	}
	
	this->resources.clear();
}