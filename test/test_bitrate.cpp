// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/core/bitrate.hpp"
#include "../src/core/link.hpp"
#include "catch.hpp"

TEST_CASE("Constructor (Bitrate)")
{
	CHECK_NOTHROW(BitRate(10.0));
	CHECK_THROWS_AS(BitRate(0.0), std::invalid_argument);
}

TEST_CASE("Add and Get Modulation")
{
	BitRate br(10.0);
	br.addModulation("QPSK", {{fns::Band::C, 20}}, {{fns::Band::C, 200}});
	br.addModulation("16QAM", {{fns::Band::C, 15}, {fns::Band::L, 10}}, {{fns::Band::C, 150}, {fns::Band::L, 100}});

	// Access by position
	CHECK_NOTHROW(br.getModulationFormat(0));
	CHECK_NOTHROW(br.getModulationFormat(1));
	CHECK_THROWS_AS(br.getModulationStr(2), std::out_of_range);

	// Access by name
	CHECK_NOTHROW(br.getModulationFormat("QPSK"));
	CHECK_NOTHROW(br.getModulationFormat("16QAM"));
	CHECK_THROWS_AS(br.getModulationFormat("8QAM"), std::invalid_argument);

	CHECK(br.getModulationStr(0) == "QPSK");
	CHECK(br.getModulationStr(1) == "16QAM");
	CHECK_THROWS_AS(br.getModulationStr(2), std::out_of_range);

	// Create a route for distance adaptive test
	std::shared_ptr<Link> link1 = std::make_shared<Link>(0, 100.0);  // id=0, length=100.0
	std::shared_ptr<Link> link2 = std::make_shared<Link>(1, 30.0);   // id=1, length=30.0
	Route route1 = {link1, link2};

	CHECK(br.getAdaptiveModulation(route1) == 1);
	CHECK(br.getAdaptiveModulation(route1, fns::Band::C) == 1);
	CHECK(br.getAdaptiveModulation(route1, fns::Band::L) == -1);
	CHECK(br.getAdaptiveModulation(route1, fns::Band::S) == -1);

	std::shared_ptr<Link> link3 = std::make_shared<Link>(2, 50.0);   // id=2, length=50.0
	Route route2 = {link2, link3};
	CHECK(br.getAdaptiveModulation(route2) == 1);
	CHECK(br.getAdaptiveModulation(route2, fns::Band::L) == 1);
}

TEST_CASE("Get Bitrate Attributes")
{
	BitRate br1(10.0);
	br1.addModulation("QPSK", {{fns::Band::C, 20}}, {{fns::Band::C, 200}});
	br1.addModulation("16QAM", {{fns::Band::C, 15}, {fns::Band::L, 10}}, {{fns::Band::C, 150}, {fns::Band::L, 100}});
	BitRate br2 = BitRate(40.0);
	br2.addModulation("QPSK", {{fns::Band::C, 40}}, {{fns::Band::C, 200}});
	br2.addModulation("16QAM", {{fns::Band::C, 30}, {fns::Band::L, 20}}, {{fns::Band::C, 150}, {fns::Band::L, 100}});

	CHECK(br2.getBitRate() == Approx(40.0).epsilon(0.01));
	CHECK(br2.getBitRateStr() == "40.000000");
	CHECK(br2.getNumberOfModulations() == 2);
	CHECK(br1.getBitRate() == Approx(10.0).epsilon(0.01));
	CHECK(br1.getBitRateStr() == "10.000000");
	CHECK(br1.getNumberOfModulations() == 2);

	CHECK(br1.getRequiredSlots(0) == 20);
	CHECK_THROWS_AS(br1.getRequiredSlots(2), std::out_of_range);
	CHECK(br1.getRequiredSlots(1, fns::Band::C) == 15);
	CHECK_THROWS_AS(br1.getRequiredSlots(2, fns::Band::C), std::out_of_range);

	CHECK(br1.getReach(0) == Approx(200).epsilon(0.01));
	CHECK_THROWS_AS(br1.getReach(2), std::out_of_range);
	CHECK(br1.getReach(1, fns::Band::L) == Approx(100).epsilon(0.01));
	CHECK_THROWS_AS(br1.getReach(2, fns::Band::L), std::out_of_range);
}

