// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/sim/simulator.hpp"
#include "catch.hpp"

/**
 * Test Algorithm: First-Fit 100G
 *
 * This test installs only **100G** lightpaths using a simple “first-fit” strategy:
 * we evaluate candidate routes in order and select the first slot block that fits,
 * preferring bands in the order C → L → S → E. 
 *
 * Assumptions:
 * - **Symmetric Provisioning:** every lightpath placed from A→B must also be
 *   placed from B→A with identical parameters.
 * - **Bidirectional Network:** every link A→B has a corresponding B→A link with
 *   the same characteristics (capacity, bands, cores, modes, etc.).
 * - **Continuity/Contiguity:** the same band, core, mode, and a contiguous slot
 *   block must be used consistently across all hops and mirrored in the reverse
 *   direction.
 * - **Fixed Bitrate:** only 100G lightpaths are placed.
 * - **Adaptive Modulation:** the best available modulation format is selected
 *   for each lightpath based on its characteristics.
 * - **Single Fiber per Link:** only one fiber per link is considered (`fiberIdx = 0`);
 *   additional fibers, if present, are ignored (no fiber switching).
 *
 * The example network is **heterogeneous**: different links may support different
 * bands, cores, modes, or slot counts. Before placing a lightpath, we verify that
 * the chosen band/core/mode is supported on **every hop in both directions**, and
 * that a common contiguous slot block is available end-to-end. If so, the lightpath
 * is placed; otherwise, the algorithm tries the next available option.
 */
