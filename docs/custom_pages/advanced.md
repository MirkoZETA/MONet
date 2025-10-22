@page advanced Advanced Usage

@tableofcontents

@note Before exploring advanced implementations, it is recommended to first review [Coding an Algorithm](@ref algorithm). That section explains the structure of the algorithm code, the library’s main macros, and provides a simple example to build upon.

This section presents **advanced implementations that extend the flexibility and functionality of the library**. It explores different strategies, optimizations, and methodologies that can be applied to address more complex scenarios. Additionally, it introduces the main elements and differences for **SDM-EONs** and **MB-EONs** scenarios, providing insights into their unique characteristics and implementation details.

@section reach-constraint Reach Constraints

The transmission range of a given modulation format is inherently limited due to **physical impairments**. The library manages these impairments through **reach tables**, specifically by defining reach values that must be provided in the `bitrates.json` file.

The following algorithm integrates **reach constraints** into the allocation process, ensuring a more realistic simulation by preventing transmissions from exceeding the maximum reach. To achieve this, we modify the allocation function to **track the total distance** of each route before attempting resource allocation.

1. To store the accumulated route length, we introduce the `routeLength` variable alongside the existing auxiliary variables:  
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit)
{
  int currentNumberSlots;
  int currentSlotIndex;
  int requiredSlots = REQ_SLOTS_FIXED;
  int routeLength; // New control variable
  std::vector<bool> totalSlots;
}
END_ALLOC_FUNCTION
```
2. When constructing `totalSlots`, we iterate over all the links of the current route. Instead of adding a separate iteration, we **efficiently compute** the total route length within the same loop:  
```cpp
  routeLength = 0;
  totalSlots = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(), false);
  for (int l = 0; l < NUMBER_OF_LINKS(r); l++) {
    routeLength += LINK_IN_ROUTE(r, l)->getLength(); // Sum link lengths
    for (int s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(); s++) {
      totalSlots[s] = totalSlots[s] | LINK_IN_ROUTE(r, l)->getSlot(s);
    }
  }
```
   By the end of this iteration, `routeLength` contains the total length of the current route.
3. To ensure the route is within the valid reach for the modulation format, we use `REQ_REACH_FIXED` (for fixed-rate scenarios with a single modulation format):  
```cpp
  if (routeLength > REQ_REACH_FIXED) continue;
```
If the route length exceeds the allowed reach, the algorithm **immediately skips** to the next available route, avoiding unnecessary computations.
@note In **flex-rate** scenarios, where multiple modulation formats exist, the reach should be accessed dynamically using `REQ_REACH(m)`, where `m` is the index of the modulation format.
4. The final implementation of the **First-Fit** algorithm with **reach constraints** is as follows:
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit)
{
  int currentNumberSlots;
  int currentSlotIndex;
  int routeLength; // new control variable
  int requiredSlots = REQ_SLOTS_FIXED;
  std::vector<bool> totalSlots;

  for (int r = 0; r < NUMBER_OF_ROUTES; r++) {

    routeLength = 0;
    totalSlots = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(), false);
    for (int l = 0; l < NUMBER_OF_LINKS(r); l++) {
      routeLength += LINK_IN_ROUTE(r, l)->getLength(); // sum link lengths
      for (int s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(); s++) {
        totalSlots[s] = totalSlots[s] | LINK_IN_ROUTE(r, l)->getSlot(s);
      }
    }

    // If reach not enough continue to next route
    if (routeLength > REQ_REACH_FIXED) continue;

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


With this modification, the algorithm ensures that only **valid routes within the transmission reach** are considered for allocation, preventing infeasible connections and improving the realism of the simulation.

@section flex-rate Flex-Rate
In a **flex-rate scenario**, an additional dimension is introduced where new connections may require **a variable number of slots** depending on the **selected modulation format**. Moreover, each modulation format has **different reach limitations**, which must be taken into account.  

To handle this, the **fixed-rate macros** `REQ_REACH_FIXED` and `REQ_SLOTS_FIXED` are **replaced** by `REQ_REACH(m)` and `REQ_SLOTS(m)`, where `m` corresponds to the **index of the modulation format** defined in the `bitrates.json` file.

1. Since the required number of slots now depends on the modulation format, we define `requiredSlots` dynamically instead of using a fixed value:
```cpp
  int currentNumberSlots;
  int currentSlotIndex;
  int routeLength;
  int requiredSlots; // Dynamically assigned
  std::vector<bool> totalSlots;