TEST_CASE("Read data from files")
{
    std::vector<std::shared_ptr<BitRate>> bitrates =
            BitRate::readBitRatesFile(std::string("../examples/example_bitrates/basic_example.json"));

    // Bitrate 10.0
    CHECK(bitrates[0]->getBitRate() == Approx(10.0).epsilon(0.01));
    CHECK(std::stod(bitrates[0]->getBitRateStr()) == Approx(10.0).epsilon(0.01));

    CHECK(bitrates[0]->getModulationStr(0) == "BPSK");
    CHECK(bitrates[0]->getReach(0) == Approx(5520.0).epsilon(0.01));
    CHECK(bitrates[0]->getRequiredSlots(0) == 1);

    CHECK_THROWS(bitrates[0]->getModulationStr(1));

    // Bitrate 40.0
    CHECK(bitrates[1]->getBitRate() == Approx(40.0).epsilon(0.01));
    CHECK(std::stod(bitrates[1]->getBitRateStr()) == Approx(40.0).epsilon(0.01));

    CHECK(bitrates[1]->getModulationStr(0) == "BPSK");
    CHECK(bitrates[1]->getReach(0) == Approx(5520.0).epsilon(0.01));
    CHECK(bitrates[1]->getRequiredSlots(0) == 4);

    CHECK(bitrates[1]->getModulationStr(1) == "QPSK");
    CHECK(bitrates[1]->getReach(1) == Approx(5520.0).epsilon(0.01));
    CHECK(bitrates[1]->getRequiredSlots(1) == 2);

    CHECK_THROWS(bitrates[1]->getModulationStr(2));

    // Bitrate 100.0
    CHECK(bitrates[2]->getBitRate() == Approx(100.0).epsilon(0.01));
    CHECK(std::stod(bitrates[2]->getBitRateStr()) == Approx(100.0).epsilon(0.01));

    CHECK(bitrates[2]->getModulationStr(0) == "BPSK");
    CHECK(bitrates[2]->getReach(0) == Approx(5520.0).epsilon(0.01));
    CHECK(bitrates[2]->getRequiredSlots(0) == 8);

    CHECK_THROWS(bitrates[2]->getModulationStr(1));

    // Bitrate 400.0
    CHECK(bitrates[3]->getBitRate() == Approx(400.0).epsilon(0.01));
    CHECK(std::stod(bitrates[3]->getBitRateStr()) == Approx(400.0).epsilon(0.01));

    CHECK(bitrates[3]->getModulationStr(0) == "BPSK");
    CHECK(bitrates[3]->getReach(0) == Approx(5520.0).epsilon(0.01));
    CHECK(bitrates[3]->getRequiredSlots(0) == 32);

    CHECK_THROWS(bitrates[3]->getModulationStr(1));

    // Bitrate 1000.0
    CHECK(bitrates[4]->getBitRate() == Approx(1000.0).epsilon(0.01));
    CHECK(std::stod(bitrates[4]->getBitRateStr()) == Approx(1000.0).epsilon(0.01));

    CHECK(bitrates[4]->getModulationStr(0) == "BPSK");
    CHECK(bitrates[4]->getReach(0) == Approx(5520.0).epsilon(0.01));
    CHECK(bitrates[4]->getRequiredSlots(0) == 80);

    CHECK_THROWS(bitrates[4]->getModulationStr(1));

    // Additional tests for band-specific values (if your JSON supports multiple bands)
    // Test accessing specific bands
    CHECK_NOTHROW(bitrates[0]->getReach(0, fns::Band::C));
    CHECK_NOTHROW(bitrates[0]->getRequiredSlots(0, fns::Band::C));
    
    // Test ModulationFormat access
    CHECK_NOTHROW(bitrates[0]->getModulationFormat(0));
    CHECK_NOTHROW(bitrates[0]->getModulationFormat("BPSK"));
}

