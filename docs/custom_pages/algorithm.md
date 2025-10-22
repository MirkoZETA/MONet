@page algorithm Coding an Algorithm

@tableofcontents

@note @note Before diving into algorithm coding, make sure you have completed the first simulation run in [Getting Started](@ref starting), as it covers **key aspects of the library's setup and execution**.

Now, let's get to the fun part. After our first simulation run in [Getting Started](@ref starting), we didn’t know exactly what we ran, and we won’t unless we look at the `main.cpp` file, where all the magic happens. So, let’s do that.

Following the <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/tree/master/examples/e001?ref_type=heads" target="_blank" rel="noopener noreferrer">e001</a> example, the`main.cpp` file contains, at first glance, **two key components**:
1. The [Allocation Algorithm](@ref anatomy) macros, which define the **First-Fit allocation function**:
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit) {
  ...
}
END_ALLOC_FUNCTION
```
2. And the [Main Function](@ref main-function), a fundamental component of any standard C++ program, which **initializes and runs the simulator**:
```cpp
int main(int argc, char* argv[]) {
  Simulator sim =
    Simulator(std::string("NSFNet.json"),
              std::string("routes.json"));

  USE_ALLOC_FUNCTION(FirstFit, sim);
  sim.setGoalConnections(1e6);
  sim.init();
  sim.run();

  return 0;
}
```

@section main-function Main Function

Starting with the main function, you’ll notice several key steps:
1. First, the **Simulator object is created**, specifying the required JSON files:
```cpp
Simulator sim =
  Simulator(std::string("NSFNet.json"),
            std::string("routes.json"));
```
2. Next, the desired **allocation function is assigned** to the simulator (in this case `FirstFit`):
```cpp
USE_ALLOC_FUNCTION(FirstFit, sim);
```
3. The **number of connections to be simulated is set**:
```cpp
sim.setGoalConnections(1e6);
```
4. The simulator is prepared for execution:
```cpp
sim.init();
```
5. And finally, the **simulation process starts**:
```cpp
sim.run();
```

Beyond these basic parameters, you can further customize the simulation. For example, adjusting the arrival rate (`lambda`) and departure rate (`mu`):
```cpp
sim.setLambda(double lambda);
sim.setMu(double mu);
```
Setting a random seed for reproducibility:
```cpp
sim.setSeedArrive(unsigned int seed)
```
And much more! 🔥

@note **For a full list of parameters**, check the Simulator class documentation.

@subsection custom-parameters Customizing Simulation Parameters

Now that we understand the basic setup, let's make some modifications.

1. To keep things tidy, let's create a `network/` folder for all JSON files:
```
proyect_folder/
  |-- network/
    |-- NSFNet.json
    |-- routes.json
  |-- main.cpp
  |-- simulator.hpp
```
@note You can replace the current network files with any available in the <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/tree/master/networks?ref_type=heads" target="_blank" rel="noopener noreferrer">networks folder</a> of the repository. Make sure to **use matching network and routes**.
2. **To gain more control over traffic profiles**, let's copy the <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/blob/master/bitrate/bitrate.json?ref_type=heads" target="_blank" rel="noopener noreferrer">`bitrates.json` file from the repository</a> and place it inside the `network/` folder:
```
proyect_folder/
  |-- network/
    |-- bitrates.json
    |-- NSFNet.json
    |-- routes.json
  |-- main.cpp
  |-- simulator.hpp
```
Feel free to modify `bitrates.json`—you can add new bit rates, define additional modulation formats, or adjust slot requirements.
3. To include the new bit rates and update the new locations, **modify the simulator initialization**:
```cpp
  Simulator sim =
    Simulator(std::string("./network/NSFNet.json"),
              std::string("./network/routes.json")
              std::string("./network/bitrates.json"));
```
4. Finally, let's **tweak some settings**:
    - Reduce the number of connections.
```cpp
sim.setGoalConnections(1e5);
```
    - Increase connection arrival rate.
```cpp
sim.setLambda(100);
```
    - Lower departure rate.
```cpp
sim.setMu(1);
```
    - Seeds for reproducibility.
```cpp
sim.setSeedSrc(123);
sim.setSeedDst(456);
sim.setSeedBitRate(789);
```
    - Adjust confidence interval.
```cpp
sim.setConfidence(0.2);
```
5. After making these changes, running the simulation will yield results like:
```
--- Flex Net Sim (0.8.0) ---

