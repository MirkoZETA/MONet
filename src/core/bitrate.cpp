#include "bitrate.hpp"

BitRate::BitRate(double bitRate) {
  if (bitRate <= 0)
      throw std::invalid_argument("BitRate must be positive.");
  this->bitRate = bitRate;
  this->bitRateStr = std::to_string(bitRate);
}

void BitRate::addModulationFormat(const ModulationFormat& modulationFormat) {
  this->modulationFormats.push_back(modulationFormat);
}

void BitRate::addModulation(const std::string& modulation, 
                            const std::map<fns::Band, int>& slotsPerBand, 
                            const std::map<fns::Band, double>& reachPerBand) {
  ModulationFormat modulationFormat(modulation, slotsPerBand, reachPerBand);
  this->addModulationFormat(modulationFormat);
}

ModulationFormat BitRate::getModulationFormat(int modulationPos) const {
  this->validateAux(modulationPos);
  return this->modulationFormats[modulationPos];
}

ModulationFormat BitRate::getModulationFormat(const std::string& modulation) const {
  for (const auto& modulationFormat : this->modulationFormats) {
    if (modulationFormat.getModulationStr() == modulation) {
        return modulationFormat;
    }
  }
  throw std::invalid_argument("Modulation format '" + modulation + 
                            "' not found in BitRate " + this->bitRateStr);
}

std::string BitRate::getModulationStr(int modulationPos) const {
  this->validateAux(modulationPos);
  return this->getModulationFormat(modulationPos).getModulationStr();
}

int BitRate::getAdaptiveModulation(const Route &route) const {
  return getAdaptiveModulation(route, fns::Band::C);
}

int BitRate::getAdaptiveModulation(const Route &route, fns::Band band) const {
  double totalLength = std::accumulate(
    route.begin(), route.end(), 0.0,
    [](double sum, const std::shared_ptr<Link> &link) { return sum + link->getLength(); }
  );

  int bestModulation = -1;
  int minSlots = std::numeric_limits<int>::max();
  double maxReach = 0.0;

  for (int i = 0; i < this->getNumberOfModulations(); ++i) {
    // Try to get the reach and slots for this modulation and band
    // If it throws, the band is not supported by this modulation
    try {
      double reach = this->getReach(i, band);
      int slots = this->getRequiredSlots(i, band);
      
      // Select modulation that meets distance requirements
      if (reach >= totalLength) {
        // Prioritize: 1) minimum slots, 2) maximum reach (for same slot count)
        if (slots < minSlots || (slots == minSlots && reach > maxReach)) {
          minSlots = slots;
          maxReach = reach;
          bestModulation = i;
        }
      }
    } catch (const std::exception&) {
      // This modulation doesn't support the specified band, skip it
      continue;
    }
  }

  return bestModulation;
}

double BitRate::getBitRate() const { 
    return this->bitRate; 
}

std::string BitRate::getBitRateStr() const { 
    return this->bitRateStr; 
}

int BitRate::getNumberOfModulations() const { 
    return this->modulationFormats.size(); 
}

int BitRate::getRequiredSlots(int modulationPos) const {
  this->validateAux(modulationPos);
  return this->getModulationFormat(modulationPos).getRequiredSlots();
}

int BitRate::getRequiredSlots(int modulationPos, fns::Band band) const {
  this->validateAux(modulationPos);
  return this->getModulationFormat(modulationPos).getRequiredSlots(band);
}

double BitRate::getReach(int modulationPos) const {
  this->validateAux(modulationPos);
  return this->getModulationFormat(modulationPos).getReach();
}

double BitRate::getReach(int modulationPos, fns::Band band) const {
  return this->getModulationFormat(modulationPos).getReach(band);
}

std::vector<std::shared_ptr<BitRate>> BitRate::readBitRatesFile(std::string fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }
    nlohmann::ordered_json bitRateJson;
    std::vector<std::shared_ptr<BitRate>> vect;

    file >> bitRateJson;

    for (auto& x : bitRateJson.items()) {
        double bitRateValue = std::stod(x.key());  // BITRATE
        std::shared_ptr<BitRate> aux = std::make_shared<BitRate>(bitRateValue);
        
        // Iterate through modulations for this bit rate
        for (const auto& modulationEntry : x.value()) {
            for (auto& modulationData : modulationEntry.items()) {
                std::string modulationName = modulationData.key();
                const auto& modulationInfo = modulationData.value();
                
                std::map<fns::Band, int> slotsPerBand;
                std::map<fns::Band, double> reachPerBand;
                
                if (modulationInfo.is_object() && modulationInfo.contains("slots") && 
                    modulationInfo.contains("reach")) {
                    // Simple format: single band (C band assumed)
                    int slots = modulationInfo["slots"];
                    double reach = modulationInfo["reach"];
                    
                    // Validation
                    if (reach < 0) {
                        throw std::runtime_error("value entered for reach is less than zero");
                    }
                    if (slots < 0) {
                        throw std::runtime_error("value entered for slots is less than zero");
                    }
                    
                    slotsPerBand[fns::Band::C] = slots;
                    reachPerBand[fns::Band::C] = reach;
                    
                } else if (modulationInfo.is_array()) {
                    // Multi-band format: array with a single object containing all bands
                    if (modulationInfo.size() != 1) {
                        throw std::runtime_error("Multi-band format should contain exactly one object with all bands");
                    }
                    
                    const auto& bandObject = modulationInfo[0];
                    for (auto& bandData : bandObject.items()) {
                        std::string bandStr = bandData.key();
                        char bandChar = bandStr[0];
                        fns::Band band = fns::charToBand(bandChar);
                        
                        int slots = bandData.value()["slots"];
                        double reach = bandData.value()["reach"];
                        
                        // Validation
                        if (reach < 0) {
                            throw std::runtime_error("value entered for reach is less than zero");
                        }
                        if (slots < 0) {
                            throw std::runtime_error("value entered for slots is less than zero");
                        }
                        
                        slotsPerBand[band] = slots;
                        reachPerBand[band] = reach;
                    }
                } else {
                    throw std::runtime_error("Unknown modulation format structure in JSON");
                }
                
                aux->addModulation(modulationName, slotsPerBand, reachPerBand);
            }
        }
        vect.push_back(aux);
    }

    return vect;
}

void BitRate::validateAux(int modulationPos) const {
  if (modulationPos < 0 || modulationPos >= this->modulationFormats.size()) {
    throw std::out_of_range("Invalid modulation position");
  }
}