TEST_CASE("Read multiband data from files")
{
    std::vector<std::shared_ptr<BitRate>> bitrates =
            BitRate::readBitRatesFile(std::string("../examples/example_bitrates/multi_band.json"));

    // Bitrate 100.0 - Test multiband characteristics (only has BPSK)
    CHECK(bitrates[0]->getBitRate() == Approx(100.0).epsilon(0.01));
    CHECK(std::stod(bitrates[0]->getBitRateStr()) == Approx(100.0).epsilon(0.01));
    CHECK(bitrates[0]->getNumberOfModulations() == 1);

    // BPSK (index 0) - Same slots for C, L, S, and E bands, different reach
    CHECK(bitrates[0]->getModulationStr(0) == "BPSK");
    CHECK(bitrates[0]->getRequiredSlots(0, fns::Band::C) == 1);
    CHECK(bitrates[0]->getRequiredSlots(0, fns::Band::L) == 1);
    CHECK(bitrates[0]->getRequiredSlots(0, fns::Band::S) == 1);
    CHECK(bitrates[0]->getRequiredSlots(0, fns::Band::E) == 1);
    CHECK(bitrates[0]->getReach(0, fns::Band::C) == Approx(17400.0).epsilon(0.01));
    CHECK(bitrates[0]->getReach(0, fns::Band::L) == Approx(16700.0).epsilon(0.01));
    CHECK(bitrates[0]->getReach(0, fns::Band::S) == Approx(14800.0).epsilon(0.01));
    CHECK(bitrates[0]->getReach(0, fns::Band::E) == Approx(3100.0).epsilon(0.01));

    // Bitrate 200.0 - Test second bitrate (has QPSK and BPSK)
    CHECK(bitrates[1]->getBitRate() == Approx(200.0).epsilon(0.01));
    CHECK(bitrates[1]->getNumberOfModulations() == 2);

    // QPSK (index 0) - Same slots, different reach for C, L, S, and E bands
    CHECK(bitrates[1]->getModulationStr(0) == "QPSK");
    CHECK(bitrates[1]->getRequiredSlots(0, fns::Band::C) == 1);
    CHECK(bitrates[1]->getRequiredSlots(0, fns::Band::L) == 1);
    CHECK(bitrates[1]->getRequiredSlots(0, fns::Band::S) == 1);
    CHECK(bitrates[1]->getRequiredSlots(0, fns::Band::E) == 1);
    CHECK(bitrates[1]->getReach(0, fns::Band::C) == Approx(8700.0).epsilon(0.01));
    CHECK(bitrates[1]->getReach(0, fns::Band::L) == Approx(8400.0).epsilon(0.01));
    CHECK(bitrates[1]->getReach(0, fns::Band::S) == Approx(7400.0).epsilon(0.01));
    CHECK(bitrates[1]->getReach(0, fns::Band::E) == Approx(1500.0).epsilon(0.01));

    // BPSK (index 1) for 200.0 bitrate - 2 slots for all bands
    CHECK(bitrates[1]->getModulationStr(1) == "BPSK");
    CHECK(bitrates[1]->getRequiredSlots(1, fns::Band::C) == 2);
    CHECK(bitrates[1]->getRequiredSlots(1, fns::Band::L) == 2);
    CHECK(bitrates[1]->getRequiredSlots(1, fns::Band::S) == 2);
    CHECK(bitrates[1]->getRequiredSlots(1, fns::Band::E) == 2);
    CHECK(bitrates[1]->getReach(1, fns::Band::C) == Approx(17400.0).epsilon(0.01));
    CHECK(bitrates[1]->getReach(1, fns::Band::L) == Approx(16700.0).epsilon(0.01));
    CHECK(bitrates[1]->getReach(1, fns::Band::S) == Approx(14800.0).epsilon(0.01));
    CHECK(bitrates[1]->getReach(1, fns::Band::E) == Approx(3100.0).epsilon(0.01));

    // Bitrate 400.0 - Test fourth bitrate (has all 4 modulations)
    CHECK(bitrates[3]->getBitRate() == Approx(400.0).epsilon(0.01));
    CHECK(bitrates[3]->getNumberOfModulations() == 4);

    // 16QAM (index 0) - 1 slot, lowest reach for all bands
    CHECK(bitrates[3]->getModulationStr(0) == "16QAM");
    CHECK(bitrates[3]->getRequiredSlots(0, fns::Band::C) == 1);
    CHECK(bitrates[3]->getRequiredSlots(0, fns::Band::L) == 1);
    CHECK(bitrates[3]->getRequiredSlots(0, fns::Band::S) == 1);
    CHECK(bitrates[3]->getRequiredSlots(0, fns::Band::E) == 1);
    CHECK(bitrates[3]->getReach(0, fns::Band::C) == Approx(2300.0).epsilon(0.01));
    CHECK(bitrates[3]->getReach(0, fns::Band::L) == Approx(2200.0).epsilon(0.01));
    CHECK(bitrates[3]->getReach(0, fns::Band::S) == Approx(2000.0).epsilon(0.01));
    CHECK(bitrates[3]->getReach(0, fns::Band::E) == Approx(400.0).epsilon(0.01));

    // 8QAM (index 1) - 2 slots, medium reach for all bands
    CHECK(bitrates[3]->getModulationStr(1) == "8QAM");
    CHECK(bitrates[3]->getRequiredSlots(1, fns::Band::C) == 2);
    CHECK(bitrates[3]->getRequiredSlots(1, fns::Band::L) == 2);
    CHECK(bitrates[3]->getRequiredSlots(1, fns::Band::S) == 2);
    CHECK(bitrates[3]->getRequiredSlots(1, fns::Band::E) == 2);
    CHECK(bitrates[3]->getReach(1, fns::Band::C) == Approx(4700.0).epsilon(0.01));
    CHECK(bitrates[3]->getReach(1, fns::Band::L) == Approx(4600.0).epsilon(0.01));
    CHECK(bitrates[3]->getReach(1, fns::Band::S) == Approx(4100.0).epsilon(0.01));
    CHECK(bitrates[3]->getReach(1, fns::Band::E) == Approx(900.0).epsilon(0.01));

    // Test distance adaptive algorithm with multiband
    std::shared_ptr<Link> shortLink = std::make_shared<Link>(0, 2000.0);  // 2000 km
    std::shared_ptr<Link> longLink = std::make_shared<Link>(1, 10000.0); // 10000 km
    Route shortRoute = {shortLink};
    Route longRoute = {longLink};

    // For short route (2000 km) with bitrate 100.0:
    // Only BPSK available, can handle the distance (17400km reach)
    CHECK(bitrates[0]->getAdaptiveModulation(shortRoute, fns::Band::C) == 0);  // BPSK
    CHECK(bitrates[0]->getAdaptiveModulation(shortRoute, fns::Band::L) == 0);  // BPSK
    CHECK(bitrates[0]->getAdaptiveModulation(shortRoute, fns::Band::S) == 0);  // BPSK
    CHECK(bitrates[0]->getAdaptiveModulation(shortRoute, fns::Band::E) == 0);  // BPSK

    // For long route (10000 km) with bitrate 100.0:
    // BPSK can handle it for C/L/S bands (17400km, 16700km, 14800km reach > 10000km)
    CHECK(bitrates[0]->getAdaptiveModulation(longRoute, fns::Band::C) == 0);  // BPSK  
    CHECK(bitrates[0]->getAdaptiveModulation(longRoute, fns::Band::L) == 0);  // BPSK
    CHECK(bitrates[0]->getAdaptiveModulation(longRoute, fns::Band::S) == 0);  // BPSK
    // E band BPSK only has 3100km reach, cannot handle 10000km route
    CHECK(bitrates[0]->getAdaptiveModulation(longRoute, fns::Band::E) == -1);  // No suitable modulation

    // Test with bitrate 400.0 for distance adaptive (has all 4 modulations)
    // For short route (2000 km), 16QAM can handle it (2300km reach > 2000km)
    // Algorithm picks minimum slots first, so 16QAM (1 slot) over others
    CHECK(bitrates[3]->getAdaptiveModulation(shortRoute, fns::Band::C) == 0);  // 16QAM
    CHECK(bitrates[3]->getAdaptiveModulation(shortRoute, fns::Band::L) == 0);  // 16QAM
    CHECK(bitrates[3]->getAdaptiveModulation(shortRoute, fns::Band::S) == 0);  // 16QAM
    // E band: 16QAM(400km), 8QAM(900km), QPSK(1500km), BPSK(3100km)
    // For 2000km route, only BPSK (3100km > 2000km) can handle it
    CHECK(bitrates[3]->getAdaptiveModulation(shortRoute, fns::Band::E) == 3);  // BPSK

    // For long route (10000 km), only BPSK can handle it (17400km reach > 10000km)
    // 16QAM (2300km), 8QAM (4700km), QPSK (8700km) cannot handle 10000km
    CHECK(bitrates[3]->getAdaptiveModulation(longRoute, fns::Band::C) == 3);  // BPSK (index 3)
    CHECK(bitrates[3]->getAdaptiveModulation(longRoute, fns::Band::L) == 3);  // BPSK (index 3)
    CHECK(bitrates[3]->getAdaptiveModulation(longRoute, fns::Band::S) == 3);  // BPSK (index 3)
    // E band: BPSK only has 3100km reach, cannot handle 10000km route
    CHECK(bitrates[3]->getAdaptiveModulation(longRoute, fns::Band::E) == -1);  // No suitable modulation

    // Test ModulationFormat access for multiband
    CHECK_NOTHROW(bitrates[0]->getModulationFormat(0));
    CHECK_NOTHROW(bitrates[0]->getModulationFormat("BPSK"));
    CHECK_NOTHROW(bitrates[3]->getModulationFormat("8QAM"));
    CHECK_THROWS_AS(bitrates[0]->getModulationFormat("64QAM"), std::invalid_argument);
}