Nodes:              14                            
Links:              42                            
Goal Connections:   100000                        
Lambda:             100                           
Mu:                 1                             
Algorithm:          FirstFit                      

+----------+----------+----------+----------+----------+----------+----------+
| progress | arrives  | blocking | time(s)  | Wald CI  | A-C. CI  | Wilson CI|
+----------+----------+----------+----------+----------+----------+----------+
|    5.0%  |   5000   |  2.9e-01 |       1  |  1.6e-03 |  1.6e-03 |  1.6e-03 |
|   10.0%  |  10000   |  2.9e-01 |       1  |  1.2e-03 |  1.2e-03 |  1.2e-03 |
|   15.0%  |  15000   |  2.9e-01 |       2  |  9.4e-04 |  9.4e-04 |  9.4e-04 |
|   20.0%  |  20000   |  2.9e-01 |       3  |  8.1e-04 |  8.1e-04 |  8.1e-04 |
|   25.0%  |  25000   |  2.9e-01 |       3  |  7.3e-04 |  7.3e-04 |  7.3e-04 |
|   30.0%  |  30000   |  2.9e-01 |       4  |  6.7e-04 |  6.7e-04 |  6.7e-04 |
|   35.0%  |  35000   |  2.9e-01 |       4  |  6.2e-04 |  6.2e-04 |  6.2e-04 |
|   40.0%  |  40000   |  2.9e-01 |       5  |  5.8e-04 |  5.8e-04 |  5.8e-04 |
|   45.0%  |  45000   |  2.9e-01 |       6  |  5.4e-04 |  5.4e-04 |  5.4e-04 |
|   50.0%  |  50000   |  2.9e-01 |       6  |  5.2e-04 |  5.2e-04 |  5.2e-04 |
|   55.0%  |  55000   |  2.9e-01 |       7  |  4.9e-04 |  4.9e-04 |  4.9e-04 |
|   60.0%  |  60000   |  2.9e-01 |       8  |  4.7e-04 |  4.7e-04 |  4.7e-04 |
|   65.0%  |  65000   |  2.9e-01 |       8  |  4.5e-04 |  4.5e-04 |  4.5e-04 |
|   70.0%  |  70000   |  2.9e-01 |       9  |  4.4e-04 |  4.4e-04 |  4.4e-04 |
|   75.0%  |  75000   |  2.9e-01 |       9  |  4.2e-04 |  4.2e-04 |  4.2e-04 |
|   80.0%  |  80000   |  2.9e-01 |      10  |  4.1e-04 |  4.1e-04 |  4.1e-04 |
|   85.0%  |  85000   |  2.9e-01 |      11  |  4.0e-04 |  4.0e-04 |  4.0e-04 |
|   90.0%  |  90000   |  2.9e-01 |      11  |  3.8e-04 |  3.8e-04 |  3.8e-04 |
|   95.0%  |  95000   |  2.9e-01 |      12  |  3.7e-04 |  3.7e-04 |  3.7e-04 |
|  100.0%  | 100000   |  2.9e-01 |      13  |  3.6e-04 |  3.6e-04 |  3.6e-04 |
```
You'll notice:
    - The simulation **runs faster** since fewer connections were simulated.
    - **Blocking probability is higher** due to the increased arrival rate (`lambda`) compared to the departure rate (`mu`).
    - **The confidence intervals are wider** because we set a high alpha value (20% confidence).

@section anatomy Anatomy of an Allocation Algorithm

The previous section focused on the [Main Function](@ref main-function), explaining its role in initializing and running the simulator, assigning the allocation algorithm, and setting simulation parameters.

In this section, we examine the second key component of a typical `main.cpp` file using the **Flex Net Sim** library: the `BEGIN_ALLOC_FUNCTION` / `END_ALLOC_FUNCTION` block. Here, the **allocation algorithm is defined**, determining how network resources are assigned to incoming connection requests.

This block consists of two macros: `BEGIN_ALLOC_FUNCTION` and `END_ALLOC_FUNCTION`. Each time a connection request arrives, the library invokes your algorithm through these macros. The logic implemented here decides **which spectral resources will be used to allocate the connection**, effectively determining **where and how connections are placed within the network**.

The allocation algorithm block is structured as follows:
```cpp
BEGIN_ALLOC_FUNCTION(identifier)
{
  // your algorithm logic goes here
}
END_ALLOC_FUNCTION
```
The `BEGIN_ALLOC_FUNCTION` macro receives an `identifier`, which serves as **the name of the algorithm and is used to assign it to the Simulator object**. This allows multiple algorithms to be defined, enabling the user to switch between them as needed:
```cpp
BEGIN_ALLOC_FUNCTION(alg1)
{
  // logic of alg1
}
END_ALLOC_FUNCTION