BEGIN_ALLOC_FUNCTION(FirstFit_100G)
{
	// Fixed parameters for this algorithm
	// We always place 100G lightpaths
	auto bitRate = GET_BITRATE_BY_VALUE(100.0);
	auto bitRateIdx = GET_BITRATE_IDX_BY_VALUE(100.0);

	// Counter of placed lightpaths on this run
	int placedLightpaths = 0;

	FOR_EACH_DEMAND_SYMMETRIC { // For each demand (symmetric)

		if (CURRENT_DEMAND.isProvisioned()) continue; // Skip if already provisioned

		// Control variables
		std::vector<bool> totalSlots;
		int currentNumberSlots;
		int currentSlotIndex;

		// Store some variables for easier access
		auto SRC = CURRENT_DEMAND.getSrc();
		auto DST = CURRENT_DEMAND.getDst();

		// How many 100G lightpaths we will need to allocate
		int neededLPs = (int)std::ceil(CURRENT_DEMAND.getUnprovisionedCapacity() / 100.0f);

		for (int lp = 0; lp < neededLPs; lp++) { // For each 100G lightpath we need

			std::vector<fns::Band> bandOrder = { // Preferred band order: C, L, S, E
					fns::Band::C, fns::Band::L, fns::Band::S, fns::Band::E};

			for (size_t r = 0; r < GET_NUM_ROUTES(SRC, DST); r++) { // For each Route

				std::vector<int> selectedFibersIdx = // We will use single fiber (always index 0)
						std::vector<int>(GET_NUM_LINKS(SRC, DST, r), 0);

				for (auto &band : bandOrder) { // For each Band in preferred order

					// Check if the band exists across all fibers of index 0
					bool bandExists = true;
					for (size_t l = 0; l < GET_NUM_LINKS(SRC, DST, r); l++) {
						auto bands = GET_BANDS(SRC, DST, r, l, selectedFibersIdx[l]);
						if (std::find(bands.begin(), bands.end(), band) == bands.end()) {
							bandExists = false;
							break;
						}
					}
					if (!bandExists) continue;

					// Get best the 100G modulation format for this route and band
					int bestModulationIdx = GET_BEST_ADAPTIVE_MF(SRC, DST, bitRateIdx, r, band);
					if (bestModulationIdx == -1) continue; // No modulation found for this band

					size_t minCores = GET_MIN_NUM_CORES(SRC, DST, r, selectedFibersIdx);

					for (size_t c = 0; c < minCores; c++) { // For each Core (assuming continuity)

						size_t minModes = GET_MIN_NUM_MODES(SRC, DST, r, selectedFibersIdx, c, band);

						for (size_t m = 0; m < minModes; m++) { // For each Mode (assuming continuity)

							// Initialize totalSlots vector with all slots free (false)
							size_t minSlots = GET_MIN_NUM_SLOTS(SRC, DST, r, selectedFibersIdx, c, band, m);

							totalSlots = std::vector<bool>(minSlots, false); // Remember, minimum available across all links

							for (size_t l = 0; l < GET_NUM_LINKS(SRC, DST, r); l++) {
								// Now we OR the slots of this link with the totalSlots vector
								for (size_t s = 0; s < minSlots; s++) {
									totalSlots[s] = totalSlots[s] | (GET_SLOT(SRC, DST, r, l, selectedFibersIdx[l], c, band, m, s) != -1);
								} // Next slot
							} // Next link

							// Get required slots for 100G with this modulation
							int requiredSlots = GET_REQ_SLOTS(bitRateIdx, bestModulationIdx, band);

							// At this point we have a representative vector of all slots in the route
							// We can now search for a contiguous block of free slots
							currentNumberSlots = 0;
							currentSlotIndex = 0;

							for (size_t s = 0; s < totalSlots.size(); s++) {
								if (totalSlots[s] == false) {
									currentNumberSlots++;
								}
								else {
									currentNumberSlots = 0;
									currentSlotIndex = s + 1;
								}
								if (currentNumberSlots == requiredSlots) {

									// Now that we found a contiguous block of free slots:
									// we create a connection object (two if working with symmetric demands)
									auto newConnection = NEW_CONNECTION(bitRate, SRC, DST);
									auto newConnectionMirror = NEW_CONNECTION(bitRate, DST, SRC);

									// Populate the connection with the route data
										const size_t numLinks = GET_NUM_LINKS(SRC, DST, r);
										for (size_t l = 0; l < numLinks; l++) {
											auto linkFwd = GET_LINK_AT(SRC, DST, r, l);
											ADD_LINK_TO_CONNECTION(/*connection object=*/newConnection,
															 /*linkId=*/linkFwd,
															 /*fiber=*/selectedFibersIdx[l],
															 /*core=*/c, /*band=*/band, /*mode=*/m,
															 /*startSlot=*/currentSlotIndex,
															 /*len=*/requiredSlots);
										}

										// Mirror: reverse the forward hops and map each hop to its opposite-direction link.
										for (size_t mirrorIdx = 0; mirrorIdx < numLinks; ++mirrorIdx) {
											const size_t fwdIdx = numLinks - 1 - mirrorIdx;
											auto linkFwd = GET_LINK_AT(SRC, DST, r, fwdIdx);
											auto linkRev = this->network->getLink(linkFwd->getDst(), linkFwd->getSrc());
											REQUIRE(linkRev);
											ADD_LINK_TO_CONNECTION(/*connection object=*/newConnectionMirror,
															 /*linkId=*/linkRev,
															 /*fiber=*/selectedFibersIdx[fwdIdx],
															 /*core=*/c, /*band=*/band, /*mode=*/m,
															 /*startSlot=*/currentSlotIndex,
															 /*len=*/requiredSlots);
										}
									// Allocate the connections
									ALLOCATE_CONNECTION(newConnection);
									ALLOCATE_CONNECTION(newConnectionMirror);

									// Go to the next lightpath
									goto next_lightpath;
								} // Placement successful
							} // Next slot
						} // Next mode
					} // Next core
				} // Next band
			} // Next route
			next_lightpath: placedLightpaths++;
		} // Next lightpath
	} // Next demand
}
END_ALLOC_FUNCTION

BEGIN_CALLBACK_FUNCTION(TestCallback)
{
	// Do nothing, just test
}
END_CALLBACK_FUNCTION