```
2. The outer loop iterates over all possible routes, while an additional loop ensures that each route is tested against all available modulation formats:
```cpp
  for (int r = 0; r < NUMBER_OF_ROUTES; r++) {
    for (int m = 0; m < NUMBER_OF_MODULATIONS; m++) {

      ...

      return ALLOCATED;
    }
  }
  return NOT_ALLOCATED;

```
3. Instead of using a single reach value, we now use `REQ_REACH(m)`, which returns the **maximum reach for modulation format `m`**. If the computed route length exceeds this value, the algorithm skips to the next modulation format:
```cpp
    for (int m = 0; m < NUMBER_OF_MODULATIONS; m++) { // Iterate over modulation formats
      if (routeLength > REQ_REACH(m)) continue; // Skip if reach is insufficient
    
      ...

    }

```
4. The required number of slots is now determined dynamically based on the selected modulation format. Higher-order modulations use fewer slots, while lower-order modulations require more:
```cpp
    for (int m = 0; m < NUMBER_OF_MODULATIONS; m++) { // Iterate over modulation formats
      if (routeLength > REQ_REACH(m)) continue;

      requiredSlots = REQ_SLOTS(m); // Get required slots for the current modulation format

      ...

    }
```
5. The final implementation of **First-Fit with flex-rate support** is as follows:
```cpp
BEGIN_ALLOC_FUNCTION(FirstFit)
{
  int currentNumberSlots;
  int currentSlotIndex;
  int routeLength;
  int requiredSlots;
  std::vector<bool> totalSlots;

  for (int r = 0; r < NUMBER_OF_ROUTES; r++) {

    routeLength = 0;
    totalSlots = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(), false);
    for (int l = 0; l < NUMBER_OF_LINKS(r); l++) {
      routeLength += LINK_IN_ROUTE(r, l)->getLength();
      for (int s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(); s++) {
        totalSlots[s] = totalSlots[s] | LINK_IN_ROUTE(r, l)->getSlot(s);
      }
    }

    for (int m = 0; m < NUMBER_OF_MODULATIONS; m++){

      if (routeLength > REQ_REACH(m)) continue;

      requiredSlots = REQ_SLOTS(m);
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
  }
  return NOT_ALLOCATED;
}
END_ALLOC_FUNCTION
```

@section multiple-loads Simulate Multiple Loads

In some cases, it may be useful to **simulate various network loads (Erlangs) within a single execution**. This can be accomplished by implementing a simple `for` loop that iterates over a range of `lambda` values.

Since the **allocation algorithm remains unchanged**, the only modifications occur in the **main function**, where different network loads are tested by adjusting the `lambda` value while keeping `mu` constant.

```cpp
int main(int argc, char* argv[]) {

  for (float lambda = 200; lambda < 355; lambda += 15.5) {
    Simulator sim =
      Simulator(std::string("./network/NSFNet.json"),
                std::string("./network/routes.json")
                std::string("./network/bitrates.json"));

    USE_ALLOC_FUNCTION(FirstFit, sim);

    // Parameters
    sim.setGoalConnections(1e6);
    sim.setLambda(lambda);
    sim.setMu(1); // Fixed Mu

    sim.init();
    sim.run();
  }

  return 0;
}
```
In this loop: 
- The **simulation runs multiple times**, each time with a different `lambda` value.
- The `lambda` value starts at `200` and increases in steps of `15.5` until it reaches `355`.
- The `mu` value **remains constant** at `1`, ensuring a controlled comparison across different loads.
- A new Simulator instance is created for each iteration to **ensure a clean state** before each run.

@subsection output-automation Automating the Output

To automate result collection, we can extend this implementation by **writing the blocking probability to a file.** Following the approach used in [Coding an Algorithm](@ref output), we retrieve the blocking probability with `sim.getBlockingProbability()` and save it to a file for further analysis:
```cpp
int main(int argc, char* argv[]) {

  // Setup output file
  std::ofstream outputFile("output.txt");

  for (float lambda = 200; lambda < 355; lambda += 15.5) {
    Simulator sim =
      Simulator(std::string("./network/NSFNet.json"),
                std::string("./network/routes.json")
                std::string("./network/bitrates.json"));

    USE_ALLOC_FUNCTION(FirstFit, sim);

    sim.setGoalConnections(1e6);
    sim.setLambda(lambda);
    sim.setMu(1);

    sim.init();
    sim.run();

    // Output to file
    outputFile << lambda << " " << sim.getBlockingProbability() << std::endl;
  }
  return 0;
}
```
With this approach, the simulation systematically records the blocking probability for each network load, allowing for efficient data collection and post-processing.

<!--
@section heterogeneous-capacity Heterogeneous Link Capacities
@todo implement and explain

@section translucent Translucent Optical Network
@todo implement and explain

@section opaque Opaque Optical Network
@todo implement and explain

@section sdm SDM-EONs
@todo
Show and explain SDM, and key diferences (macros, JSON), define subsections!

@subsection multi-mode Few-Mode/Multi-Mode Fiber
@todo implement, explain

@subsection multi-core Multi-Core Fiber
@todo implement, explain
-->

@section mb MB-EONs
@todo
Explain MB implementation: key diferences (macros, JSON, main function, etc.).

```cpp
// Global variables
std::vector<char> bandOrder;
bandOrder.assign({'C', 'L', 'S', 'E'});

