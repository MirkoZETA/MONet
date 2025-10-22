#ifndef UTILS_HPP
#define UTILS_HPP
#include <stdexcept>
#include <string>

namespace fns {

// Fiber types
enum class FiberType {
    SSMF,    // Standard Single-Mode Fiber
    FMF,     // Few-Mode Fiber
    MCF,     // Multi-Core Fiber
    FMMCF,   // Few-Mode Multi-Core Fiber
    HCF      // Hollow-Core Fiber
};

// Optical bands
enum class Band {
    O,       // Original band (1260-1360 nm)
    E,       // Extended band (1360-1460 nm)
    S,       // Short band (1460-1530 nm)
    C,       // Conventional band (1530-1565 nm)
    L,       // Long band (1565-1625 nm)
    U        // Ultra-long band (1625-1675 nm)
};

// Default link parameters
namespace defaults {
    constexpr int SLOTS = 320;
    constexpr double LENGTH = 100.0;
    constexpr int FIBERS = 1;
    constexpr int CORES = 1;
    constexpr int MODES = 1;
    constexpr Band BAND = Band::C;
    constexpr int FIBER_INDEX = 0;
    constexpr int CORE_INDEX = 0;
    constexpr int MODE_INDEX = 0;
}

// Converts Band enum to char
char bandToChar(Band band);

// Converts char to Band enum
Band charToBand(char c);

// Converts string to FiberType enum
FiberType stringToFiberType(const std::string& type);

} // namespace fns

#endif // UTILS_HPP