TEST_CASE("Constructor (Simulator)")
{

	CHECK_NOTHROW(Simulator());

	CHECK_NOTHROW(
			Simulator("../examples/example_networks/5_node_example.json",
								"../examples/example_networks/5_node_example_routes.json"));

	CHECK_NOTHROW(
			Simulator("../examples/example_networks/5_node_example.json",
								"../examples/example_networks/5_node_example_routes.json",
								"../examples/example_bitrates/basic_example.json"));

	CHECK_NOTHROW(
			Simulator("../examples/example_networks/5_node_example.json",
								"../examples/example_networks/5_node_example_routes.json",
								"../examples/example_bitrates/basic_example.json",
								"../examples/example_demands/5_node_example_demands.json"));

	CHECK_NOTHROW(
			Simulator("../examples/example_networks/5_node_example.json"));

	CHECK_NOTHROW(
			Simulator("../examples/example_networks/5_node_example.json", 5));

	CHECK_NOTHROW(
			Simulator("../examples/example_networks/5_node_example.json",
								"../examples/example_bitrates/basic_example.json", 5));

	CHECK_NOTHROW(
			Simulator("../examples/example_networks/5_node_example.json",
								"../examples/example_bitrates/basic_example.json",
								"../examples/example_demands/5_node_example_demands.json", 5));
}

TEST_CASE("Demands file supports node labels with spaces") {
	Simulator s(
			"../test/test_label_spaces_network.json",
			"../test/test_label_spaces_routes.json",
			"../examples/example_bitrates/basic_example.json",
			"../test/test_label_spaces_demands.json");

	auto* d01 = s.getDemand(0, 1);
	REQUIRE(d01 != nullptr);
	CHECK(d01->getRequiredCapacity() == Approx(200.0));

	auto* d10 = s.getDemand(1, 0);
	REQUIRE(d10 != nullptr);
	CHECK(d10->getRequiredCapacity() == Approx(150.0));
}

TEST_CASE("Getters and Setters (Simulator)")
{

	Simulator s{"../examples/example_networks/5_node_example.json",
							"../examples/example_bitrates/basic_example.json", 5};

	// Allocator and Callback function must be set before init()
	CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
	CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));

	// Periods
	CHECK_NOTHROW(s.setNumberOfPeriods(5));
	CHECK_THROWS_AS(s.setNumberOfPeriods(0), std::invalid_argument);
	CHECK(s.getNumberOfPeriods() == 5);

	// Growth rates
	CHECK_NOTHROW(s.setBaseGrowthRate(0.55));
	CHECK(s.getBaseGrowthRate() == Approx(0.55));
	CHECK_THROWS_AS(s.setBaseGrowthRate(-0.1), std::invalid_argument);

	CHECK_NOTHROW(s.setGrowthRateStdDev(0.05));
	CHECK(s.getGrowthRateStdDev() == Approx(0.05));
	CHECK_THROWS_AS(s.setGrowthRateStdDev(-0.1), std::invalid_argument);

	CHECK_NOTHROW(s.setSeedGrowthRate(2025));

	CHECK_NOTHROW(s.setGrowthRates(std::vector<double>{0.1, 0.2, 0.3, 0.14, 0.18}));
	CHECK_NOTHROW(s.getGrowthRates());
	CHECK_THROWS_AS(s.setGrowthRates(std::vector<double>{0.1, 0.2, 0.3}), std::invalid_argument);

	CHECK_NOTHROW(s.getBitRates());
	CHECK_NOTHROW(s.getPaths());
	CHECK_NOTHROW(s.getController());
	CHECK_NOTHROW(s.getDemands());

	CHECK_NOTHROW(s.init());

	// After init()
	CHECK_THROWS_AS(USE_ALLOC_FUNCTION(FirstFit_100G, s), std::runtime_error);
	CHECK_THROWS_AS(USE_CALLBACK_FUNCTION(TestCallback, s), std::runtime_error);
	CHECK_THROWS_AS(s.setNumberOfPeriods(5), std::runtime_error);
	CHECK_THROWS_AS(s.setBaseGrowthRate(0.15), std::runtime_error);
	CHECK_THROWS_AS(s.setGrowthRateStdDev(0.05), std::runtime_error);
	CHECK_THROWS_AS(s.setSeedGrowthRate(2025), std::runtime_error);
	CHECK_THROWS_AS(s.setGrowthRates(std::vector<double>{0.1, 0.2, 0.3, 0.14, 0.18}), std::runtime_error);
}

