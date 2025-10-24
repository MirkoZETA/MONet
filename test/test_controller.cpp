#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../src/sim/controller.hpp"
#include "../src/core/network.hpp"
#include "../src/alloc/dummy_allocator.hpp"
#include "../src/util/utils.hpp"
#include "../src/util/json.hpp"

TEST_CASE("Controller Constructor and Basic Getters/Setters")
{
	Controller controller;

	SECTION("Default Constructor")
	{

		CHECK(controller.getNetwork() == nullptr);
		CHECK_NOTHROW(controller.getAllocator()); // Should have a default allocator
		CHECK(controller.getConnections().empty());
		CHECK(controller.getP2Ps().empty());
	}

	auto network = std::make_shared<Network>("../examples/example_networks/5_node_example.json");
	controller.setNetwork(network);

	SECTION("Constructor with Network")
	{
		CHECK(controller.getNetwork() == network);
		REQUIRE(controller.getNetwork()->getNumberOfNodes() == 5);
	}

	SECTION("Set and Get Network")
	{
		controller.setNetwork(network);
		CHECK(controller.getNetwork() == network);
		REQUIRE(controller.getNetwork()->getNumberOfNodes() == 5);
	}

	SECTION("Set and Get Allocator")
	{
		auto dummyAllocator = std::make_unique<DummyAllocator>();
		CHECK_NOTHROW(controller.setAllocator(std::move(dummyAllocator)));
		CHECK_NOTHROW(controller.getAllocator());
	}

	SECTION("Set Call Back Function")
	{
		CHECK_NOTHROW(controller.setCallbackFunction(nullptr));
	}

	SECTION("Get Connection/P2P - Error cases")
	{
		CHECK_THROWS_AS(controller.getConnection(0), std::out_of_range);
		CHECK_THROWS_AS(controller.getConnection(-1), std::out_of_range);
		CHECK_THROWS_AS(controller.getP2P(0), std::out_of_range);

		auto conn = std::make_unique<Connection>(0, 1.0, std::make_shared<const BitRate>(10.0), false, 0, 1);
		controller.addConnection(std::move(conn));
		CHECK_NOTHROW(controller.getConnection(0));

		std::vector<int> fiberIdxs = {0};

		CHECK_THROWS_AS(controller.addP2P(0, 1, 0, fiberIdxs), std::runtime_error);
		controller.setPaths(5);
		CHECK_NOTHROW(controller.addP2P(0, 1, 0, fiberIdxs));

		CHECK_NOTHROW(controller.getP2P(0));
		CHECK_NOTHROW(controller.getP2Ps());
	}
}

TEST_CASE("Controller P2P Management")
{
	Controller controller;

	SECTION("No network set")
	{
		std::vector<int> fiberIdxs = {0, 0};
		CHECK_THROWS_AS(controller.addP2P(0, 2, 0, fiberIdxs), std::runtime_error);
	}

	controller.setNetwork(std::make_shared<Network>("../examples/example_networks/5_node_example.json"));
	controller.setPaths(5);

	SECTION("Add P2P")
	{
		std::vector<int> fiberIdxs = {0, 0, 0};

		// Invalid node IDs
		CHECK_THROWS_AS(controller.addP2P(10, 12, 0, fiberIdxs), std::invalid_argument);

		// Should not exist more than 5 paths between nodes 0 and 1
		CHECK_THROWS_AS(controller.addP2P(0, 1, 6, fiberIdxs), std::out_of_range);

		// Size of fiberIdxs must match number of links in path
		CHECK_THROWS_AS(controller.addP2P(0, 1, 0, fiberIdxs), std::invalid_argument);

		// Valid case
		CHECK_NOTHROW(controller.addP2P(0, 1, 1, fiberIdxs));
		CHECK(controller.getP2Ps().size() == 1);

		// Add P2P with new fibers
		std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix = {
				{fns::Band::C, {{400}}}};
		CHECK_NOTHROW(controller.addP2P(0, 1, 0, bandSlotMatrix));
		CHECK(controller.getP2Ps().size() == 2);
	}

	SECTION("Migrate connection to P2P")
	{
		auto bitrate = std::make_shared<BitRate>(10.0);
		auto conn = std::make_unique<Connection>(0, 1.0, bitrate, false, 0, 1);

		// Store the ID before moving the connection
		int connectionId = conn->getId();

		controller.addConnection(std::move(conn));

		CHECK(controller.getConnections().size() == 1);

		std::vector<int> fiberIdxs = {0};
		CHECK_NOTHROW(controller.addP2P(0, 1, 0, fiberIdxs));
		CHECK(controller.getP2Ps().size() == 1);

		auto &p2p = controller.getP2P(0);
		CHECK_NOTHROW(controller.migrateConnectionToP2P(0, 0, fns::Band::C, 0, 0, 10, connectionId, bitrate));
		auto &connection = controller.getConnection(0);
		CHECK(connection.isAllocatedInP2P() == true);
	}
}

TEST_CASE("Paths")
{
	Controller controller;

	// Network dont set
	CHECK_THROWS_AS(controller.setPaths(5), std::runtime_error);

	controller.setNetwork(std::make_shared<Network>("../examples/example_networks/5_node_example.json"));
	CHECK_NOTHROW(controller.setPaths(4));

	// Now check that the paths calculated are same as expected
	// TODO.
}