BEGIN_ALLOC_FUNCTION(FirstFit) {

  // Control variables
  int routeLength;
  
  std::map<char, std::vector<bool>> slotState; // Slot state for each band
  int requiredSlots; // Number of required slots
  int totalSlots; // Total number of slots available in the route
  int index; // Index of the current slot
  
  for (size_t r = 0; r < NUMBER_OF_ROUTES; r++){ // RMLBSA: -> R (For each Route).

    // First we get the route length
    routeLength = 0;
    for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){
      routeLength += LINK_IN_ROUTE(r, l)->getLength();
    }
    for (size_t m = 0; m < NUMBER_OF_MODULATIONS; m++){ // RMLBSA: -> ML (For each Modulation Level).
      for (size_t b = 0; b < NUMBER_OF_BANDS(r, 0); b++){ // RMLBSA -> B (For each Band)

        // Check if the route length is greater than the reach of the modulation level
        if (REQ_REACH_MB(m, REQ_POS_BANDS(m)[bandOrder[b]]) < routeLength) continue;

        // Get the number of required slots given by the modulation level
        requiredSlots = REQ_SLOTS_MB(m, REQ_POS_BANDS(m)[bandOrder[b]]);

        // Get the representative vector of the slots in the route (r)
        // only if it has not been previously calculated (just for eficiency)
        if (slotState.find(bandOrder[b]) == slotState.end()){
          slotState[bandOrder[b]] = std::vector<bool>(LINK_IN_ROUTE(r, 0)->getSlots(bandOrder[b]), false);

          // Fill the vector with the slots of the route
          for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){
            for (size_t s = 0; s < LINK_IN_ROUTE(r, l)->getSlots(bandOrder[b]); s++){
              slotState[bandOrder[b]][s] = slotState[bandOrder[b]][s] | LINK_IN_ROUTE(r, l)->getSlot(s, bandOrder[b]);
            }
          }
        }

        // Given the representative vector of the slots in the route (r) for the band (b)
        // we look for the block of slots that satisfy the required slots
        totalSlots = 0;
        index = 0;

        for (size_t s = 0; s < slotState[bandOrder[b]].size(); s++){ // RMLBSA -> S (For each Slot)
          if (slotState[bandOrder[b]][s] == false){
            totalSlots++;
          } else {
            totalSlots = 0;
            index = s + 1;
          }
          if (totalSlots == requiredSlots){
            // Assign the slots to the connection
            for (size_t l = 0; l < NUMBER_OF_LINKS(r); l++){
              //std::cout << "Allocating slots in link " << LINK_IN_ROUTE_ID(r, l) << " band " << bandOrder[b] << " index " << index << " requiredSlots " << requiredSlots << std::endl;
              ALLOC_SLOTS_MB(LINK_IN_ROUTE_ID(r, l), bandOrder[b], index, requiredSlots);
            }
            return ALLOCATED;
          }
        }
      }
    }
  }
  return NOT_ALLOCATED;
}
END_ALLOC_FUNCTION

// For the moment, MB requires to define the unallocation function (can be empty)
BEGIN_UNALLOC_CALLBACK_FUNCTION{
}
END_UNALLOC_CALLBACK_FUNCTION
```

<div class="section_buttons">
|                        Read Previous |
|---------------------------------:|
| [Coding an Algorithm](@ref algorithm) |
</div>