TEST_CASE("Run simple Simulation")
{
	Simulator s{"../examples/example_networks/5_node_example.json",
							"../examples/example_bitrates/basic_example.json", 5};

	CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
	s.setNumberOfPeriods(15);
	s.setBaseGrowthRate(0.29);
	s.init();

	const std::filesystem::path outputFolder = std::filesystem::path("results") / "test_simulator";
	// Verbose output is written to "<outputFolder>/period_report.txt"
	const std::filesystem::path out = outputFolder / "period_report.txt";

	// Clean old file to avoid false positives
	std::error_code ec;
	std::filesystem::remove(out, ec); // ignore errors if it doesn't exist

	// Run the simulation (with verbose output)
	CHECK_NOTHROW(s.run(outputFolder.string()));

	// Check existence, regularity, and non-zero size of the output file
	REQUIRE(std::filesystem::exists(out));
	REQUIRE(std::filesystem::is_regular_file(out));
	REQUIRE(std::filesystem::file_size(out) > 0);
}

/**
 * Generic function to generate all possible combinations given counts for each position
 * Example: fiberCounts = [2, 3] returns: [[0,0], [0,1], [0,2], [1,0], [1,1], [1,2]]
 */
std::vector<std::vector<int>> generateAllCombinations(const std::vector<int>& counts) {
	std::vector<std::vector<int>> combinations;
	if (counts.empty()) return combinations;
	// Calculate total number of combinations
	int totalCombinations = 1;
	for (int count : counts) {
		totalCombinations *= count;
	}
	// Generate all combinations using counter arithmetic
	for (int i = 0; i < totalCombinations; i++) {
		std::vector<int> combination;
		int temp = i;
		// Convert counter to combination indices (like converting to mixed radix)
		for (int j = counts.size() - 1; j >= 0; j--) {
			combination.insert(combination.begin(), temp % counts[j]);
			temp /= counts[j];
		}
		combinations.push_back(combination);
	}
	return combinations;
}

/**
 * Simple function that takes a std::vector<int> and inverses it
 */
std::vector<int> inverseVector(const std::vector<int>& vec) {
	std::vector<int> inverted = vec;
	std::reverse(inverted.begin(), inverted.end());
	return inverted;
}


/**
 * Test Algorithm: First-Fit 100G (Multi-Fiber)
 *
 * This test installs only **100G** lightpaths with a “first-fit” strategy over
 * **multi-fiber links**. For each candidate route, we enumerate **all possible
 * fiber selections**—choosing exactly one fiber per hop—and try them in a
 * deterministic order. For each (route, fiber-combination) we then search bands
 * in the order **C → L → S → E**, cores, modes, and contiguous slot blocks
 * until we find the first feasible placement.
 *
 * Assumptions:
 * - **Symmetric Provisioning:** every A→B lightpath requires a mirrored B→A
 *   lightpath with identical parameters.
 * - **Bidirectional Network:** every A→B link has a corresponding B→A link with
 *   the same characteristics (capacity, bands, cores, modes, fibers, etc.).
 * - **Continuity/Contiguity:** the same band, core, mode, and a single contiguous
 *   slot block must be used on every hop and mirrored in the reverse direction.
 * - **Fixed Bitrate:** only 100G lightpaths are placed.
 * - **Adaptive Modulation:** for each candidate, we pick the best available
 *   modulation permitted by the lightpath characteristics.
 * - **Multi-Fiber per Link:** links may have multiple fibers. We select one fiber
 *   **per hop**; the chosen fiber indices are mirrored on the reverse direction.
 *
 * Procedure (per route):
 * 1) Build the set of **fiber combinations**: for a route of H hops with F_h
 *    fibers on hop h, enumerate the Cartesian product F_0 × F_1 × … × F_{H−1}.
 *    (Implementation detail: we iterate combinations in lexicographic order by
 *    hop index, then by fiber index.)
 * 2) For each fiber combination:
 *      a. For bands in order C→L→S→E,
 *      b. For each feasible (core, mode) supported on **every hop** of the route
 *         for the **selected fibers**,
 *      c. Find a **common contiguous slot block** free across all hops (and
 *         available in the reverse direction with the same parameters).
 *      d. If found, place the lightpath A→B and B→A and **stop** (first-fit).
 * 3) If no combination yields a feasible placement, the route is considered
 *    blocked; proceed to the next candidate route.
 *
 */
