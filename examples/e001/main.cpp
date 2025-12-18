
// Posible solution to Controller Allocator dilema: (I would need to have a version per networkType)

#define INC_ALLOC_SLOTS_CHAYA(demand, bitrate_idx, link_id, from, len, route_idx, link_idx) \
do { \
    Connection _connection = Connection(0, 0.0, bitRates[bitrate_idx], \
                                      demand->getSrc(), demand->getDst()); \
    connections->push_back(_connection); \
    size_t __from = (from); \
    size_t __len = (len); \
    _connection.addLink(link_id, __from, __from + __len); \
    /* Mark slots as occupied on the specific link in the route */ \
    for (size_t __s = __from; __s < __from + __len; ++__s) { \
        INC_LINK_IN_ROUTE((demand), (route_idx), (link_idx))->setSlot(__s, true); \
    } \
} while (0)

// Define certain macros depending on the simulation type? No INC_ needed.

// getDistanceAdaptive could hold the physical layer logic?
// Or something similar really

// Fiber class:
// - Few-Mode Fiber
// - Few-Mode Multi-Core Fiber
// - Standard Single-Mode Fiber
// - Hollow-Core Fiber
// - Multi-Core Fiber
// Link class would have a vector<Fiber> fibers; This way multi-fiber is possible
// with heterogeneous fiber types!
// Each fiber would have its own properties and vector length depending of its slots
// Implementation of physical layer would actually need this implementation.



/*********************************************************************************
 * This is an incremental allocation example
 *
 *
 *
 *
 *
 **********************************************************************************/

#include "/home/mirkozeta/flex-net-sim/build/simulator.hpp"
#include "/opt/gurobi1202/linux64/include/gurobi_c++.h"

Simulator sim;

std::vector<int> solveBitrateILP(const std::vector<std::shared_ptr<BitRate>>& bitRates,
                                 const Demand& demand) {
    try {
        GRBEnv env(true);
        env.set(GRB_IntParam_OutputFlag, 0);
        env.start();
        GRBModel model(env);
        model.set(GRB_IntParam_OutputFlag, 0);

        const int n = (int)bitRates.size();
        const double D = demand.getUnprovisionedBitRate();

        // Vars: x[i] = number of LPs of bitrate i
        std::vector<GRBVar> x(n);
        for (int i = 0; i < n; ++i)
            x[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_INTEGER);

        // Constraint: sum r_i x_i >= D (must satisfy demand)
        GRBLinExpr capacity = 0;
        for (int i = 0; i < n; ++i)
            capacity += bitRates[i]->getBitRate() * x[i];
        model.addConstr(capacity >= D);
        
        // Constraint: sum r_i x_i <= D + 100 (limit overprovisioning to 100 Gbps)
        model.addConstr(capacity <= D + 100.0);

        // ----- Pass 1: minimize number of LPs -----
        GRBLinExpr lpCount = 0;
        for (int i = 0; i < n; ++i) lpCount += x[i];
        model.setObjective(lpCount, GRB_MINIMIZE);
        model.optimize();
        if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL)
            return std::vector<int>(n, 0);

        int optCount = (int)std::llround(model.get(GRB_DoubleAttr_ObjVal));

        // ----- Pass 2: minimize overshoot among min-count solutions -----
        model.addConstr(lpCount == optCount);
        model.setObjective(capacity, GRB_MINIMIZE);
        model.optimize();
        if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL)
            return std::vector<int>(n, 0);

        // Extract
        std::vector<int> result(n, 0);
        for (int i = 0; i < n; ++i)
            result[i] = (int)std::llround(x[i].get(GRB_DoubleAttr_X));

        return result;

    } catch (const GRBException&) {
        return std::vector<int>(bitRates.size(), 0);
    }
}


