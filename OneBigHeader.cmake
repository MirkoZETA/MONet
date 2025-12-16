# OneBigHeader.cmake - Generates a single amalgamated header file
# This version uses explicit ordering to handle dependencies correctly.

set(HEADER_NAME "simulator.hpp")
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME})

# Define the correct order of headers based on dependencies
# Order: utilities -> core types -> complex types -> allocators -> simulation
set(ORDERED_HEADERS
    # Utilities first (no dependencies)
    "util/version.hpp"
    "util/utils.hpp"
    "util/json.hpp"
    
    # Math classes (depend on utilities)
    "math/random_variable.hpp"
    "math/uniform_variable.hpp"
    "math/normal_variable.hpp"
    "math/exp_variable.hpp"
    
    # Core simulation event
    "sim/event.hpp"
    
    # Core types in dependency order
    "core/node.hpp"
    "core/modulation_format.hpp"
    "core/p2p.hpp"
    "core/fiber.hpp"
    "core/link.hpp"
    "core/bitrate.hpp"
    "core/connection.hpp"
    "core/demand.hpp"
    "core/network.hpp"
    
    # Allocator base class
    "alloc/allocator.hpp"
    "alloc/dummy_allocator.hpp"
    
    # Simulation classes (depend on core and allocator)
    "sim/controller.hpp"
    "sim/simulator.hpp"
    
    # Macros MUST be last (they reference all the above types)
    "util/macros.hpp"
)

# Concatenate all headers in order
foreach(header ${ORDERED_HEADERS})
    set(header_path "${PROJECT_SOURCE_DIR}/src/${header}")
    if(EXISTS "${header_path}")
        file(READ "${header_path}" aux)
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME} "${aux}\n")
        message(STATUS "Added: ${header}")
    else()
        message(WARNING "Header not found: ${header_path}")
    endif()
endforeach()

# Comment out all internal #include statements
file(READ ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME} filedata)
string(REGEX REPLACE "#include \"" "// #include \"" filedata "${filedata}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME} "${filedata}")

message(STATUS "Generated single header: ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME}")