BEGIN_ALLOC_FUNCTION(FirstFit_100G_MF)
{
	// Fixed parameters for this algorithm
	// We always place 100G lightpaths
	auto bitRate = GET_BITRATE_BY_VALUE(100.0);
	auto bitRateIdx = GET_BITRATE_IDX_BY_VALUE(100.0);

	// Preferred band order
	std::vector<fns::Band> bandOrder = { // Preferred band order: C, L, S, E
			fns::Band::C, fns::Band::L, fns::Band::S, fns::Band::E};

	// Counter of placed lightpaths on this run
	int placedLightpaths = 0;

	FOR_EACH_DEMAND_SYMMETRIC { // For each demand (symmetric)

		if (CURRENT_DEMAND.isProvisioned()) continue; // Skip if already provisioned

		// Control variables
		std::vector<bool> totalSlots;
		int currentNumberSlots;
		int currentSlotIndex;

		// Store some variables for easier access
		auto SRC = CURRENT_DEMAND.getSrc();
		auto DST = CURRENT_DEMAND.getDst();

		// How many 100G lightpaths we will need to allocate
		int neededLPs = (int)std::ceil(CURRENT_DEMAND.getUnprovisionedCapacity() / 100.0f);

		for (int lp = 0; lp < neededLPs; lp++) { // For each 100G lightpath we need

			for (size_t r = 0; r < GET_NUM_ROUTES(SRC, DST); r++) { // For each Route

				// Generate all fiber combinations for the current route
				std::vector<int> fiberCounts;
				for (size_t l = 0; l < GET_NUM_LINKS(SRC, DST, r); l++) fiberCounts.push_back(GET_NUM_FIBERS(SRC, DST, r, l));
				std::vector<std::vector<int>> fiberCombinations = generateAllCombinations(fiberCounts);

				// Try each fiber combination
				for (const auto& selectedFibersIdx : fiberCombinations) { // For each fiber combination

					for (auto &band : bandOrder) { // For each Band in preferred order

						// Check if the band exists across all fibers of this combination
						bool bandExists = true;
						for (size_t l = 0; l < GET_NUM_LINKS(SRC, DST, r); l++) {
							auto bands = GET_BANDS(SRC, DST, r, l, selectedFibersIdx[l]);
							if (std::find(bands.begin(), bands.end(), band) == bands.end()) {
								bandExists = false;
								break;
							}
						}
						if (!bandExists) continue;

						// Get best the 100G modulation format for this route and band
						int bestModulationIdx = GET_BEST_ADAPTIVE_MF(SRC, DST, bitRateIdx, r, band);
						if (bestModulationIdx == -1) continue; // No modulation found for this band

						size_t minCores = GET_MIN_NUM_CORES(SRC, DST, r, selectedFibersIdx);
						for (size_t c = 0; c < minCores; c++) { // For each Core (assuming continuity)

							size_t minModes = GET_MIN_NUM_MODES(SRC, DST, r, selectedFibersIdx, c, band);
							for (size_t m = 0; m < minModes; m++) { // For each Mode (assuming continuity)

								size_t minSlots = GET_MIN_NUM_SLOTS(SRC, DST, r, selectedFibersIdx, c, band, m);
								totalSlots = std::vector<bool>(minSlots, false); // Remember, minimum available across all links

								for (size_t l = 0; l < GET_NUM_LINKS(SRC, DST, r); l++) {
									// Now we OR the slots of this link with the totalSlots vector
									for (size_t s = 0; s < minSlots; s++) {
										totalSlots[s] = totalSlots[s] | (GET_SLOT(SRC, DST, r, l, selectedFibersIdx[l], c, band, m, s) != -1);
									} // Next slot
								} // Next link

								// Get required slots for 100G with this modulation
								int requiredSlots = GET_REQ_SLOTS(bitRateIdx, bestModulationIdx, band);

								// At this point we have a representative vector of all slots in the route
								// We can now search for a contiguous block of free slots
								currentNumberSlots = 0;
								currentSlotIndex = 0;

								for (size_t s = 0; s < totalSlots.size(); s++) {
									if (totalSlots[s] == false) {
										currentNumberSlots++;
									}
									else {
										currentNumberSlots = 0;
										currentSlotIndex = s + 1;
									}
									if (currentNumberSlots == requiredSlots) {

										// Now that we found a contiguous block of free slots:
										// we create a connection object (two if working with symmetric demands)
										auto newConnection = NEW_CONNECTION(bitRate, SRC, DST);
										auto newConnectionMirror = NEW_CONNECTION(bitRate, DST, SRC);

										// Populate the connection with the route data
										const size_t numLinks = GET_NUM_LINKS(SRC, DST, r);
										for (size_t l = 0; l < numLinks; l++) {
											auto linkFwd = GET_LINK_AT(SRC, DST, r, l);
											ADD_LINK_TO_CONNECTION(/*connection object=*/newConnection,
															/*linkId=*/linkFwd,
															/*fiber=*/selectedFibersIdx[l],
															/*core=*/c, /*band=*/band, /*mode=*/m,
															/*startSlot=*/currentSlotIndex,
															/*len=*/requiredSlots);
										}

										for (size_t mirrorIdx = 0; mirrorIdx < numLinks; ++mirrorIdx) {
											const size_t fwdIdx = numLinks - 1 - mirrorIdx;
											auto linkFwd = GET_LINK_AT(SRC, DST, r, fwdIdx);
											auto linkRev = this->network->getLink(linkFwd->getDst(), linkFwd->getSrc());
											REQUIRE(linkRev);
											ADD_LINK_TO_CONNECTION(/*connection object=*/newConnectionMirror,
															/*linkId=*/linkRev,
															/*fiber=*/selectedFibersIdx[fwdIdx],
															/*core=*/c, /*band=*/band, /*mode=*/m,
															/*startSlot=*/currentSlotIndex,
															/*len=*/requiredSlots);
										}
										// Allocate the connections
										ALLOCATE_CONNECTION(newConnection);
										ALLOCATE_CONNECTION(newConnectionMirror);

										// Go to the next lightpath
										goto next_lightpath;
									} // Placement successful
								} // Next slot
							} // Next mode
						} // Next core
					} // Next band
				}
			} // Next route
			next_lightpath: placedLightpaths++;
		} // Next lightpath
	} // Next demand
}
END_ALLOC_FUNCTION

