@page starting Getting Started

@tableofcontents

@note Before proceeding, ensure you have installed the library by following the instructions in [Installation](@ref installation).

**This guide** will walk you through the essential steps to **set up, configure, and run your first simulation**. It covers installation options, required input files, and a step-by-step example to ensure a successful first run.

@section importing Importing the Library

Learn how to import the library and verify it's working correctly.

@subsection import-header Using the Header File

If you're using the single header file version, follow these steps:

1. Place <a href="https://daniloborquez.gitlab.io/flex-net-sim/simulator.hpp" target="_blank" rel="noopener noreferrer">`simulator.hpp`</a> in your project directory.
2. Include it in your C++ code:
```cpp
#include "simulator.hpp"
```
3. Compile and run your program:
```bash
g++ your_program.cpp -o simulation.out
```
```
./simulation.out
```

If the code **runs without any errors**, the library is installed and working correctly.

@subsection import-syswide System-Wide

@subsubsection import-unix Unix Systems

1. Include the library header in your code:
```cpp
#include <fnsim/simulator.h>
```
2. Compile with the library flag:
```bash
g++ your_program.cpp -lfnsim -o simulation.out
```
3. Run your program:
```
./simulation.out
```

If the code **runs without any errors**, the library is installed and working correctly.

@subsubsection import-windows Windows Systems

Windows requires additional compilation flags and path specifications:

1. Include the library header:
```cpp
#include <fnsim/simulator.hpp>
```
2. Compile with required flags and paths:
```cmd
g++ -std=c++17 your_program.cpp ^
    -I"C:/Program Files (x86)/flexible-networks-simulator/include/" ^
    -L"C:/Program Files (x86)/flexible-networks-simulator/lib/" ^
    -lfnsim ^
    -o simulation.exe
```
@note
The default installation path is typically ```C:/Program Files (x86)/flexible-networks-simulator/```. Adjust the path if you installed the library elsewhere.
3. Run your program:
```cmd
simulation.exe
```

If the code **runs without any errors**, the library is installed and working correctly.

@section files Necessary Files

To run a simulation with the library, you need JSON files that define the network topology and configuration. These files specify the network structure, precomputed paths, and optionally, the bit rates and modulation formats.

@subsection network-files Network and Routes

The network and route files are mandatory, as they specify the network structure and a set of precomputed paths. For a simple demonstration, consider the example files from <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/tree/master/examples/e001?ref_type=heads" target="_blank" rel="noopener noreferrer">e001 (example 1)</a> in the repository.

In this example, all JSON files are placed in the project directory, which should look like this:
```
proyect_folder/
    |-- NSFNet.json
    |-- routes.json
    |-- simulator.hpp // only if using big header file!
```
@subsection bitrate-modulation-files Bit Rates

As mentioned earlier, you can optionally use a bit rate JSON file. This file defines the bit rates and the corresponding modulation formats, considering the required slots and their maximum reach (abstracted in units).

If this file is not provided, the library defaults to simulating 10, 40, 100, 400, and 1000 Gbp/s connections, all using BPSK modulation. For more details on its structure, refer to the BitRate Class or the <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/blob/master/bitrate/bitrate.json?ref_type=heads" target="_blank" rel="noopener noreferrer">example file in the repository</a>.

@warning
For [MB-EON](@ref MB) networks, the bit rate file has a different structure, as the reach varies depending on the band.

@section first-run Running Your First Simulation