BEGIN_ALLOC_FUNCTION(alg2)
{
  // logic of alg2
}
END_ALLOC_FUNCTION

BEGIN_ALLOC_FUNCTION(alg3)
{
  // logic of alg3
}
END_ALLOC_FUNCTION
```
Once an algorithm is defined, it must communicate whether a connection request has been successfully allocated or not. This is achieved using the macros `ALLOCATED` and `NOT_ALLOCATED`, **ensuring the simulator correctly registers the outcome of each request**:
```cpp
BEGIN_ALLOC_FUNCTION(algorithm_alias)
{
  if (suitable_lightpath_found) {
    return ALLOCATED;
  }
  return NOT_ALLOCATED;
}
END_ALLOC_FUNCTION
```

To complete the integration, **the algorithm must be assigned** in the [Main Function](@ref main-function) using `USE_ALLOC_FUNCTION`:
```cpp
USE_ALLOC_FUNCTION(identifier, sim);
```
@warning
The allocation algorithm **must be declared before the `main` function**.

@section basic-macros Basic Macros

To simplify the development process **the library incorporates the use of macros**. These serve as tools to help you efficiently define and manage the behavior of your allocation strategy within the code:

1. The most fundamental macros are the `BEGIN_ALLOC_FUNCTION` / `END_ALLOC_FUNCTION` pair, which **define the allocation algorithm block**:
```cpp
BEGIN_ALLOC_FUNCTION(algorithm_alias)
{
  // your algorithm logic goes here
}
END_ALLOC_FUNCTION
```
2. `ALLOCATED` and `NOT_ALLOCATED` allow us to communicate to the simulation library **whether the connection has been successfully allocated or not**:
```cpp
BEGIN_ALLOC_FUNCTION(algorithm_alias)
{
  if (suitable_lightpath_found) {
    return ALLOCATED;
  }
  return NOT_ALLOCATED;
}
END_ALLOC_FUNCTION
```
3. Get the **number of routes available** between the source and destination of the current request (returns `int`):
```cpp
NUMBER_OF_ROUTES;
```
4. Get the **number of modulation formats available** for the requested bit rate (returns `int`):
```cpp
NUMBER_OF_MODULATIONS;
```
5. Get the **number of links** of a given route `r` (returns `int`):
```cpp
NUMBER_OF_LINKS(r);
```
@note This value depends on the number of routes precomputed in the `routes.json` file.
6. Retrieve the **Link object** corresponding to the link at index `l` in route `r`:
```cpp
LINK_IN_ROUTE(r,l);
```
@note The returned object provides access to various methods useful for algorithm definition.
7. Retreive the **ID of the link** at index `l` in route `r`:
```cpp
LINK_IN_ROUTE_ID(r,l);
```
8. Retrieve the **number of slots required** to transmit the requested bit rate. Optionally, specify the modulation format at index `m` (returns `int`):  
```cpp
REQ_SLOTS_FIXED;	// Uses modulation at index 0 (fixed-rate)
REQ_SLOTS(m);	// Useful for flex-rate systems
```
9. Finally, to allocate `n` slots on the link at index `l` within the route `r`, starting from slot index `s` and spanning `s + n - 1`:
```cpp
ALLOC_SLOTS(LINK_IN_ROUTE_ID(r, l), s, n)
```

With this set of macros, you can implement **a wide range of allocation algorithms, from basic to advanced**, including the example provided in the next section.


@note For a **complete list of available macros**, refer to the [Full List of Macros](@ref full-macros).

@section first-fit First-Fit Implementation

Let's put everything together with a classic example: the **First-Fit** algorithm.

This section demonstrates how to implement **First-Fit** in **Flex Net Sim**, a fundamental allocation strategy that selects the first available spectrum block that meets the connection’s requirements.

@note
The provided example is just one possible approach, but it highlights the essential steps and concepts needed to define a working allocation algorithm within the framework.

@subsection assumptions Assumptions

Before coding the algorithm, we make the following assumptions to keep the implementation as simple as possible:  

- We are considering an **RSA algorithm**, meaning the process follows **Routing → Spectrum Assignment**.  
- We assume **a single modulation format**, i.e., fixed-rate transmission (consistent with <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/blob/master/bitrate/bitrate.json?ref_type=heads" target="_blank" rel="noopener noreferrer">`bitrates.json`</a>).  
- All network links have the **same capacity** (aligned with <a href="https://gitlab.com/DaniloBorquez/flex-net-sim/-/blob/master/examples/e001/NSFNet.json?ref_type=heads" target="_blank" rel="noopener noreferrer">`NSFNet.json`</a>).  
- We operate in **a transparent optical network**, meaning signals propagate without optical-electrical-optical (OEO) conversion at intermediate nodes. As a result, we must **comply with the continuity constraint**, in addition to the **contiguity constraint**.

@subsection code Code

Now that we have established the key assumptions and constraints, let's break down the code step by step.

1. The algorithm begins with the `BEGIN_ALLOC_FUNCTION` and `END_ALLOC_FUNCTION` macros, which define the function's scope. The identifier `FirstFit` is assigned as the algorithm name:
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit)
{
  ...
}
END_ALLOC_FUNCTION
```
2. In the first lines of code, we define auxiliary variables that will help us track slot availability:
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit)
{
  int currentNumberSlots;
  int currentSlotIndex;
  int requiredSlots = REQ_SLOTS_FIXED;
}
END_ALLOC_FUNCTION
```
Since the current bit rate configuration file ([`bitrates.json`](https://gitlab.com/DaniloBorquez/flex-net-sim/-/blob/master/bitrate/bitrate.json?ref_type=heads)) contains only one modulation format (BPSK), we assume a **fixed-rate** scenario and use `REQ_SLOTS_FIXED`.
@note In scenarios where multiple modulation formats are available, the required slot count must be determined dynamically based on the current modulation format using `REQ_SLOTS(m)`.
3. Aditionally we define the `totalSlots` vector:
```cpp
std::vector<bool> totalSlots;
```
This vector will represent **the slot availability across all links in a given route**. It is crucial for ensuring continuity constraints in the allocation process.
4. The outer loop of our algorithm iterates over all possible routes between the **source and destination nodes** using the `NUMBER_OF_ROUTES` macro:
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit)
{
  int currentNumberSlots;
  int currentSlotIndex;
  int requiredSlots = REQ_SLOTS_FIXED;
  std::vector<bool> totalSlots;

  for (int r = 0; r < NUMBER_OF_ROUTES; r++) {
    ...
  }
}
END_ALLOC_FUNCTION
```
5. Now, let's make use of the totalSlots vector we defined earlier. This vector will help us track slot availability across all links in a given route, ensuring compliance with the **continuity constraint**. First, we initialize `totalSlots` to match the slot size of any link in the route (since all links have the same capacity). Then, we iterate over each link in the route and update the vector accordingly:
```cpp
totalSlots = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(), false);
for (int l = 0; l < NUMBER_OF_LINKS(r); l++) {
  for (int s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(); s++) {
    totalSlots[s] = totalSlots[s] | LINK_IN_ROUTE(r, l)->getSlot(s);
  }
}
```
With the help of `LINK_IN_ROUTE(r,l)`, we can retrieve link properties such as `getSlots()`, which returns the total number of slots in the link, and `getSlot(s)`, which returns true if slot s is occupied and false if it's available. To aggregate this information across all links, we perform a **bitwise OR** operation between `totalSlots[s]` and the status of the slot in each link, ensuring that **`totalSlots` accurately reflects slot availability across the entire route**.
6. The final task is to **find a contiguous block of available slots** that meets the request's slot requirement. To achieve this, we use the auxiliary variables `currentNumberSlots` (counting contiguous slots) and `currentSlotIndex` (tracking the start index of an available block). We iterate through totalSlots and check for available blocks:
```cpp
currentNumberSlots = 0;
currentSlotIndex = 0;
for (int s = 0; s < totalSlots.size(); s++) {
  if (totalSlots[s] == false) {
    currentNumberSlots++;
  }
  else {
    currentNumberSlots = 0;
    currentSlotIndex = s + 1;
  }
  if (currentNumberSlots == requiredSlots) {
    for (int l = 0; l < NUMBER_OF_LINKS(r); l++) {
      ALLOC_SLOTS(LINK_IN_ROUTE_ID(r, l), currentSlotIndex, requiredSlots);
    }
    return ALLOCATED;
  }
}
```
If a **suitable slot block is found**, the function returns `ALLOCATED`. If no valid block is found after checking all routes, it returns `NOT_ALLOCATED`.
7. The complete implementation of the First-Fit algorithm is as follows:
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit)
{
  int currentNumberSlots;
  int currentSlotIndex;
  int requiredSlots = REQ_SLOTS_FIXED;
  std::vector<bool> totalSlots;

  for (int r = 0; r < NUMBER_OF_ROUTES; r++) {

    totalSlots = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(), false);
    for (int l = 0; l < NUMBER_OF_LINKS(r); l++) {
      for (int s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(); s++) {
        totalSlots[s] = totalSlots[s] | LINK_IN_ROUTE(r, l)->getSlot(s);
      }
    }

    currentNumberSlots = 0;
    currentSlotIndex = 0;

    for (int s = 0; s < totalSlots.size(); s++) {
      if (totalSlots[s] == false) {
        currentNumberSlots++;
      }
      else {
        currentNumberSlots = 0;
        currentSlotIndex = s + 1;
      }
      if (currentNumberSlots == requiredSlots) {
        for (int l = 0; l < NUMBER_OF_LINKS(r); l++) {
          ALLOC_SLOTS(LINK_IN_ROUTE_ID(r, l), currentSlotIndex, requiredSlots);
        }
        return ALLOCATED;
      }
    }
  }
  return NOT_ALLOCATED;
}
END_ALLOC_FUNCTION
```

@remark This implementation of **First-Fit** is a simple yet effective approach to spectrum allocation. However, **this template can be extended to achieve much more**. For instance, with minor modifications, [reach constraints](@ref reach-constraint) can be incorporated, or enhancements like [flex-rate](@ref flex-rate), which supports multiple modulation formats, can be introduced. **This and other more advanced implementations** are explored in [Advanced Usage](@ref advanced).  


@subsection output Output Results

Once the simulation has been set up, it is important to **retrieve and analyze the results**. The simulator provides built-in methods to obtain relevant performance metrics, such as the **blocking probability**. These results can be printed to the console for quick inspection or saved to a file for further analysis.

Since the **allocation algorithm remains unchanged**, the only required modifications occur in the **main function**.

1. To display the blocking probability in the console after the simulation finishes, use `sim.getBlockingProbability()`:
```cpp
int main(int argc, char* argv[]) {
  Simulator sim =
    Simulator(std::string("./network/NSFNet.json"),
              std::string("./network/routes.json")
              std::string("./network/bitrates.json"));

  USE_ALLOC_FUNCTION(FirstFit, sim);
  sim.setGoalConnections(1e6);
  sim.init();
  sim.run();

  // Print blocking probability to console
  std::cout << "Blocking Probability: " << sim.getBlockingProbability() << std::endl;

  return 0;
}
```
2. Alternatively, the results can be written to a file to facilitate later analysis:
```cpp
int main(int argc, char* argv[]) {
  Simulator sim =
    Simulator(std::string("./network/NSFNet.json"),
              std::string("./network/routes.json")
              std::string("./network/bitrates.json"));

  USE_ALLOC_FUNCTION(FirstFit, sim);
  sim.setGoalConnections(1e6);
  sim.init();
  sim.run();

  // Write blocking probability to file
  outputFile << "Blocking Probability: " << sim.getBlockingProbability() << std::endl;

  return 0;
}
```

By implementing this, simulation results can be inspected directly in the terminal or stored for further processing, enabling deeper performance evaluations.

<div class="section_buttons">
| Previous          |                              Next |
|:------------------|----------------------------------:|
| [Getting Started](@ref starting) | [Advanced Usage](@ref advanced) |
</div>