TEST_CASE("Run Simulation with Multi-Fiber") {
	Simulator s{"../examples/example_networks/5_node_example.json",
							"../examples/example_bitrates/basic_example.json", 5};

	CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G_MF, s));
	s.setNumberOfPeriods(15);
	s.setBaseGrowthRate(0.29);
	s.init();

	// Run the simulation
	CHECK_NOTHROW(s.run(""));
}

BEGIN_CALLBACK_FUNCTION(AddFibersCallback)
{
	FOR_EACH_LINK {
		if (CURRENT_LINK.getUsagePercentage() > 30.0f/*%*/) {
			// add a new fiber of type SSMF with 400 and 520 slots, C and L bands
			auto bandSlotMatrix = std::map<fns::Band, std::vector<std::vector<int>>>{
					{fns::Band::C, {{400}}}, {fns::Band::L, {{520}}}}; // 1 core, 1 mode
			auto newFiber = NEW_FIBER(bandSlotMatrix);
			CURRENT_LINK.addFiber(newFiber);
		}
	}
}
END_CALLBACK_FUNCTION

TEST_CASE("Run Simulation with Fiber Addition")
{
	Simulator s{"../examples/example_networks/5_node_example.json",
							"../examples/example_bitrates/basic_example.json", 5};

	CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G_MF, s));
	CHECK_NOTHROW(USE_CALLBACK_FUNCTION(AddFibersCallback, s));
	s.setNumberOfPeriods(15);
	s.setBaseGrowthRate(0.29);
	s.init();

	// Run the simulation
	s.run("");
}