BEGIN_INCREMENTAL_ALLOC_FUNCTION(Example1)
{
	FOR_EACH_DEMAND_SYMMETRIC {
		
		if (CURRENT_DEMAND->isProvisioned()) continue;
		
		// Solve ILP for current demand
		std::vector<int> allocation = solveBitrateILP(BITRATES, *CURRENT_DEMAND);

		for (size_t b = 0; b < NUMBER_OF_BITRATES; b++) {
			
			// Store bitrate value
			double bitRateValue = BITRATE_AT(b)->getBitRate();

			for (size_t n = 0; n < allocation[b]; ++n) {

				// Define control variables
				std::vector<bool> totalSlots;
				int currentNumberSlots;
				int currentSlotIndex;
				Route currentRoute;
				int modulation;
				int requiredSlots;
				bool routeSuccess = false;

				// Iterate through available routes
				for (size_t r = 0; r < INC_NUMBER_OF_ROUTES(CURRENT_DEMAND); r++) {

					if (routeSuccess) break;
					routeSuccess = false;

					if (r > 1) continue; // K = 1

					// Reset slot availability for the current route
					totalSlots = std::vector<bool>(INC_LINK_IN_ROUTE(CURRENT_DEMAND, r, 0)->getSlots(), false);

					// Iterate through each link in the route
					for (size_t l = 0; l < INC_NUMBER_OF_LINKS(CURRENT_DEMAND, r); l++) {
						// Update totalSlots vector with slot status from each link
						for (size_t s = 0; s < INC_LINK_IN_ROUTE(CURRENT_DEMAND, r, l)->getSlots(); s++) {
							totalSlots[s] = totalSlots[s] | INC_LINK_IN_ROUTE(CURRENT_DEMAND, r, l)->getSlot(s);
						}
					}

					// Search for a contiguous block of available slots
					currentNumberSlots = 0;
					currentSlotIndex = 0;

					currentRoute = INC_REQ_ROUTE(CURRENT_DEMAND, r);
					modulation = BITRATE_AT(b)->getDistanceAdaptive(currentRoute);

					if (modulation == -1)
						continue;

					requiredSlots = BITRATE_AT(b)->getNumberOfSlots(modulation);

					for (size_t s = 0; s < totalSlots.size(); s++) {
						if (totalSlots[s] == false) {
							currentNumberSlots++;
						} else {
							currentNumberSlots = 0;
							currentSlotIndex = s + 1;
						}
						if (currentNumberSlots == requiredSlots) {
							for (size_t l = 0; l < INC_NUMBER_OF_LINKS(CURRENT_DEMAND, r); l++) {
								INC_ALLOC_SLOTS(CURRENT_DEMAND, b, INC_LINK_IN_ROUTE_ID(CURRENT_DEMAND, r, l), currentSlotIndex, requiredSlots);
								INC_ALLOC_SLOTS(SYMMETRIC_DEMAND, b, INC_LINK_IN_ROUTE_ID(SYMMETRIC_DEMAND, r, l), currentSlotIndex, requiredSlots);

								// This should not be necesary!
								for (size_t s = currentSlotIndex; s < currentSlotIndex + requiredSlots; s++) {
									INC_LINK_IN_ROUTE(CURRENT_DEMAND, r, l)->setSlot(s, true);
									INC_LINK_IN_ROUTE(SYMMETRIC_DEMAND, r, l)->setSlot(s, true);
								}
							}
							// Allocate the same amount to both directions (symmetric)
							CURRENT_DEMAND->addAllocatedBitRate(bitRateValue);
							SYMMETRIC_DEMAND->addAllocatedBitRate(bitRateValue);
							
							routeSuccess = true;
							break; // Next route

						}
					}
				}
			}
		}
	}
}
END_INCREMENTAL_ALLOC_FUNCTION

BEGIN_INTERPERIOD_CALLBACK_FUNCTION
{
	// Implement inter-period logic here
	//std::cout << "Inter-period callback for period " << CURRENT_PERIOD << std::endl;

	// Traverse DEMANDS matrix
	FOR_EACH_DEMAND {
		// Update required bitrate
		CURRENT_DEMAND->setRequiredBitRate(0.0);
	}

	// Or set it by JSON file
	sim.updateDemandsFromFile("./demands/demands_1.json");
}
END_INTERPERIOD_CALLBACK_FUNCTION

int main()
{
	sim = Simulator(
			std::string("./DT17.json"),
			std::string("./DT17_routes.json"),
			std::string("./bitrates.json"),
			EON,
			INCREMENTAL_MODE);

	// Set simulation parameters
	sim.setNumberOfPeriods(10);

	// Set growth parameters
	sim.setSeedGrowth(505);
	sim.setBaseGrowthRate(0.30);
	sim.setBaseGrowthRateStdDev(0.1);
	
	// Set initial demands from file (for period 1)
	sim.setInitialDemandsFile("./demands/demands_init.json", true);

	// Modify growth factors manually
	for (int period = 1; period < sim.getNumberOfPeriods(); ++period) {
		//sim.setGrowthFactor(period, 1.35);
		continue;
	}

	USE_INCREMENTAL_ALLOC_FUNCTION(Example1, sim);
	//USE_INTERPERIOD_FUNCTION(sim);

	// Initialize the simulator
	sim.init();

	// Run the simulation
	sim.run("results");

	// Obtain the growth factors used per period
	//std::vector<double> growthFactors = sim.getGrowthValues();
	//for (size_t i = 0; i < growthFactors.size(); ++i)
	//{
	//	std::cout << "Growth factor for period " << i << ": " << std::fixed << std::setprecision(4) << growthFactors[i] << std::endl;
	//}

	return 0;
}