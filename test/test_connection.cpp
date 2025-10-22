#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../src/core/connection.hpp"

TEST_CASE("Constructor (Connection)") {

  CHECK_NOTHROW(Connection());

  auto bitrate = std::make_shared<BitRate>(10.0);
  CHECK_NOTHROW(Connection(0, 0, bitrate, false, 0, 1));

	CHECK_THROWS_AS(Connection(-1, 0, bitrate, false, 0, 1), std::invalid_argument);
	CHECK_THROWS_AS(Connection(0, -1, bitrate, false, 0, 1), std::invalid_argument);
	CHECK_THROWS_AS(Connection(0, 0, nullptr, false, 0, 1), std::invalid_argument);
	CHECK_THROWS_AS(Connection(0, 0, bitrate, false, -1, 1), std::invalid_argument);
	CHECK_THROWS_AS(Connection(0, 0, bitrate, false, 0, -1), std::invalid_argument);
}

TEST_CASE("Add Link") {
	Connection conn;

	auto bitrate = std::make_shared<BitRate>(10.0);
	conn = Connection(0, 0, bitrate, false, 0, 1);

	SECTION("Add link with slot range") {
		CHECK_NOTHROW(conn.addLink(0, 0, 0, fns::Band::C, 0, 10, 20));
		CHECK(conn.getLinks().size() == 1);
		CHECK(conn.getFibers().size() == 1);
		CHECK(conn.getCores().size() == 1);
		CHECK(conn.getBands().size() == 1);
		CHECK(conn.getModes().size() == 1);
		CHECK(conn.getSlots().size() == 1);
		CHECK(conn.getSlots()[0].size() == 10); // Slots from 10 to 20 exclusive
	}

	SECTION("Add link with specific slots") {
		std::vector<int> slots = {5, 15, 25};
		CHECK_NOTHROW(conn.addLink(1, 1, 1, fns::Band::L, 1, slots));
		CHECK(conn.getLinks().size() == 1);
		CHECK(conn.getFibers().size() == 1);
		CHECK(conn.getCores().size() == 1);
		CHECK(conn.getBands().size() == 1);
		CHECK(conn.getModes().size() == 1);
		CHECK(conn.getSlots().size() == 1);
		CHECK(conn.getSlots()[0].size() == slots.size());
		for (size_t i = 0; i < slots.size(); ++i) {
			CHECK(conn.getSlots()[0][i] == slots[i]);
		}
	}

	SECTION("Add Link with Link Object") {
		auto link = std::make_shared<Link>(0, 100.0f);
		CHECK_NOTHROW(conn.addLink(link, 0, 0, fns::Band::C, 0, 10, 20));
		CHECK(conn.getLinks().size() == 1);
		CHECK(conn.getFibers().size() == 1);
		CHECK(conn.getCores().size() == 1);
		CHECK(conn.getBands().size() == 1);
		CHECK(conn.getModes().size() == 1);
		CHECK(conn.getSlots().size() == 1);
		CHECK(conn.getSlots()[0].size() == 10); // Slots from 10 to 20 exclusive
	}

	SECTION("Invalid Link Parameters") {
		CHECK_THROWS_AS(conn.addLink(-1, 0, 0, fns::Band::C, 0, 10, 20), std::invalid_argument);
		CHECK_THROWS_AS(conn.addLink(0, -1, 0, fns::Band::C, 0, 10, 20), std::invalid_argument);
		CHECK_THROWS_AS(conn.addLink(0, 0, -1, fns::Band::C, 0, 10, 20), std::invalid_argument);
		CHECK_THROWS_AS(conn.addLink(0, 0, 0, fns::Band::C, -1, 10, 20), std::invalid_argument);
		CHECK_THROWS_AS(conn.addLink(0, 0, 0, fns::Band::C, 0, -10, 20), std::invalid_argument);
		CHECK_THROWS_AS(conn.addLink(0, 0, 0, fns::Band::C, 0, 10, -20), std::invalid_argument);
		CHECK_THROWS_AS(conn.addLink(0, 0, 0, fns::Band::C, 0, 20, 10), std::invalid_argument);
		CHECK_THROWS_AS(conn.addLink(nullptr, 0, 0, fns::Band::C, 0, 10, 20), std::invalid_argument);
	}
}

TEST_CASE("Set and Get Connection Attributes") {

	Connection conn = Connection();

	CHECK(conn.getId() == -1);
	CHECK(conn.getTimeConnection() == Approx(0.0).epsilon(0.01));
	CHECK(conn.getBitrate() == nullptr);
	CHECK(conn.isAllocatedInP2P() == false);
	CHECK(conn.getSrc() == -1);
	CHECK(conn.getDst() == -1);

	CHECK_THROWS_AS(conn.setTime(-5.0), std::invalid_argument);
	CHECK_NOTHROW(conn.setTime(5.0));
	CHECK(conn.getTimeConnection() == Approx(5.0).epsilon(0.01));

	CHECK_THROWS_AS(conn.setId(-1), std::invalid_argument);
	CHECK_NOTHROW(conn.setId(0));
	CHECK_THROWS_AS(conn.setId(2), std::runtime_error); // ID already set
	CHECK(conn.getId() == 0);

	CHECK_NOTHROW(conn.setAllocatedInP2P(true));
	CHECK(conn.isAllocatedInP2P() == true);
	
	auto bitrate = std::make_shared<BitRate>(10.0);
	CHECK_THROWS_AS(conn.changeBitRate(nullptr), std::invalid_argument);
	CHECK_NOTHROW(conn.changeBitRate(bitrate));
	CHECK(conn.getBitrate() == bitrate);

	int link_id = 123;
	int fiber_selected = 0;
	int core_selected = 0;
	int mode_selected = 0;
	std::vector<int> slots_link_1 = {1, 2, 3};
	std::vector<int> slots_link_2 = {1, 2, 3};

	CHECK_NOTHROW(conn.addLink(link_id, fiber_selected, core_selected, fns::Band::C, mode_selected, slots_link_1));
	CHECK_THROWS_AS(conn.addLink(link_id, fiber_selected, core_selected, fns::Band::C, mode_selected, slots_link_2), std::invalid_argument); // Link ID already exists
	CHECK_NOTHROW(conn.addLink(link_id + 1, fiber_selected, core_selected, fns::Band::C, mode_selected, slots_link_2));

	CHECK(conn.getLinks().size() == 2);
	CHECK(conn.getFibers().size() == 2);
	CHECK(conn.getCores().size() == 2);
	CHECK(conn.getBands().size() == 2);
	CHECK(conn.getModes().size() == 2);
	CHECK(conn.getSlots().size() == 2);
}