TEST_CASE("Check Allocator")
{

	Controller controller;
	controller.setNetwork(std::make_shared<Network>("../examples/example_networks/5_node_example.json"));

	CHECK_NOTHROW(Allocator());

	std::vector<std::vector<Demand>> demands(4, std::vector<Demand>(4));
	auto bitrate = std::make_shared<BitRate>(10.0);
	std::vector<std::shared_ptr<BitRate>> bitRates = {bitrate};
	CHECK_THROWS_AS(controller.assignConnections(demands, bitRates, 0.0), std::runtime_error);
}

TEST_CASE("Modify Network and clearPaths")
{
	Controller controller;
	auto network = std::make_shared<Network>("../examples/example_networks/5_node_example.json");
	controller.setNetwork(network);

	// Minimal band/slot matrix for new fibers
	std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix = {
			{fns::Band::C, {{400}}}};

	SECTION("clearPaths empties cached paths (subsequent recompute repopulates)")
	{
		controller.setPaths(2);
		REQUIRE_FALSE(controller.getPaths()->empty());

		controller.clearPaths();
		CHECK(controller.getPaths()->empty());

		// Rebuild after clear
		CHECK_NOTHROW(controller.setPaths(2));
		CHECK_FALSE(controller.getPaths()->empty());
	}

	SECTION("addLink with bidirectional=true and recomputeNow=true updates paths immediately")
	{
		// Set an initial k so addLink can recompute paths
		controller.setPaths(3);
		// Sanity: paths exist
		REQUIRE_FALSE(controller.getPaths()->empty());

		// Add a new direct connection between two nodes that are not connected and recompute immediately
		CHECK_NOTHROW(controller.addLink(/*src=*/3, /*dst=*/0, /*length=*/7.5f,
																		 bandSlotMatrix));

		controller.clearPaths();
		controller.setPaths(3);

		// After recompute, paths should still be non-empty (graph valid)
		CHECK_FALSE(controller.getPaths()->empty());

		// There should be direct route between 0->3 now
		const auto &paths = *controller.getPaths();
		CHECK(paths.size() == controller.getNetwork()->getNumberOfNodes());
		CHECK(paths.at(3).at(0).at(0).size() == 1);
		// And reverse also present because this is intended for bidirectional networks
		CHECK(paths.at(0).at(3).at(0).size() == 1);

		// Check ID of the new link, should be 16
		CHECK(paths.at(3).at(0).at(0).at(0)->getId() == 16);
		// And reverse link ID should be 17
		CHECK(paths.at(0).at(3).at(0).at(0)->getId() == 17);
	}

	SECTION("Batch adding links with recomputeNow=false then recompute once")
	{

		// TODO.

		// Start fresh
		controller.clearPaths();
		CHECK(controller.getPaths()->empty());

		// Batch add: do NOT recompute between adds
		CHECK_NOTHROW(controller.addLink(1, 2, 5.0f, bandSlotMatrix));
		CHECK_NOTHROW(controller.addLink(2, 3, 5.0f, bandSlotMatrix));

		// Still empty because we didn't recompute yet
		CHECK(controller.getPaths()->empty());

		// Recompute once at the end
		CHECK_NOTHROW(controller.setPaths(3));

		CHECK_FALSE(controller.getPaths()->empty());
		// The batched links should allow some path(s) between 1 and 3
		const auto &paths = *controller.getPaths();
		CHECK(paths[1][3].size() >= 1);
		CHECK(paths[3][1].size() >= 1);
	}

	SECTION("addNodes and connect them to the network")
	{

		controller.setPaths(3);

		CHECK_NOTHROW(controller.addNode(5, std::nullopt, std::nullopt, std::nullopt, "Node_F"));
		CHECK_NOTHROW(controller.addNode(6, std::nullopt, std::nullopt, std::nullopt, "Node_G"));
		CHECK_NOTHROW(controller.addLink(5, 6, 500.0f, bandSlotMatrix));

		// Reset
		controller.clearPaths();
		controller.setPaths(3);

		// Check that are connected between 5 and 6
		CHECK(controller.getPaths()->at(5).at(6).at(0).size() == 1);
		CHECK(controller.getPaths()->at(6).at(5).at(0).size() == 1);
		// But not to the rest of the network
		// (check a few random pairs)
		CHECK(controller.getPaths()->at(0).at(5).empty());
		CHECK(controller.getPaths()->at(1).at(5).empty());
		CHECK(controller.getPaths()->at(0).at(6).empty());
		CHECK(controller.getPaths()->at(1).at(6).empty());

		// Now connect nodes to the network via node 5-0
		CHECK_NOTHROW(controller.addLink(5, 0, 5.0f, bandSlotMatrix));

		// Reset
		controller.clearPaths();
		controller.setPaths(3);

		// Now 5 and 6 should be connected to the rest of the network
		// (check a few random pairs)
		CHECK_FALSE(controller.getPaths()->at(0).at(5).empty());
		CHECK_FALSE(controller.getPaths()->at(1).at(5).empty());
		CHECK_FALSE(controller.getPaths()->at(0).at(6).empty());
		CHECK_FALSE(controller.getPaths()->at(1).at(6).empty());
	}
}