TEST_CASE("Build & init with all JSON files (simple)")
{

  // DT-17
  {
    Simulator s{"../examples/example_networks/DT-17.json",
                "../examples/example_networks/DT-17_routes.json",
                "../examples/example_bitrates/basic_example.json"};
    CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
    CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));
    CHECK_NOTHROW(s.init());
		CHECK_NOTHROW(s.run(""));
  }

	// DT-50
  {
	Simulator s{"../examples/example_networks/DT-50.json",
				"../examples/example_networks/DT-50_routes.json",
				"../examples/example_bitrates/basic_example.json"};
	CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
	CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));
	CHECK_NOTHROW(s.init());
		CHECK_NOTHROW(s.run(""));
  }

  // EURO-16
  {
    Simulator s{"../examples/example_networks/EURO-16.json",
                "../examples/example_networks/EURO-16_routes.json",
                "../examples/example_bitrates/basic_example.json"};
    CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
    CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));
    CHECK_NOTHROW(s.init());
		CHECK_NOTHROW(s.run(""));
  }

  // MCF example
  {
    Simulator s{"../examples/example_networks/NSFNet_MCF.json",
                "../examples/example_networks/NSFNet_routes.json",
                "../examples/example_bitrates/basic_example.json"};
    CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
    CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));
    CHECK_NOTHROW(s.init());
		CHECK_NOTHROW(s.run(""));
  }

  // NSFNet_MB (uses NSFNet_routes.json as in your list)
  {
    Simulator s{"../examples/example_networks/NSFNet_MB.json",
                "../examples/example_networks/NSFNet_routes.json",
                "../examples/example_bitrates/basic_example.json"};
    CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
    CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));
    CHECK_NOTHROW(s.init());
		CHECK_NOTHROW(s.run(""));
  }

  // NSFNet
  {
    Simulator s{"../examples/example_networks/NSFNet.json",
                "../examples/example_networks/NSFNet_routes.json",
                "../examples/example_bitrates/basic_example.json"};
    CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
    CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));
    CHECK_NOTHROW(s.init());
		CHECK_NOTHROW(s.run(""));
  }

  // UKNet
  {
    Simulator s{"../examples/example_networks/UKNet.json",
                "../examples/example_networks/UKNet_routes.json",
                "../examples/example_bitrates/basic_example.json"};
    CHECK_NOTHROW(USE_ALLOC_FUNCTION(FirstFit_100G, s));
    CHECK_NOTHROW(USE_CALLBACK_FUNCTION(TestCallback, s));
    CHECK_NOTHROW(s.init());
		CHECK_NOTHROW(s.run(""));
  }
}

/**
 * Test that verifies slots are actually marked after ALLOCATE_CONNECTION
 * and that utilization calculation correctly counts used slots.
 * 
 * This test specifically checks:
 * 1. Slots are initialized to -1 (free)
 * 2. After allocation, slots are set to connection ID (positive int)
 * 3. Utilization calculation correctly counts slots != -1 as used
 */
TEST_CASE("Slot marking and utilization calculation") {
  Simulator s{"../examples/example_networks/5_node_example.json",
              "../examples/example_bitrates/basic_example.json", 5};

  USE_ALLOC_FUNCTION(FirstFit_100G, s);
  s.setNumberOfPeriods(1);
  s.init();

  auto network = s.getController()->getNetwork();

  // SECTION 1: Verify slots are initialized to -1 before allocation
  SECTION("Slots initialized to -1 before allocation") {
    auto link = network->getLink(0);
    REQUIRE(link->getFibers().size() > 0);
    auto fiber = link->getFiber(0);
    
    for (auto band : fiber->getBands()) {
      for (int c = 0; c < fiber->getNumberOfCores(); c++) {
        for (int m = 0; m < fiber->getNumberOfModes(c, band); m++) {
          for (int slot = 0; slot < std::min(10, fiber->getNumberOfSlots(c, band, m)); slot++) {
            CHECK(fiber->getSlot(c, band, m, slot) == -1);
          }
        }
      }
    }
  }

  // SECTION 2: Run allocation and verify slots are marked
  SECTION("Slots marked after allocation") {
		s.run("");

    // Count connections
    auto& connections = s.getController()->getConnections();
    REQUIRE(connections.size() > 0);

    // Verify at least some slots are marked (not -1)
    int usedSlots = 0;
    int totalSlots = 0;
    for (auto link : network->getLinks()) {
      for (auto fiber : link->getFibers()) {
        for (auto band : fiber->getBands()) {
          for (int c = 0; c < fiber->getNumberOfCores(); c++) {
            for (int m = 0; m < fiber->getNumberOfModes(c, band); m++) {
              for (int slot = 0; slot < fiber->getNumberOfSlots(c, band, m); slot++) {
                totalSlots++;
                if (fiber->getSlot(c, band, m, slot) != -1) {
                  usedSlots++;
                }
              }
            }
          }
        }
      }
    }

    INFO("Total slots: " << totalSlots);
    INFO("Used slots: " << usedSlots);
    INFO("Connections: " << connections.size());
    
    // With connections allocated, we MUST have some used slots
    CHECK(usedSlots > 0);
    
    // Utilization should be > 0 if we have connections
    double utilization = (totalSlots > 0) ? (100.0 * usedSlots / totalSlots) : 0.0;
    CHECK(utilization > 0.0);
  }
}

