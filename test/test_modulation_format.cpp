// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/core/modulation_format.hpp"
#include "catch.hpp"

TEST_CASE("Constructor (ModulationFormat)")
{
    // Test simple constructor
    CHECK_NOTHROW(ModulationFormat("QPSK"));

    // Test constructor with band-specific parameters
    std::map<fns::Band, int> slots = {{fns::Band::C, 15}, {fns::Band::L, 10}};
    std::map<fns::Band, double> reach = {{fns::Band::C, 150.0}, {fns::Band::L, 100.0}};
    CHECK_NOTHROW(ModulationFormat("16QAM", slots, reach));

    // Test with multiple bands
    std::map<fns::Band, int> multiSlots = {{fns::Band::C, 5}, {fns::Band::L, 3}, {fns::Band::S, 8}};
    std::map<fns::Band, double> multiReach = {{fns::Band::C, 2000.0}, {fns::Band::L, 1800.0}, {fns::Band::S, 2200.0}};
    CHECK_NOTHROW(ModulationFormat("8-QAM", multiSlots, multiReach));
}

TEST_CASE("Basic ModulationFormat Operations")
{
    ModulationFormat mf("QPSK");
    
    // Test modulation string access
    CHECK(mf.getModulationStr() == "QPSK");

    // Test future use parameters (initially zero)
    CHECK(mf.getRequiredGSNR() == Approx(0.0).epsilon(0.01));
    CHECK(mf.getBaudRate() == Approx(0.0).epsilon(0.01));
}

TEST_CASE("Band-specific access and operations")
{
    std::map<fns::Band, int> slots = {{fns::Band::C, 15}, {fns::Band::L, 10}, {fns::Band::S, 8}};
    std::map<fns::Band, double> reach = {{fns::Band::C, 150.0}, {fns::Band::L, 100.0}, {fns::Band::S, 200.0}};
    ModulationFormat mf("16-QAM", slots, reach);

    // Test slots access for different bands
    CHECK(mf.getRequiredSlots(fns::Band::C) == 15);
    CHECK(mf.getRequiredSlots(fns::Band::L) == 10);
    CHECK(mf.getRequiredSlots(fns::Band::S) == 8);
    CHECK(mf.getRequiredSlots() == 15);  // Default C band

    // Test reach access for different bands
    CHECK(mf.getReach(fns::Band::C) == Approx(150.0).epsilon(0.01));
    CHECK(mf.getReach(fns::Band::L) == Approx(100.0).epsilon(0.01));
    CHECK(mf.getReach(fns::Band::S) == Approx(200.0).epsilon(0.01));
    CHECK(mf.getReach() == Approx(150.0).epsilon(0.01));  // Default C band

    // Test exception for unsupported bands
    CHECK_THROWS_AS(mf.getRequiredSlots(fns::Band::E), std::invalid_argument);
    CHECK_THROWS_AS(mf.getRequiredSlots(fns::Band::U), std::invalid_argument);
    CHECK_THROWS_AS(mf.getReach(fns::Band::E), std::invalid_argument);
    CHECK_THROWS_AS(mf.getReach(fns::Band::O), std::invalid_argument);
}

TEST_CASE("Setters and parameter updates")
{
    ModulationFormat mf("BPSK");

    // Test setting slots for different bands
    CHECK_NOTHROW(mf.setRequiredSlots(fns::Band::C, 20));
    CHECK_NOTHROW(mf.setRequiredSlots(fns::Band::L, 15));
    CHECK(mf.getRequiredSlots(fns::Band::C) == 20);
    CHECK(mf.getRequiredSlots(fns::Band::L) == 15);

    // Test setting reach for different bands
    CHECK_NOTHROW(mf.setReach(fns::Band::C, 5000.0));
    CHECK_NOTHROW(mf.setReach(fns::Band::L, 4500.0));
    CHECK(mf.getReach(fns::Band::C) == Approx(5000.0).epsilon(0.01));
    CHECK(mf.getReach(fns::Band::L) == Approx(4500.0).epsilon(0.01));

    // Test invalid parameters
    CHECK_THROWS_AS(mf.setRequiredSlots(fns::Band::C, -5), std::invalid_argument);
    CHECK_THROWS_AS(mf.setReach(fns::Band::C, -100.0), std::invalid_argument);

    // Test future use parameters
    CHECK_NOTHROW(mf.setRequiredGSNR(15.5));
    CHECK_NOTHROW(mf.setBaudRate(32.0));
    CHECK(mf.getRequiredGSNR() == Approx(15.5).epsilon(0.01));
    CHECK(mf.getBaudRate() == Approx(32.0).epsilon(0.01));

    // Test invalid baud rate
    CHECK_THROWS_AS(mf.setBaudRate(-10.0), std::invalid_argument);

    // Test updating existing band values
    CHECK_NOTHROW(mf.setRequiredSlots(fns::Band::C, 25));
    CHECK(mf.getRequiredSlots(fns::Band::C) == 25);
    CHECK_NOTHROW(mf.setReach(fns::Band::C, 6000.0));
    CHECK(mf.getReach(fns::Band::C) == Approx(6000.0).epsilon(0.01));
}