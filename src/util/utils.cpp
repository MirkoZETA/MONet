#include "utils.hpp"

namespace fns {

char bandToChar(Band band) {
    switch (band) {
        case Band::C: return 'C';
        case Band::L: return 'L';
        case Band::S: return 'S';
        case Band::E: return 'E';
        case Band::U: return 'U';
        case Band::O: return 'O';
        default: throw std::runtime_error("Unknown band enum value");
    }
}

Band charToBand(char c) {
    switch (c) {
        case 'C': return Band::C;
        case 'L': return Band::L;
        case 'S': return Band::S;
        case 'E': return Band::E;
        case 'U': return Band::U;
        case 'O': return Band::O;
        default: throw std::runtime_error("Unknown band: " + std::string(1, c));
    }
}

FiberType stringToFiberType(const std::string& type) {
    if (type == "SSMF") return FiberType::SSMF;
    if (type == "FMF") return FiberType::FMF;
    if (type == "MCF") return FiberType::MCF;
    if (type == "FMMCF") return FiberType::FMMCF;
    if (type == "HCF") return FiberType::HCF;
    throw std::runtime_error("Unknown fiber type: " + type);
}

std::string fiberTypeToString(FiberType type) {
    switch (type) {
        case FiberType::SSMF: return "SSMF";
        case FiberType::FMF: return "FMF";
        case FiberType::MCF: return "MCF";
        case FiberType::FMMCF: return "FMMCF";
        case FiberType::HCF: return "HCF";
        default: throw std::runtime_error("Unknown fiber type enum value");
    }
}

}