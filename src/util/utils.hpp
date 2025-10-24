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

// Converts FiberType enum to string
std::string fiberTypeToString(FiberType type);

// ANSI color codes for console output
namespace colors {
    // Reset and formatting
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* ITALIC = "\033[3m";
    constexpr const char* BOLD_CYAN = "\033[1;36m";
    
    // Colors for status indicators
    constexpr const char* RED = "\033[31m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BRIGHT_GREEN = "\033[92m";
    constexpr const char* BRIGHT_RED = "\033[91m";
    // Orange (256-color palette) for intermediate warning
    constexpr const char* ORANGE = "\033[38;5;208m";
}

} // namespace fns

#endif // UTILS_HPP