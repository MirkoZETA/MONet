@page installation Installation

@tableofcontents

@section quick-start Quick Start (Recommended)

The easiest way to use Flex Net Sim is through our single header file implementation:

1. Download the latest stable version of the header file <a href="https://daniloborquez.gitlab.io/flex-net-sim/simulator.hpp" target="_blank" rel="noopener noreferrer">**here**</a>.

2. Place the header file in your project directory and include it:
   ~~~~~~~~~~~~~~~{.cpp}
   #include "./simulator.hpp"
   ~~~~~~~~~~~~~~~

That's it! You're ready to start simulating. See the [Getting Started](@ref starting) or [Coding an Algorithm](@ref algorithm) pages for next steps.

@section long-start Install System-Wide

@note
The **full installation** is only necessary if you want system-wide access to the library.

For system-wide access, follow these steps:

@subsection prerequisites Prerequisites
   - <a href="https://cmake.org" target="_blank" rel="noopener noreferrer">CMake</a>
   - C++ compatible compiler
   - Git (optional, for development version)

@subsection download Download and Setup
   1. Download the latest release from our <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/releases" target="_blank" rel="noopener noreferrer">releases page</a>
   2. Extract the archive.
   3. Navigate to the root folder
   ~~~~~~~~~~~~~~~{.bash}
   cd flex-net-sim
   ~~~~~~~~~~~~~~~

@subsection build Build and Install

   Choose your platform:

   **Unix (Linux/macOS)**
   ~~~~~~~~~~~~~~~{.bash}
   mkdir build
   cd build
   cmake ..
   cmake --build .
   sudo cmake --install .
   ~~~~~~~~~~~~~~~

   **Windows (Visual Studio)**
   ~~~~~~~~~~~~~~~{.powershell}
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   cmake --install .
   ~~~~~~~~~~~~~~~

   **Windows (MinGW)**
   ~~~~~~~~~~~~~~~{.bash}
   mkdir build
   cd build
   cmake .. -G "MinGW Makefiles"
   cmake --build .
   cmake --install .
   ~~~~~~~~~~~~~~~

@subsection use Usage
   After installation, include the library in your C++ files:
   ~~~~~~~~~~~~~~~{.cpp}
   #include <fnsim/simulator.hpp>
   ~~~~~~~~~~~~~~~
   Compile your program using:
   ~~~~~~~~~~~~~~~{.bash}
    g++ <your cpp files> -lfnsim
   ~~~~~~~~~~~~~~~

@section develop-start Development Version

If you prefer the **latest development version**, clone the develop branch from the repository using Git:

~~~~~~~~~~~~~~~{.bash}
git clone -b develop --single-branch git@gitlab.com:DaniloBorquez/flex-net-sim.git
~~~~~~~~~~~~~~~

@warning
The development version is updated frequently and may contain unstable changes or bugs that could affect results.

@section troubleshooting Troubleshooting

Common installation issues and solutions:

1. **CMake Not Found**: Ensure CMake is installed and added to your system PATH.
2. **Compilation Errors**: Verify you have a C++ compatible compiler.
3. **Installation Permission Denied**: Use sudo (Linux/macOS) or run as administrator (Windows).

<div class="section_buttons">
| Previous          |                              Next |
|:------------------|----------------------------------:|
| [Home](@ref index) | [Getting Started](@ref starting) |
</div>