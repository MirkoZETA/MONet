
#include "/home/mirkozeta/flex-net-sim-copy/build/simulator.hpp"
#include "/opt/gurobi1202/linux64/include/gurobi_c++.h"

BEGIN_ALLOC_FUNCTION(Test)
{
  int testVar = 0;
}
END_ALLOC_FUNCTION

int main()
{
  int kPaths = 3;
  int numPeriods = 10;

  Simulator sim(
      std::string("../example_networks/DT-17.json"),
      std::string("../example_bitrates/basic_example.json"),
      std::string("../example_demands/DT-17.json"),
      kPaths);

  // Set simulation parameters
  sim.setNumberOfPeriods(numPeriods);

  // Set growth parameters
  sim.setSeedGrowthRate(505);
  sim.setBaseGrowthRate(0.30);
  sim.setGrowthRateStdDev(0.1);

  USE_ALLOC_FUNCTION(Test, sim);

  // Initialize the simulator
  sim.init();

  // Run the simulation
  sim.run(true);

  // Print growth Rates
  for (int period = 1; period < sim.getNumberOfPeriods(); ++period)
  {
    std::cout << "Growth rate for period "
              << period << ": " 
              << sim.getGrowthRates()[period] << std::endl;
  }
  return 0;
}