/**
 * Test that verifies the Network::useSlots() method actually marks slots
 */
TEST_CASE("Network::useSlots marks slots with connection ID") {
  Network net("../examples/example_networks/5_node_example.json");
  
  // Get first link and fiber
  auto link = net.getLink(0);
  REQUIRE(link->getFibers().size() > 0);
  auto fiber = link->getFiber(0);
  
  // Find first available band
  auto bands = fiber->getBands();
  REQUIRE(bands.size() > 0);
  auto band = bands[0];
  
  int connectionId = 42;
  int slotFrom = 0;
  int slotTo = 5;
  
  // Verify slots are -1 before
  for (int s = slotFrom; s < slotTo; s++) {
    CHECK(fiber->getSlot(0, band, 0, s) == -1);
  }
  
  // Use slots
  net.useSlots(link->getId(), 0, 0, band, 0, slotFrom, slotTo, connectionId);
  
  // Verify slots are now marked with connection ID
  for (int s = slotFrom; s < slotTo; s++) {
    CHECK(fiber->getSlot(0, band, 0, s) == connectionId);
  }
  
  // Verify isSlotUsed returns true (1)
  for (int s = slotFrom; s < slotTo; s++) {
    CHECK(net.isSlotUsed(link->getId(), 0, 0, band, 0, s) == 1);
  }
  
  // Verify slots outside range are still -1
  CHECK(fiber->getSlot(0, band, 0, slotTo) == -1);
  CHECK(net.isSlotUsed(link->getId(), 0, 0, band, 0, slotTo) == 0);
}

/**
 * Test that verifies Allocator::alloc marks slots via Network::useSlots
 */
TEST_CASE("Allocator::alloc marks slots via Network::useSlots") {
  Network net("../examples/example_networks/5_node_example.json");
  
  // Create allocator with the network
  Allocator alloc(std::make_shared<Network>(net));
  
  // Create bitrate 
  auto bitRate = std::make_shared<BitRate>(100.0);
  bitRate->addModulation("BPSK", {{fns::Band::C, 8}}, {{fns::Band::C, 5520}});
  int reqSlots = bitRate->getRequiredSlots(0, fns::Band::C);
  
  // Get first link
  auto link = alloc.getNetwork()->getLink(0);
  int linkId = link->getId();
  auto fiber = link->getFiber(0);
  auto bands = fiber->getBands();
  REQUIRE(bands.size() > 0);
  auto band = bands[0];
  
  // Create connection (ID will be assigned by allocator during alloc)
  Connection conn(bitRate, 0, 1);
  // Note: conn.getId() returns -1 here, but allocator generates its own ID
  
  // Add link info to connection
  conn.addLink(linkId, 0, 0, band, 0, 0, reqSlots);
  
  // Verify slots before allocation
  for (int s = 0; s < reqSlots; s++) {
    CHECK(fiber->getSlot(0, band, 0, s) == -1);
  }
  
  // Allocate via allocator
  alloc.alloc(conn);
  
  // Verify slots after allocation - should be marked with a positive ID (not -1)
  // The allocator generates its own temporary IDs for marking slots
  for (int s = 0; s < reqSlots; s++) {
    int slotValue = fiber->getSlot(0, band, 0, s);
    CHECK(slotValue > 0);  // Must be positive (marked as used)
    CHECK(slotValue != -1);  // Must not be -1 (free)
  }
}