Now that you have the necessary files, let's run a simple simulation.
1. Extract `main.cpp` from the <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/tree/master/examples/e001?ref_type=heads" target="_blank" rel="noopener noreferrer">e001</a> example and place it in your root folder.
2. Your project folder should now look like this:
```
proyect_folder/
    |-- main.cpp
    |-- NSFNet.json
    |-- routes.json
    |-- simulator.hpp // only if using big header file!
```
3. If you are using the [big header file installation](@ref quick-start), update the `#include` directive:
```cpp
#include <fnsim/simulator.hpp>
```
Change it to:
```cpp
#include "./simulator.hpp"
```
4. Finally, **compile and run** the program. On Linux, for example:
```bash
g++ main.cpp -o simulation.out && ./simulation.out
```
5. And *voilà*! You should see your first simulation running in the terminal. The output will provide **real-time updates** on simulation progress:
```
--- Flex Net Sim (0.8.0) ---

Nodes:              14                            
Links:              42                            
Goal Connections:   1000000                       
Lambda:             3                             
Mu:                 10                            
Algorithm:          FirstFit                      

+----------+----------+----------+----------+----------+----------+----------+
| progress | arrives  | blocking | time(s)  | Wald CI  | A-C. CI  | Wilson CI|
+----------+----------+----------+----------+----------+----------+----------+
|    5.0%  |  50000   |  0.0e+00 |       4  |  0.0e+00 |  5.5e-05 |  3.8e-05 |
|   10.0%  | 100000   |  4.0e-05 |       8  |  3.9e-05 |  4.8e-05 |  4.4e-05 |
|   15.0%  | 150000   |  3.3e-05 |      11  |  2.9e-05 |  3.5e-05 |  3.2e-05 |
|   20.0%  | 200000   |  5.0e-05 |      15  |  3.1e-05 |  3.4e-05 |  3.2e-05 |
|   25.0%  | 250000   |  4.4e-05 |      19  |  2.6e-05 |  2.8e-05 |  2.7e-05 |
|   30.0%  | 300000   |  4.3e-05 |      22  |  2.4e-05 |  2.5e-05 |  2.4e-05 |
|   35.0%  | 350000   |  4.6e-05 |      26  |  2.2e-05 |  2.4e-05 |  2.3e-05 |
|   40.0%  | 400000   |  4.0e-05 |      30  |  2.0e-05 |  2.1e-05 |  2.0e-05 |
|   45.0%  | 450000   |  3.8e-05 |      33  |  1.8e-05 |  1.9e-05 |  1.8e-05 |
|   50.0%  | 500000   |  3.8e-05 |      37  |  1.7e-05 |  1.8e-05 |  1.8e-05 |
|   55.0%  | 550000   |  3.6e-05 |      41  |  1.6e-05 |  1.7e-05 |  1.6e-05 |
|   60.0%  | 600000   |  3.7e-05 |      45  |  1.5e-05 |  1.6e-05 |  1.6e-05 |
|   65.0%  | 650000   |  3.7e-05 |      49  |  1.5e-05 |  1.5e-05 |  1.5e-05 |
|   70.0%  | 700000   |  4.0e-05 |      52  |  1.5e-05 |  1.5e-05 |  1.5e-05 |
|   75.0%  | 750000   |  4.1e-05 |      56  |  1.5e-05 |  1.5e-05 |  1.5e-05 |
|   80.0%  | 800000   |  4.4e-05 |      60  |  1.4e-05 |  1.5e-05 |  1.5e-05 |
|   85.0%  | 850000   |  4.5e-05 |      64  |  1.4e-05 |  1.5e-05 |  1.4e-05 |
|   90.0%  | 900000   |  4.2e-05 |      67  |  1.3e-05 |  1.4e-05 |  1.4e-05 |
|   95.0%  | 950000   |  4.3e-05 |      71  |  1.3e-05 |  1.4e-05 |  1.3e-05 |
|  100.0%  |1000000   |  4.4e-05 |      75  |  1.3e-05 |  1.3e-05 |  1.3e-05 |
```
Below is a description of each parameter displayed in the simulation output:  
| **Parameter**       | **Description** |
|--------------------|----------------|
| `Nodes`         | Number of nodes in the network topology. |
| `Links`         | Total number of links in the network topology. |
| `Goal Connections` | Total number of connection requests to be simulated. |
| `Lambda`        | Arrival rate of new connection requests. |
| `Mu`            | Departure rate of active connections. |
| `Algorithm`     | Resource allocation strategy used in the simulation. |
| `progress`      | Percentage of completed connection requests. |
| `arrives`       | Cumulative number of processed connection requests. |
| `blocking`      | Blocking probability: the fraction of failed connection attempts. |
| `time(s)`       | Total elapsed simulation time (in seconds). |
| `Wald CI`       | Confidence interval for blocking probability using the Wald method. |
| `A-C. CI`       | Confidence interval for blocking probability using the Agresti-Coull method. |
| `Wilson CI`     | Confidence interval for blocking probability using the Wilson method. |


<div class="section_buttons">
| Previous          |                              Next |
|:------------------|----------------------------------:|
| [Installation](@ref installation) | [Coding an Algorithm](@ref algorithm) |
</div>